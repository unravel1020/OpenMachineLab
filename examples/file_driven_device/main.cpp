// file_driven_device - a DieBonder built entirely from files.
//
// End-to-end validation of the persistence trio: main() loads a DeviceConfig
// from diebonder.cfg and a RecipeSpec from diebonder.recipe, registers the
// device's named actions (bound to its resources), and builds the device from
// them. Nothing about the wiring or the recipe is hardcoded in this binary -
// edit the files and rerun. (The cfg uses non-default axis positions so the run
// visibly proves the files were read.)
#include "config/DeviceConfig.h"
#include "device/Device.h"
#include "log/Logger.h"
#include "module/IoModule.h"
#include "module/MotionModule.h"
#include "module/VisionModule.h"
#include "resource/Axis.h"
#include "resource/Camera.h"
#include "resource/DigitalIO.h"
#include "workflow/ActionRegistry.h"
#include "workflow/RecipeSpec.h"
#include "workflow/Workflow.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace oml;

namespace {

// A DieBonder whose config (channels/positions) and recipe (step sequence) both
// come from files. The actions themselves are code - they are registered by name
// and the recipe file references those names.
class FileDrivenDieBonder : public Device {
public:
    FileDrivenDieBonder(const DeviceConfig& cfg, std::istream& recipe_stream)
        : Device("DieBonder") {
        auto axis   = std::make_unique<Axis>();
        auto camera = std::make_unique<Camera>();
        auto io     = std::make_unique<DigitalIO>();
        Axis*      axis_p   = axis.get();
        Camera*    camera_p = camera.get();
        DigitalIO* io_p     = io.get();

        AddResource(std::move(axis));
        AddResource(std::move(camera));
        AddResource(std::move(io));
        AddModule(std::make_unique<MotionModule>(*axis_p));
        AddModule(std::make_unique<VisionModule>(*camera_p));
        AddModule(std::make_unique<IoModule>(*io_p));

        // Config-driven wiring (falls back to sane defaults per key).
        const int  part_di = static_cast<int>(cfg.GetLong("part_present_di", 1));
        const int  vacuum  = static_cast<int>(cfg.GetLong("vacuum_do", 1));
        const long load    = cfg.GetLong("load_pos", 100);
        const long align   = cfg.GetLong("align_pos", 200);
        const long unload  = cfg.GetLong("unload_pos", 300);
        io_p->SimulateInput(part_di, true); // feed a part so LoadFrame succeeds

        // Named actions, bound to the resources. The recipe file names these.
        ActionRegistry reg;
        reg.Register("load", [axis_p, io_p, part_di, load] {
            if (!io_p->Read(part_di)) {
                Log().Warn("            ! no part present");
                return false;
            }
            axis_p->MoveTo(load);
            Log().Info("            axis @ " + std::to_string(axis_p->Position()));
            return true;
        });
        reg.Register("align", [axis_p, camera_p, align] {
            camera_p->Trigger();
            axis_p->MoveTo(align);
            Log().Info("            camera #" + std::to_string(camera_p->Captures())
                       + "; axis @ " + std::to_string(axis_p->Position()));
            return true;
        });
        reg.Register("bond", [io_p, vacuum] {
            io_p->Write(vacuum, true);
            Log().Info("            vacuum on");
            return true;
        });
        reg.Register("unload", [axis_p, io_p, vacuum, unload] {
            axis_p->MoveTo(unload);
            io_p->Write(vacuum, false);
            Log().Info("            axis @ " + std::to_string(axis_p->Position())
                       + "; vacuum off");
            return true;
        });

        RecipeSpec spec;
        if (!LoadRecipe(recipe_stream, spec)) {
            Log().Error("            could not parse recipe file");
            return;
        }
        auto wf = BuildWorkflow(spec, reg);
        if (!wf) {
            Log().Error("            recipe references an unknown action");
            return;
        }
        AddWorkflow(std::move(wf));
    }
};

} // namespace

int main() {
    DeviceConfig cfg;
    std::ifstream cfg_file("diebonder.cfg");
    if (cfg_file) {
        cfg.Load(cfg_file);
        Log().Info("loaded config from diebonder.cfg");
    } else {
        Log().Warn("could not open diebonder.cfg - using built-in defaults");
    }

    std::ifstream recipe_file("diebonder.recipe");
    if (!recipe_file) {
        Log().Error("could not open diebonder.recipe");
        return 1;
    }
    Log().Info("loaded recipe from diebonder.recipe");

    FileDrivenDieBonder device(cfg, recipe_file);

    Log().Info("\n>>> press <Enter> to stop the " + device.Name() + " <<<");
    std::thread stopper([&device] {
        std::string line;
        std::getline(std::cin, line);
        device.Stop();
    });

    device.Initialize();
    device.Run();
    stopper.join();
    device.Shutdown();

    return 0;
}
