#pragma once

#include "module/Module.h"
#include "resource/DigitalIO.h"

#include <string>

namespace oml {

// A module that owns the device's digital I/O block. Exercises the full Phase 2
// module lifecycle against a DigitalIO resource: Configure/Initialize set a
// safe state and sample the e-stop, Start/Stop drive the ready light, Reset
// re-samples the e-stop after a fault.
class IoModule : public Module {
public:
    explicit IoModule(DigitalIO& io) : io_(io) {}

    std::string Name() const override { return "IoModule"; }

    void Configure() override {} // no parameters in the stub

    void Initialize() override { SafeState(); }

    void Start() override { io_.Write(kReadyLight, true); }  // running -> light on
    void Stop() override  { io_.Write(kReadyLight, false); } // stopped -> light off

    void Reset() override { SafeState(); }

    bool EstopPressed() const { return estop_pressed_; }

private:
    // Outputs off, then sample the e-stop input. The known-safe baseline.
    void SafeState() {
        io_.Write(kReadyLight, false);
        estop_pressed_ = io_.Read(kEStop);
    }

    // Well-known lines on this device's I/O block. DI and DO are separate line
    // spaces, so a DI channel and a DO channel may share a number.
    static constexpr int kEStop      = 0; // DI: emergency stop
    static constexpr int kReadyLight = 0; // DO: "ready" indicator

    DigitalIO& io_;
    bool       estop_pressed_ = false;
};

} // namespace oml
