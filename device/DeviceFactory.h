#pragma once

#include "config/DeviceConfig.h"
#include "device/Device.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace oml {

// Creates devices by type name + config. A registry maps a type name to a
// Creator (DeviceConfig -> unique_ptr<Device>). The application registers its
// concrete device types once; Create builds them without the caller knowing the
// concrete class - the seam for a file-driven device line.
//
// Standalone (the Host does not own it): the app registers types, creates
// devices via the factory, and hands them to the Host to manage. Creation and
// management stay decoupled (ADR-0019).
class DeviceFactory {
public:
    using Creator = std::function<std::unique_ptr<Device>(const DeviceConfig&)>;

    void Register(const std::string& type, Creator creator) {
        creators_[type] = std::move(creator);
    }

    bool Knows(const std::string& type) const { return creators_.count(type) != 0; }

    // Build a device of `type` with `cfg`, or nullptr if the type is unknown.
    std::unique_ptr<Device> Create(const std::string& type, const DeviceConfig& cfg = {}) const {
        const auto it = creators_.find(type);
        if (it == creators_.end()) return nullptr;
        return it->second(cfg);
    }

private:
    std::map<std::string, Creator> creators_;
};

} // namespace oml
