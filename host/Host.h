#pragma once

#include "device/Device.h"
#include "state/MachineState.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace oml {

// Manages a set of Devices uniformly: register by name, drive the non-blocking
// lifecycle across all of them, and query status. This is the "hosting" layer
// over the Device facade (ADR-0011) — a host process (or a test) manages many
// devices through a single handle.
// 统一管理一组 Device：按名注册、统一驱动非阻塞生命周期、查询状态。
// 这是 Device facade 之上的"托管"层（ADR-0011）——一个 host 进程（或测试）
// 通过一个句柄管理多台设备。
//
// Run() is intentionally NOT centralized here: it blocks, so a single-threaded
// host runs one device at a time via Find(name)->Run(), and running several
// devices concurrently is the job of the future task/delegate pool
// (RoadMap Phase 3).
// Run() 刻意不集中在这里：它会阻塞，单线程 host 通过 Find(name)->Run()
// 一次跑一台设备；多设备并发运行是未来 task/delegate pool 的职责
// （RoadMap Phase 3）。
class Host {
public:
    // Take ownership of a device. Its Name() must be unique within this host.
    // 接管设备所有权。其 Name() 在本 host 内必须唯一。
    void Register(std::unique_ptr<Device> device);

    bool         Has(const std::string& name) const;
    Device*      Find(const std::string& name) const; // nullptr if not found 未找到返回 nullptr
    std::size_t  Size() const { return devices_.size(); }

    // Device names, in registration order.
    // 设备名称，按注册顺序。
    const std::vector<std::string>& Names() const { return names_; }

    // Uniform, non-blocking lifecycle applied to every device.
    // 对每台设备施加统一的非阻塞生命周期。
    void InitializeAll();
    void StopAll();      // request stop on every device (e.g. a global stop) 对每台设备请求停止（如全局停止）
    void ResetAll();
    void ShutdownAll();

    // Snapshot of every device's name and state, in registration order.
    // 每台设备的名称和状态快照，按注册顺序。
    std::vector<std::pair<std::string, MachineState>> Status() const;

private:
    std::vector<std::unique_ptr<Device>> devices_;
    std::vector<std::string>             names_;
};

} // namespace oml
