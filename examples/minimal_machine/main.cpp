// minimal_machine - Phase 1 acceptance demo for OpenMachineLab.
//
// Proves the runtime model runs end to end:
//   Case 1  create a virtual machine
//   Case 2  attach two resources
//   Case 3  attach two modules
//   Case 4  attach a per-part recipe (LoadFrame -> Align -> Bond -> Unload)
//   Case 5  run in a loop until the operator issues Stop, then Stopping -> Stopped
//
// A machine is not a script: it keeps cycling until told to stop. Here the
// "operator console" is stdin - a background thread waits for a line (press
// Enter) and then requests Stop. Run() blocks in the main thread until then.
#include "machine/Machine.h"
#include "module/Module.h"
#include "resource/Resource.h"
#include "workflow/Workflow.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace oml;

// ---- concrete resources -------------------------------------------------
class AxisResource final : public Resource {
public:
    std::string Name() const override { return "AxisResource"; }
};

class CameraResource final : public Resource {
public:
    std::string Name() const override { return "CameraResource"; }
};

// ---- concrete modules ---------------------------------------------------
class ModuleA final : public Module {
public:
    std::string Name() const override { return "ModuleA"; }
};

class ModuleB final : public Module {
public:
    std::string Name() const override { return "ModuleB"; }
};

int main() {
    // Case 1: a virtual device.
    Machine machine;

    // Case 2: resources.
    machine.AddResource(std::make_unique<AxisResource>());
    machine.AddResource(std::make_unique<CameraResource>());

    // Case 3: modules.
    machine.AddModule(std::make_unique<ModuleA>());
    machine.AddModule(std::make_unique<ModuleB>());

    // Case 4: a per-part recipe, executed once every run cycle.
    auto recipe = std::make_unique<Workflow>("Recipe");
    recipe->AddStep("LoadFrame", [] { /* device-specific */ });
    recipe->AddStep("Align",     [] { /* device-specific */ });
    recipe->AddStep("Bond",      [] { /* device-specific */ });
    recipe->AddStep("Unload",    [] { /* device-specific */ });
    machine.AddWorkflow(std::move(recipe));

    machine.Initialize();

    // Case 5: run in a loop until an exit command arrives. The operator console
    // is stdin: a background thread waits for one line (press Enter) and then
    // requests Stop. Run() blocks the main thread, cycling until that happens.
    std::cout << "\n>>> press <Enter> to stop the machine <<<\n";
    std::thread operator_console([&machine] {
        std::string line;
        std::getline(std::cin, line); // blocks until the operator types a line
        machine.Stop();
    });

    machine.Run(); // cycles until Stop() is requested
    operator_console.join();
    machine.Shutdown();

    return 0;
}
