#pragma once

#include "module/Module.h"
#include "resource/Axis.h"

#include <string>

namespace oml {

// A module that drives an axis. This is the Phase 2 step that gives a resource a
// reason to exist: a module owns (references) a resource and exercises it. On
// Initialize it homes its axis.
class MotionModule : public Module {
public:
    explicit MotionModule(Axis& axis) : axis_(axis) {}

    std::string Name() const override { return "MotionModule"; }

    void Initialize() override { axis_.Home(); }

private:
    Axis& axis_;
};

} // namespace oml
