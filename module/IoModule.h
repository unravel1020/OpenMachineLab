#pragma once

#include "module/Module.h"
#include "resource/DigitalIO.h"

#include <string>

namespace oml {

// A module that owns the device's digital I/O block. This is the Phase 2 step
// that gives DigitalIO a reason to exist: on Initialize it drives its outputs
// to a safe state and samples the emergency-stop input.
class IoModule : public Module {
public:
    explicit IoModule(DigitalIO& io) : io_(io) {}

    std::string Name() const override { return "IoModule"; }

    void Initialize() override {
        // Safe state at startup: controlled outputs off.
        io_.Write(kReadyLight, false);
        // Sample the e-stop input (true == pressed).
        estop_pressed_ = io_.Read(kEStop);
    }

    bool EstopPressed() const { return estop_pressed_; }

private:
    // Well-known lines on this device's I/O block. DI and DO are separate line
    // spaces, so a DI channel and a DO channel may share a number.
    static constexpr int kEStop      = 0; // DI: emergency stop
    static constexpr int kReadyLight = 0; // DO: "ready" indicator

    DigitalIO& io_;
    bool       estop_pressed_ = false;
};

} // namespace oml
