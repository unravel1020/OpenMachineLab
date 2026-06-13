#pragma once

#include "resource/Axis.h"
#include "resource/Camera.h"
#include "resource/DigitalIO.h"
#include "workflow/Workflow.h"

#include <iostream>
#include <memory>

namespace oml::example {

// Device-specific I/O channels and axis positions for the demo device. DI and
// DO are separate line spaces, so a DI channel and a DO channel may share a
// number (channel 0 is reserved by IoModule for e-stop / ready light).
constexpr int  kPartPresentDi = 1; // DI: "part present at the load station"
constexpr int  kVacuumDo      = 1; // DO: vacuum pick
constexpr long kLoadPos       = 100;
constexpr long kAlignPos      = 200;
constexpr long kUnloadPos     = 300;

// The per-part recipe for the demo device. Unlike the earlier no-op recipe, it
// actually drives the resources each cycle: the axis moves through the
// stations, the camera triggers at align, and the vacuum picks/releases.
//
// LoadFrame has a real precondition: it fails (and faults the machine) when no
// part is present. Feed one with io.SimulateInput(kPartPresentDi, true) for the
// happy path; withhold it to watch the machine fault.
inline std::unique_ptr<Workflow> BuildRecipe(Axis& axis, Camera& camera, DigitalIO& io) {
    auto recipe = std::make_unique<Workflow>("Recipe");

    recipe->AddStep("LoadFrame", [&] {
        if (!io.Read(kPartPresentDi)) {
            std::cout << "            ! no part present\n";
            return false;
        }
        axis.MoveTo(kLoadPos);
        std::cout << "            axis @ " << axis.Position() << "\n";
        return true;
    });

    recipe->AddStep("Align", [&] {
        camera.Trigger();
        axis.MoveTo(kAlignPos);
        std::cout << "            camera #" << camera.Captures()
                  << "; axis @ " << axis.Position() << "\n";
        return true;
    });

    recipe->AddStep("Bond", [&] {
        io.Write(kVacuumDo, true); // pick
        std::cout << "            vacuum on\n";
        return true;
    });

    recipe->AddStep("Unload", [&] {
        axis.MoveTo(kUnloadPos);
        io.Write(kVacuumDo, false); // release
        std::cout << "            axis @ " << axis.Position() << "; vacuum off\n";
        return true;
    });

    return recipe;
}

} // namespace oml::example
