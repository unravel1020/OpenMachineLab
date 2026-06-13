#pragma once

#include "device/Device.h"
#include "log/Logger.h"
#include "module/IoModule.h"
#include "module/MotionModule.h"
#include "resource/Axis.h"
#include "resource/DigitalIO.h"
#include "workflow/Workflow.h"

#include <memory>
#include <string>

namespace oml::example {

// Device-specific channels/positions for the pick-and-place. (DI and DO are
// separate line spaces; channel 0 is reserved by IoModule for e-stop / light.)
constexpr int  kPartAtFeederDi = 1; // DI: part available at the feeder
constexpr int  kVacuumDo       = 1; // DO: vacuum nozzle
constexpr long kFeederPos      = 0;
constexpr long kPlacePos       = 500;
constexpr long kHomePos        = 0;

inline std::unique_ptr<Workflow> BuildPickAndPlaceRecipe(Axis& axis, DigitalIO& io) {
    auto recipe = std::make_unique<Workflow>("PickAndPlace");

    recipe->AddStep("Pick", [&] {
        axis.MoveTo(kFeederPos);
        if (!io.Read(kPartAtFeederDi)) {
            Log().Warn("            ! no part at feeder");
            return false;
        }
        io.Write(kVacuumDo, true); // vacuum on
        Log().Info("            picked @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Transfer", [&] {
        axis.MoveTo(kPlacePos);
        Log().Info("            transferred @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Place", [&] {
        io.Write(kVacuumDo, false); // release
        Log().Info("            placed @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Home", [&] {
        axis.MoveTo(kHomePos);
        Log().Info("            homed @ " + std::to_string(axis.Position()));
        return true;
    });

    return recipe;
}

// A pick-and-place device profile: a gantry axis and a vacuum I/O block. A
// different resource/module set and a different recipe than the DieBonder, on
// the exact same model - proving the Device abstraction is device-agnostic.
class PickAndPlace : public Device {
public:
    PickAndPlace() : Device("PickAndPlace") {
        auto axis = std::make_unique<Axis>();
        auto io   = std::make_unique<DigitalIO>();
        Axis&      axis_ref = *axis;
        DigitalIO& io_ref   = *io;

        AddResource(std::move(axis));
        AddResource(std::move(io));

        AddModule(std::make_unique<MotionModule>(axis_ref));
        AddModule(std::make_unique<IoModule>(io_ref));

        io_ref.SimulateInput(kPartAtFeederDi, true); // part available
        AddWorkflow(BuildPickAndPlaceRecipe(axis_ref, io_ref));
    }
};

} // namespace oml::example
