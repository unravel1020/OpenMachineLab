#pragma once

#include "resource/Resource.h"

#include <string>
#include <unordered_map>

namespace oml {

// Simulated digital input/output (Phase 2 stub). No hardware behind it: the
// input and output lines are just bools in maps, so a module can exercise a
// real DI/DO interface. Where Axis is a position and Camera is an image,
// DigitalIO is the on/off signal lines that make up the bulk of a device's
// field I/O (sensors, solenoids, valves, interlocks, light tower, ...).
class DigitalIO : public Resource {
public:
    std::string Name() const override { return "DigitalIO"; }

    // --- outputs (DO): the device drives these ---------------------------
    void Write(int channel, bool on) { outputs_[channel] = on; }
    bool Output(int channel) const {
        const auto it = outputs_.find(channel);
        return it != outputs_.end() ? it->second : false;
    }

    // --- inputs (DI): the device reads these -----------------------------
    // Real inputs arrive from hardware; simulation/tests feed them via
    // SimulateInput.
    bool Read(int channel) const {
        const auto it = inputs_.find(channel);
        return it != inputs_.end() ? it->second : false;
    }
    void SimulateInput(int channel, bool on) { inputs_[channel] = on; }

private:
    std::unordered_map<int, bool> inputs_;
    std::unordered_map<int, bool> outputs_;
};

} // namespace oml
