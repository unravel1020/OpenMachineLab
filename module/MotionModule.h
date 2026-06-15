#pragma once

#include "module/Module.h"
#include "resource/Axis.h"

#include <string>

namespace oml {

// A module that drives an axis. This is the Phase 2 step that gives a resource a
// reason to exist: a module owns (references) a resource and exercises it. On
// Initialize it homes its axis.
// 驱动轴的模块。这是第二阶段"让资源有存在意义"的步骤：模块持有（引用）一个资源
// 并驱动它。Initialize 时将轴回零。
class MotionModule : public Module {
public:
    explicit MotionModule(Axis& axis) : axis_(axis) {}

    std::string Name() const override { return "MotionModule"; }

    void Initialize() override { axis_.Home(); }

private:
    Axis& axis_;
};

} // namespace oml
