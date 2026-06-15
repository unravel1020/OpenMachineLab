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
// 固晶机通道/位置的内置默认值。DeviceConfig 可覆盖任意一项
// （key 与下面的名字匹配）。DI 和 DO 是独立的线路空间；
// 通道 0 被 IoModule 保留（急停 / 指示灯）。
constexpr int  kPartPresentDi = 1; // DI: "part present at the load station" DI：工位有工件
constexpr int  kVacuumDo      = 1; // DO: vacuum pick DO：真空吸取
constexpr long kLoadPos       = 100;
constexpr long kAlignPos      = 200;
constexpr long kUnloadPos     = 300;

// The per-part recipe: axis through the stations, camera at align, vacuum
// pick/release. Channels/positions come from `cfg` (falling back to the
// constants above). LoadFrame fails (-> Fault) when no part is present.
// 单工件配方：轴经过各工位、相机对位、真空吸取/释放。通道/位置来自 `cfg`
// （回退到上面的常量）。LoadFrame 在无工件时失败（→ Fault）。
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
        io.Write(vacuum, true); // pick 吸取
        Log().Info("            vacuum on");
        return true;
    });

    recipe->AddStep("Unload", [&, vacuum, unload] {
        axis.MoveTo(unload);
        io.Write(vacuum, false); // release 释放
        Log().Info("            axis @ " + std::to_string(axis.Position())
                   + "; vacuum off");
        return true;
    });

    return recipe;
}

} // namespace oml::example::die_bonder
