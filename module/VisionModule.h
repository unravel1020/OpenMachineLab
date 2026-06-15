#pragma once

#include "module/Module.h"
#include "resource/Camera.h"

#include <string>

namespace oml {

// A module that owns a camera. On Initialize it opens the camera — again, a
// module exercising a real resource (Phase 2).
// 拥有相机的模块。Initialize 时打开相机——同样是模块驱动真实资源（第二阶段）。
class VisionModule : public Module {
public:
    explicit VisionModule(Camera& camera) : camera_(camera) {}

    std::string Name() const override { return "VisionModule"; }

    void Initialize() override { camera_.Open(); }

private:
    Camera& camera_;
};

} // namespace oml
