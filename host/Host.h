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
// over the Device facade (ADR-0011) - a host process (or a test) manages many
// devices through a single handle.
//
// Run() is intentionally NOT centralized here: it blocks, so a single-threaded
// host runs one device at a time via Find(name)->Run(), and running several
// devices concurrently is the job of the future task/delegate pool
// (RoadMap Phase 3).
class Host {
public:
    // Take ownership of a device. Its Name() must be unique within this host.
    void Register(std::unique_ptr<Device> device);

    bool         Has(const std::string& name) const;
    Device*      Find(const std::string& name) const; // nullptr if not found
    std::size_t  Size() const { return devices_.size(); }

    // Device names, in registration order.
    const std::vector<std::string>& Names() const { return names_; }

    // Uniform, non-blocking lifecycle applied to every device.
    void InitializeAll();
    void StopAll();      // request stop on every device (e.g. a global stop)
    void ResetAll();
    void ShutdownAll();

    // Snapshot of every device's name and state, in registration order.
    std::vector<std::pair<std::string, MachineState>> Status() const;

private:
    std::vector<std::unique_ptr<Device>> devices_;
    std::vector<std::string>             names_;
};

} // namespace oml
