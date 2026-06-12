// minimal_machine - entry point for the demo device.
//
// main() only ASSEMBLES the machine from reusable library parts (resources and
// modules, now in the library as Phase 2 stubs) plus a device-specific recipe,
// then drives the lifecycle. No concrete class is defined here.
//
//   Case 1  create a virtual machine
//   Case 2  attach two resources (Axis, Camera)
//   Case 3  attach two modules that use those resources (MotionModule, VisionModule)
//   Case 4  attach a per-part recipe (LoadFrame -> Align -> Bond -> Unload)
//   Case 5  run in a loop until the operator issues Stop(), then Stopping -> Stopped
#include "machine/Machine.h"
#include "module/MotionModule.h"
#include "module/VisionModule.h"
#include "resource/Axis.h"
#include "resource/Camera.h"

#include "OperatorConsole.h"
#include "Recipe.h"

#include <memory>
#include <utility>

using namespace oml;
using namespace oml::example;

int main() {
    // Case 1: a virtual device.
    Machine machine;

    // Case 2: reusable resources (library stubs). Heap-allocated so the machine
    // owns them while the modules borrow stable references.
    auto axis   = std::make_unique<Axis>();
    auto camera = std::make_unique<Camera>();
    Axis&   axis_ref   = *axis;
    Camera& camera_ref = *camera;

    machine.AddResource(std::move(axis));
    machine.AddResource(std::move(camera));

    // Case 3: modules that drive those resources.
    machine.AddModule(std::make_unique<MotionModule>(axis_ref));
    machine.AddModule(std::make_unique<VisionModule>(camera_ref));

    // Case 4: the per-part recipe.
    machine.AddWorkflow(BuildRecipe());

    // Case 5: run in a loop until an exit command arrives. The operator console
    // (stdin) requests Stop on Enter; Run() blocks until then.
    OperatorConsole console(machine);
    machine.Initialize();
    machine.Run();
    console.Wait();
    machine.Shutdown();

    return 0;
}
