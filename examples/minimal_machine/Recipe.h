#pragma once

#include "config/DeviceConfig.h"
#include "log/Logger.h"
#include "resource/Axis.h"
#include "resource/Camera.h"
#include "resource/DigitalIO.h"
#include "workflow/Workflow.h"

#include <memory>
#include <string>

namespace oml::example::die_bonder {

// Built-in defaults for the die bonder's channels/positions. A DeviceConfig can
// override any of these (keys match the names below). DI and DO are separate
// line spaces; channel 0 is reserved by IoModule (e-stop / light).
constexpr int  kPartPresentDi = 1; // DI: "part present at the load station"
constexpr int  kVacuumDo      = 1; // DO: vacuum pick
constexpr long kLoadPos       = 100;
constexpr long kAlignPos      = 200;
constexpr long kUnloadPos     = 300;

// The per-part recipe: axis through the stations, camera at align, vacuum
// pick/release. Channels/positions come from `cfg` (falling back to the
// constants above). LoadFrame fails (-> Fault) when no part is present.
inline std::unique_ptr<Workflow> BuildRecipe(Axis& axis, Camera& camera, DigitalIO& io,
                                             const DeviceConfig& cfg = {}) {
    const int  partPresent = static_cast<int>(cfg.GetLong("part_present_di", kPartPresentDi));
    const int  vacuum      = static_cast<int>(cfg.GetLong("vacuum_do", kVacuumDo));
    const long load        = cfg.GetLong("load_pos", kLoadPos);
    const long align       = cfg.GetLong("align_pos", kAlignPos);
    const long unload      = cfg.GetLong("unload_pos", kUnloadPos);

    auto recipe = std::make_unique<Workflow>("Recipe");

    recipe->AddStep("LoadFrame", [&, partPresent, load] {
        if (!io.Read(partPresent)) {
            Log().Warn("            ! no part present");
            return false;
        }
        axis.MoveTo(load);
        Log().Info("            axis @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Align", [&, align] {
        camera.Trigger();
        axis.MoveTo(align);
        Log().Info("            camera #" + std::to_string(camera.Captures())
                   + "; axis @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Bond", [&, vacuum] {
        io.Write(vacuum, true); // pick
        Log().Info("            vacuum on");
        return true;
    });

    recipe->AddStep("Unload", [&, vacuum, unload] {
        axis.MoveTo(unload);
        io.Write(vacuum, false); // release
        Log().Info("            axis @ " + std::to_string(axis.Position())
                   + "; vacuum off");
        return true;
    });

    return recipe;
}

} // namespace oml::example::die_bonder
