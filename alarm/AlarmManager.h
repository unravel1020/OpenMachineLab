#pragma once

#include "alarm/Alarm.h"
#include "event/EventBus.h"

#include <iosfwd>
#include <vector>

namespace oml {

// One entry in the alarm log: a sequence number + the alarm that was raised.
struct AlarmEntry {
    unsigned seq;
    Alarm    alarm;
};

// Tracks active alarms and publishes AlarmRaised/AlarmCleared on an EventBus.
// Alarms are the CAUSES of conditions; a Fault/Critical-severity alarm is what
// drives the machine into the Fault state (the machine asks HasFault()).
//
// Every raised alarm is also appended to a log (independent of active/cleared
// state), which can be saved/loaded for audit and diagnostics.
class AlarmManager {
public:
    explicit AlarmManager(EventBus* bus = nullptr) : bus_(bus) {}

    void SetBus(EventBus* bus) { bus_ = bus; }

    // Add an active alarm, append it to the log, and publish AlarmRaised.
    void Raise(Alarm alarm);

    // Clear an active alarm by code and publish AlarmCleared. No-op if inactive.
    // (The log is unaffected - it records history.)
    void Clear(int code);

    // Clear every active alarm, publishing AlarmCleared for each.
    void ClearAll();

    const std::vector<Alarm>& Active() const { return active_; }
    bool HasActive() const { return !active_.empty(); }

    // True if any active alarm is Fault or Critical severity.
    bool HasFault() const;

    // Every alarm ever raised, in order. Persistable.
    const std::vector<AlarmEntry>& Log() const { return log_; }
    // One "<seq> <code> <Severity> <name> <message...>" per line.
    void SaveLog(std::ostream& out) const;
    void LoadLog(std::istream& in);

private:
    EventBus*              bus_ = nullptr;
    std::vector<Alarm>      active_;
    std::vector<AlarmEntry> log_;
    unsigned               next_seq_ = 1;
};

} // namespace oml
