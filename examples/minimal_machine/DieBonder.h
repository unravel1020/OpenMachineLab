#pragma once

#include "Recipe.h"
#include "config/DeviceConfig.h"
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
// by a module, running the load -> align -> bond -> unload recipe.
// 固晶机设备 profile：轴、相机和 IO 模块，各由一个模块驱动，
// 运行 load → align → bond → unload 配方。
//
// Channels/positions are read from a DeviceConfig (with built-in defaults), so a
// deployment can override them via a config file without recompiling. Omit the
// argument to use the defaults.
// 通道/位置从 DeviceConfig 读取（带内置默认值），部署可通过配置文件覆盖
// 而无需重编译。不传参数则使用默认值。
class DieBonder : public Device {
public:
    explicit DieBonder(const DeviceConfig& cfg = {}) : Device("DieBonder") {
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

        // Feed a part on the configured channel so LoadFrame succeeds.
        // 在配置的通道上喂入工件，使 LoadFrame 成功。
        const int part_di = static_cast<int>(cfg.GetLong("part_present_di", die_bonder::kPartPresentDi));
        io_ref.SimulateInput(part_di, true);
        AddWorkflow(die_bonder::BuildRecipe(axis_ref, camera_ref, io_ref, cfg));
    }
};

} // namespace oml::example
