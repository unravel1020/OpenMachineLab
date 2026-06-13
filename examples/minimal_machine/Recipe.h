#pragma once

#include "log/Logger.h"
#include "resource/Axis.h"
#include "resource/Camera.h"
#include "resource/DigitalIO.h"
#include "workflow/Workflow.h"

#include <memory>
#include <string>

namespace oml::example::die_bonder {

// Device-specific I/O channels and axis positions for the die bonder. DI and DO
// are separate line spaces; channel 0 is reserved by IoModule (e-stop / light).
constexpr int  kPartPresentDi = 1; // DI: "part present at the load station"
constexpr int  kVacuumDo      = 1; // DO: vacuum pick
constexpr long kLoadPos       = 100;
constexpr long kAlignPos      = 200;
constexpr long kUnloadPos     = 300;

// The per-part recipe: axis through the stations, camera at align, vacuum
// pick/release. LoadFrame fails (-> Fault) when no part is present.
inline std::unique_ptr<Workflow> BuildRecipe(Axis& axis, Camera& camera, DigitalIO& io) {
    auto recipe = std::make_unique<Workflow>("Recipe");

    recipe->AddStep("LoadFrame", [&] {
        if (!io.Read(kPartPresentDi)) {
            Log().Warn("            ! no part present");
            return false;
        }
        axis.MoveTo(kLoadPos);
        Log().Info("            axis @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Align", [&] {
        camera.Trigger();
        axis.MoveTo(kAlignPos);
        Log().Info("            camera #" + std::to_string(camera.Captures())
                   + "; axis @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Bond", [&] {
        io.Write(kVacuumDo, true); // pick
        Log().Info("            vacuum on");
        return true;
    });

    recipe->AddStep("Unload", [&] {
        axis.MoveTo(kUnloadPos);
        io.Write(kVacuumDo, false); // release
        Log().Info("            axis @ " + std::to_string(axis.Position())
                   + "; vacuum off");
        return true;
    });

    return recipe;
}

} // namespace oml::example::die_bonder
