#pragma once

#include "machine/Machine.h"
#include "state/MachineState.h"

#include <memory>
#include <string>
#include <utility>

namespace oml {

// A named, configured device: a thin composition root over a Machine.
//
// Concrete devices (a die bonder, a pick-and-place, ...) subclass Device and
// populate their resources, modules, and workflows in their own constructor via
// the protected Add* helpers. Every device then exposes the same lifecycle
// (Initialize/Run/Stop/Reset/Shutdown + State), so a host, tooling, or tests
// can manage any device uniformly - without the core Machine knowing about any
// specific device (ADR-0001).
//
// Deliberately a thin facade (ADR-0011): no plugin system, registry, config
// loader, or IoC container. Those wait for a concrete need. Device holds a
// Machine by value, so (like Machine) it is non-copyable and non-movable.
class Device {
public:
    explicit Device(std::string name) : name_(std::move(name)) {}
    virtual ~Device() = default;

    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;

    const std::string& Name() const { return name_; }

    // Uniform lifecycle, delegated to the owned machine.
    void Initialize() { machine_.Initialize(); }
    void Run()        { machine_.Run(); }
    void Stop()       { machine_.Stop(); }
    void Reset()      { machine_.Reset(); }
    void Shutdown()   { machine_.Shutdown(); }

    MachineState State() const { return machine_.State(); }

protected:
    // Composition helpers for subclasses to call from their constructor.
    void AddResource(std::unique_ptr<Resource> resource) { machine_.AddResource(std::move(resource)); }
    void AddModule(std::unique_ptr<Module> module)       { machine_.AddModule(std::move(module)); }
    void AddWorkflow(std::unique_ptr<Workflow> workflow) { machine_.AddWorkflow(std::move(workflow)); }

private:
    std::string name_;
    Machine     machine_;
};

} // namespace oml
