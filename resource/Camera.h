#pragma once

#include "resource/Resource.h"

#include <string>

namespace oml {

// A simulated camera (Phase 2 stub). Tracks only whether it has been opened, so
// a module can exercise a real resource interface.
class Camera : public Resource {
public:
    std::string Name() const override { return "Camera"; }

    void Open() { open_ = true; }
    void Close() { open_ = false; }
    bool IsOpen() const { return open_; }

    // Simulate grabbing a frame (e.g. for alignment). Counts captures so a
    // recipe/observer can see the camera being used during production.
    void Trigger() { ++captures_; }
    long Captures() const { return captures_; }

private:
    bool open_      = false;
    long captures_  = 0;
};

} // namespace oml
