#pragma once

#include "alarm/Alarm.h"
#include "event/EventBus.h"

#include <vector>

namespace oml {

// Tracks active alarms and publishes AlarmRaised/AlarmCleared on an EventBus.
// Alarms are the CAUSES of conditions; a Fault/Critical-severity alarm is what
// drives the machine into the Fault state (the machine asks HasFault()).
class AlarmManager {
public:
    explicit AlarmManager(EventBus* bus = nullptr) : bus_(bus) {}

    void SetBus(EventBus* bus) { bus_ = bus; }

    // Add an active alarm and publish AlarmRaised.
    void Raise(Alarm alarm);

    // Clear an active alarm by code and publish AlarmCleared. No-op if inactive.
    void Clear(int code);

    // Clear every active alarm, publishing AlarmCleared for each.
    void ClearAll();

    const std::vector<Alarm>& Active() const { return active_; }
    bool HasActive() const { return !active_.empty(); }

    // True if any active alarm is Fault or Critical severity.
    bool HasFault() const;

private:
    EventBus*         bus_ = nullptr;
    std::vector<Alarm> active_;
};

} // namespace oml
