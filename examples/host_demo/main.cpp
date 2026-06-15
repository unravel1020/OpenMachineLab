// host_demo - one Host managing devices built by a DeviceFactory.
// host_demo - 一个 Host 管理 DeviceFactory 构建的设备。
//
// Registers the concrete device types (DieBonder, PickAndPlace) with a factory
// by name, builds them via the factory, and hands them to a Host for uniform
// management (InitializeAll, Status, ShutdownAll). Creation (factory) and
// management (Host) stay decoupled.
// 将具体设备类型（DieBonder、PickAndPlace）按名注册到工厂，通过工厂构建，
// 交给 Host 统一管理（InitializeAll、Status、ShutdownAll）。
// 创建（工厂）与管理（Host）保持解耦。
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
    // 将具体设备类型注册到工厂（按类型名）。
    DeviceFactory factory;
    factory.Register("DieBonder",
                     [](const DeviceConfig& c) { return std::make_unique<DieBonder>(c); });
    factory.Register("PickAndPlace",
                     [](const DeviceConfig&) { return std::make_unique<PickAndPlace>(); });

    // Build devices by type name and hand them to a Host to manage.
    // 按类型名构建设备，交给 Host 管理。
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
