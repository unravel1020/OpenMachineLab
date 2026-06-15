#pragma once

#include "alarm/Alarm.h"
#include "state/MachineState.h"

#include <string>
#include <variant>

namespace oml {

// A lifecycle transition: the machine moved `from` one state `to` another, with
// an optional note (e.g. a fault reason). Published by Machine::TransitionTo.
// 生命周期转换：机器从 `from` 状态变到 `to` 状态，附带可选备注（如故障原因）。
// 由 Machine::TransitionTo 发布。
struct StateChanged {
    MachineState from;
    MachineState to;
    std::string  note;
};

// An alarm became active. Carries the cause (the Alarm) — distinct from the
// StateChanged to Fault that a Fault-severity alarm may produce.
// 告警变为活跃。携带原因（Alarm）——与 Fault 级告警可能产生的 StateChanged(to Fault)
// 是不同的概念。
struct AlarmRaised {
    Alarm alarm;
};

// An alarm was cleared (by code), e.g. during Reset.
// 告警被清除（按 code），如 Reset 期间。
struct AlarmCleared {
    int code;
};

// The set of things that can flow over the EventBus.
// EventBus 上可传播的事件集合。
using Event = std::variant<StateChanged, AlarmRaised, AlarmCleared>;

} // namespace oml
