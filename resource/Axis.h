#pragma once

#include "resource/Resource.h"

#include <string>

namespace oml {

// A simulated linear axis (Phase 2 stub). There is no hardware behind it; it
// just tracks a logical position so the runtime model has a resource with a
// real, exercisable interface rather than a bare name.
class Axis : public Resource {
public:
    std::string Name() const override { return "Axis"; }

    // Park the axis at its home position.
    void Home() { position_ = 0; }
    // Command an absolute move, in counts.
    void MoveTo(long position) { position_ = position; }
    long Position() const { return position_; }

private:
    long position_ = 0;
};

} // namespace oml
