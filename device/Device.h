#pragma once

#include "machine/Machine.h"
#include "state/MachineState.h"

#include <memory>
#include <string>
#include <utility>

namespace oml {

// A named, configured device: a thin composition root over a Machine.
// 具名、已配置的设备：基于 Machine 的薄组装层。
//
// Concrete devices (a die bonder, a pick-and-place, ...) subclass Device and
// populate their resources, modules, and workflows in their own constructor via
// the protected Add* helpers. Every device then exposes the same lifecycle
// (Initialize/Run/Stop/Reset/Shutdown + State), so a host, tooling, or tests
// can manage any device uniformly — without the core Machine knowing about any
// specific device (ADR-0001).
// 具体设备（固晶机、贴片机……）继承 Device，在自己的构造函数中通过
// protected Add* 辅助方法填充资源、模块和工作流。每个设备暴露相同的生命周期
// （Initialize/Run/Stop/Reset/Shutdown + State），使 host、工具或测试可以
// 统一管理任意设备——核心 Machine 不感知任何具体设备（ADR-0001）。
//
// Deliberately a thin facade (ADR-0011): no plugin system, registry, config
// loader, or IoC container. Those wait for a concrete need. Device holds a
// Machine by value, so (like Machine) it is non-copyable and non-movable.
// 刻意做成薄 facade（ADR-0011）：无插件系统、注册表、配置加载器或 IoC 容器。
// 那些等真实需求出现再说。Device 按值持有 Machine，因此（和 Machine 一样）
// 不可拷贝、不可移动。
class Device {
public:
    explicit Device(std::string name) : name_(std::move(name)) {}
    virtual ~Device() = default;

    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;

    const std::string& Name() const { return name_; }

    // Uniform lifecycle, delegated to the owned machine.
    // 统一生命周期，委托给持有的 machine。
    void Initialize() { machine_.Initialize(); }
    void Run()        { machine_.Run(); }
    void Stop()       { machine_.Stop(); }
    void Reset()      { machine_.Reset(); }
    void Shutdown()   { machine_.Shutdown(); }

    MachineState State() const { return machine_.State(); }

    // Observability hooks on the owned machine.
    // 持有 machine 上的可观测接口。
    EventBus&     Bus()    { return machine_.Bus(); }
    AlarmManager& Alarms() { return machine_.Alarms(); }

protected:
    // Composition helpers for subclasses to call from their constructor.
    // 子类在构造函数中调用的组装辅助方法。
    void AddResource(std::unique_ptr<Resource> resource) { machine_.AddResource(std::move(resource)); }
    void AddModule(std::unique_ptr<Module> module)       { machine_.AddModule(std::move(module)); }
    void AddWorkflow(std::unique_ptr<Workflow> workflow) { machine_.AddWorkflow(std::move(workflow)); }

private:
    std::string name_;
    Machine     machine_;
};

} // namespace oml
