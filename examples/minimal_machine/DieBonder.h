#pragma once

#include "Recipe.h"
#include "device/Device.h"
#include "module/IoModule.h"
#include "module/MotionModule.h"
#include "module/VisionModule.h"
#include "resource/Axis.h"
#include "resource/Camera.h"
#include "resource/DigitalIO.h"

#include <memory>

namespace oml::example {

// A die-bonder device profile: an axis, a camera, and an I/O block, each driven
// by a module, running the load -> align -> bond -> unload recipe. A concrete
// device built on the generic model via the Device facade.
class DieBonder : public Device {
public:
    DieBonder() : Device("DieBonder") {
        auto axis   = std::make_unique<Axis>();
        auto camera = std::make_unique<Camera>();
        auto io     = std::make_unique<DigitalIO>();
        Axis&      axis_ref   = *axis;
        Camera&    camera_ref = *camera;
        DigitalIO& io_ref     = *io;

        AddResource(std::move(axis));
        AddResource(std::move(camera));
        AddResource(std::move(io));

        AddModule(std::make_unique<MotionModule>(axis_ref));
        AddModule(std::make_unique<VisionModule>(camera_ref));
        AddModule(std::make_unique<IoModule>(io_ref));

        // Feed a part so LoadFrame's precondition is satisfied.
        io_ref.SimulateInput(die_bonder::kPartPresentDi, true);
        AddWorkflow(die_bonder::BuildRecipe(axis_ref, camera_ref, io_ref));
    }
};

} // namespace oml::example
