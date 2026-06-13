#pragma once

#include "state/MachineState.h"

#include <string>
#include <variant>

namespace oml {

// A lifecycle transition: the machine moved `from` one state `to` another, with
// an optional note (e.g. a fault reason). Published by Machine::TransitionTo.
struct StateChanged {
    MachineState from;
    MachineState to;
    std::string  note;
};

// The set of things that can flow over the EventBus. Starts with lifecycle;
// alarm events (AlarmRaised/AlarmCleared) are added with the Alarm system.
using Event = std::variant<StateChanged>;

} // namespace oml
