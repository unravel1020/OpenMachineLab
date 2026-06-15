#include "host/Host.h"

#include <utility>

namespace oml {

void Host::Register(std::unique_ptr<Device> device) {
    // Record the name, then take ownership.
    // 记录名称，然后接管所有权。
    names_.push_back(device->Name());
    devices_.push_back(std::move(device));
}

bool Host::Has(const std::string& name) const {
    return Find(name) != nullptr;
}

// Linear search by device name. 设备名线性查找。
Device* Host::Find(const std::string& name) const {
    for (const auto& device : devices_) {
        if (device->Name() == name) return device.get();
    }
    return nullptr;
}

void Host::InitializeAll() {
    for (const auto& device : devices_) device->Initialize();
}

void Host::StopAll() {
    for (const auto& device : devices_) device->Stop();
}

void Host::ResetAll() {
    for (const auto& device : devices_) device->Reset();
}

void Host::ShutdownAll() {
    for (const auto& device : devices_) device->Shutdown();
}

// Build a snapshot of (name, state) for every device.
// 为每台设备构建 (名称, 状态) 快照。
std::vector<std::pair<std::string, MachineState>> Host::Status() const {
    std::vector<std::pair<std::string, MachineState>> status;
    status.reserve(devices_.size());
    for (const auto& device : devices_) {
        status.emplace_back(device->Name(), device->State());
    }
    return status;
}

} // namespace oml
