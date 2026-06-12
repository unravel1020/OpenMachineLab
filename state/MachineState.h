#pragma once

#include <string_view>

namespace oml {

// Minimal lifecycle of a machine.
//
// No state-machine framework is used: transitions are plain assignments
// guarded by Machine. Every consumer goes through ToString(), so adding a new
// state later does not require touching call sites.
enum class MachineState {
    Created,
    Initializing,
    Ready,
    Running,
    Stopping,
    Stopped,
    Fault,
    Paused,
    Recovering
};

// Human-readable name for a state, used by the runtime trace.
std::string_view ToString(MachineState state);

} // namespace oml
