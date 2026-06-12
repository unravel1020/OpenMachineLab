// minimal_machine - Phase 1 acceptance demo for OpenMachineLab.
//
// Proves the runtime model runs end to end:
//   Case 1  create a virtual machine
//   Case 2  attach two resources
//   Case 3  attach two modules
//   Case 4  attach one workflow (Initialize -> Run -> Shutdown)
//   Case 5  run the full lifecycle and print the state trace
#include "machine/Machine.h"
#include "module/Module.h"
#include "resource/Resource.h"
#include "workflow/Workflow.h"

#include <memory>

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

    // Case 4: a workflow with three steps.
    auto workflow = std::make_unique<Workflow>("MainWorkflow");
    workflow->AddStep("Initialize", [] { /* device-specific init */ });
    workflow->AddStep("Run",        [] { /* device-specific run  */ });
    workflow->AddStep("Shutdown",   [] { /* device-specific stop */ });
    machine.AddWorkflow(std::move(workflow));

    // Case 5: full lifecycle. The trace is printed to stdout.
    machine.Initialize();
    machine.Run();
    machine.Shutdown();

    return 0;
}
