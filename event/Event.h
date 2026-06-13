#pragma once

#include "alarm/Alarm.h"
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

// An alarm became active. Carries the cause (the Alarm) - distinct from the
// StateChanged to Fault that a Fault-severity alarm may produce.
struct AlarmRaised {
    Alarm alarm;
};

// An alarm was cleared (by code), e.g. during Reset.
struct AlarmCleared {
    int code;
};

// The set of things that can flow over the EventBus.
using Event = std::variant<StateChanged, AlarmRaised, AlarmCleared>;

} // namespace oml
