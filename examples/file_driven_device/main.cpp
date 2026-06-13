// file_driven_device - a DieBonder built from files, demonstrated with a fault.
//
// Loads a DeviceConfig from diebonder.cfg and a RecipeSpec from diebonder.recipe,
// registers the device's named actions, and builds the device from them. It runs
// normally for a couple of cycles, then the bond action fails (a simulated
// intermittent process error) on the 3rd cycle: that raises a PROCESS_FAIL alarm
// (the CAUSE) and drives the machine into Fault (the STATE). main then lists the
// active alarms and recovers via Reset - the full cause/state/recovery picture.
#include "alarm/Alarm.h"
#include "config/DeviceConfig.h"
#include "device/Device.h"
#include "log/Logger.h"
#include "module/IoModule.h"
#include "module/MotionModule.h"
#include "module/VisionModule.h"
#include "resource/Axis.h"
#include "resource/Camera.h"
#include "resource/DigitalIO.h"
#include "state/MachineState.h"
#include "workflow/ActionRegistry.h"
#include "workflow/RecipeSpec.h"
#include "workflow/Workflow.h"

#include <fstream>
#include <memory>
#include <string>

using namespace oml;

namespace {

// A DieBonder whose config (channels/positions) and recipe (step sequence) both
// come from files. The "bond" action simulates an intermittent failure: it
// succeeds twice, then faults on the third attempt.
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

        const int  part_di = static_cast<int>(cfg.GetLong("part_present_di", 1));
        const int  vacuum  = static_cast<int>(cfg.GetLong("vacuum_do", 1));
        const long load    = cfg.GetLong("load_pos", 100);
        const long align   = cfg.GetLong("align_pos", 200);
        const long unload  = cfg.GetLong("unload_pos", 300);
        io_p->SimulateInput(part_di, true); // feed a part so LoadFrame succeeds

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
        reg.Register("bond", [this, io_p, vacuum] {
            if (++bond_attempts_ >= 3) {
                Log().Warn("            bond failed (simulated process error)");
                return false; // -> PROCESS_FAIL alarm -> Fault
            }
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

private:
    int bond_attempts_ = 0; // simulates wear: the 3rd bond fails
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
    device.Initialize();
    device.Run(); // runs cycles; faults when the bond action fails

    if (device.State() == MachineState::Fault) {
        Log().Info("machine FAULTED - active alarms (the causes):");
        for (const Alarm& a : device.Alarms().Active()) {
            Log().Info(std::string("  [") + std::string(ToString(a.severity))
                       + "] " + a.name + ": " + a.message);
        }
        Log().Info("resetting (clears the alarms, recovers to Ready)...");
        device.Reset();
    }

    device.Shutdown();
    return 0;
}
