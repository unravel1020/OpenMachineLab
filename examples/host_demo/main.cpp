// host_demo - one Host managing multiple devices.
//
// Registers two different device profiles (a DieBonder and a PickAndPlace) with
// a single Host, then drives them uniformly: InitializeAll, Status, ShutdownAll.
// This is the "hosting" layer over the Device facade - one handle, many devices.
// Running devices concurrently is deferred to the task/delegate pool (Phase 3);
// here we only show uniform management.
#include "host/Host.h"
#include "log/Logger.h"
#include "minimal_machine/DieBonder.h"
#include "pick_and_place/PickAndPlace.h"

#include <memory>
#include <string>

using namespace oml;
using namespace oml::example;

int main() {
    Host host;
    host.Register(std::make_unique<DieBonder>());
    host.Register(std::make_unique<PickAndPlace>());

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
