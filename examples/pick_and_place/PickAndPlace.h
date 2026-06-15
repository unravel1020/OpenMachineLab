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

namespace oml::example::pick_and_place {

// Device-specific channels/positions for the pick-and-place. (DI and DO are
// separate line spaces; channel 0 is reserved by IoModule for e-stop / light.)
// 贴片机设备相关的通道/位置。（DI 和 DO 是独立的线路空间；
// 通道 0 被 IoModule 保留用于急停 / 指示灯。）
constexpr int  kPartAtFeederDi = 1; // DI: part available at the feeder DI：供料器有料
constexpr int  kVacuumDo       = 1; // DO: vacuum nozzle DO：真空吸嘴
constexpr long kFeederPos      = 0;
constexpr long kPlacePos       = 500;
constexpr long kHomePos        = 0;

inline std::unique_ptr<Workflow> BuildRecipe(Axis& axis, DigitalIO& io) {
    auto recipe = std::make_unique<Workflow>("PickAndPlace");

    recipe->AddStep("Pick", [&] {
        axis.MoveTo(kFeederPos);
        if (!io.Read(kPartAtFeederDi)) {
            Log().Warn("            ! no part at feeder");
            return false;
        }
        io.Write(kVacuumDo, true); // vacuum on 真空开
        Log().Info("            picked @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Transfer", [&] {
        axis.MoveTo(kPlacePos);
        Log().Info("            transferred @ " + std::to_string(axis.Position()));
        return true;
    });

    recipe->AddStep("Place", [&] {
        io.Write(kVacuumDo, false); // release 释放
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

} // namespace oml::example::pick_and_place

namespace oml::example {

// A pick-and-place device profile: a gantry axis and a vacuum I/O block. A
// different resource set and recipe than the DieBonder, on the same model.
// 贴片机设备 profile：龙门轴和真空 IO 模块。与 DieBonder 不同的资源集和配方，
// 跑在同一模型上。
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

        io_ref.SimulateInput(pick_and_place::kPartAtFeederDi, true); // part available 有料
        AddWorkflow(pick_and_place::BuildRecipe(axis_ref, io_ref));
    }
};

} // namespace oml::example
