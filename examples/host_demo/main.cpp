// host_demo - one Host managing devices built by a DeviceFactory.
//
// Registers the concrete device types (DieBonder, PickAndPlace) with a factory
// by name, builds them via the factory, and hands them to a Host for uniform
// management (InitializeAll, Status, ShutdownAll). Creation (factory) and
// management (Host) stay decoupled.
#include "config/DeviceConfig.h"
#include "device/DeviceFactory.h"
#include "host/Host.h"
#include "log/Logger.h"
#include "minimal_machine/DieBonder.h"
#include "pick_and_place/PickAndPlace.h"

#include <memory>
#include <string>

using namespace oml;
using namespace oml::example;

int main() {
    // Register concrete device types with a factory (by type name).
    DeviceFactory factory;
    factory.Register("DieBonder",
                     [](const DeviceConfig& c) { return std::make_unique<DieBonder>(c); });
    factory.Register("PickAndPlace",
                     [](const DeviceConfig&) { return std::make_unique<PickAndPlace>(); });

    // Build devices by type name and hand them to a Host to manage.
    Host host;
    host.Register(factory.Create("DieBonder"));
    host.Register(factory.Create("PickAndPlace"));

    Log().Info("host managing " + std::to_string(host.Size()) + " device(s):");
    for (const auto& name : host.Names()) Log().Info("    - " + name);

    Log().Info("");
    Log().Info("InitializeAll ->");
    host.InitializeAll();

    Log().Info("");
    Log().Info("Status ->");
    for (const auto& [name, state] : host.Status()) {
        Log().Info("    " + name + ": " + std::string(ToString(state)));
    }

    Log().Info("");
    Log().Info("ShutdownAll ->");
    host.ShutdownAll();

    return 0;
}
