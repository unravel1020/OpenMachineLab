#pragma once

#include "alarm/Alarm.h"
#include "event/EventBus.h"

#include <iosfwd>
#include <vector>

namespace oml {

// One entry in the alarm log: a sequence number + the alarm that was raised.
// 告警日志中的一条记录：序列号 + 被产生的告警。
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
// 跟踪活跃告警并在 EventBus 上发布 AlarmRaised/AlarmCleared。
// 告警是状态的"原因"；Fault/Critical 级告警驱动机器进入 Fault 状态
// （机器询问 HasFault()）。
// 每条产生的告警也追加到日志（独立于活跃/清除状态），可 Save/Load 用于审计和诊断。
class AlarmManager {
public:
    explicit AlarmManager(EventBus* bus = nullptr) : bus_(bus) {}

    void SetBus(EventBus* bus) { bus_ = bus; }

    // Add an active alarm, append it to the log, and publish AlarmRaised.
    // 添加活跃告警，追加到日志，并发布 AlarmRaised。
    void Raise(Alarm alarm);

    // Clear an active alarm by code and publish AlarmCleared. No-op if inactive.
    // (The log is unaffected — it records history.)
    // 按 code 清除活跃告警并发布 AlarmCleared。不活跃则无操作。
    // （日志不受影响——它记录历史。）
    void Clear(int code);

    // Clear every active alarm, publishing AlarmCleared for each.
    // 清除所有活跃告警，为每条发布 AlarmCleared。
    void ClearAll();

    const std::vector<Alarm>& Active() const { return active_; }
    bool HasActive() const { return !active_.empty(); }

    // True if any active alarm is Fault or Critical severity.
    // 是否有活跃的 Fault 或 Critical 级告警。
    bool HasFault() const;

    // Every alarm ever raised, in order. Persistable.
    // 所有产生过的告警，按顺序。可持久化。
    const std::vector<AlarmEntry>& Log() const { return log_; }
    // One "<seq> <code> <Severity> <name> <message...>" per line.
    // 每行格式："<seq> <code> <Severity> <name> <message...>"
    void SaveLog(std::ostream& out) const;
    void LoadLog(std::istream& in);

private:
    EventBus*              bus_ = nullptr;
    std::vector<Alarm>      active_;
    std::vector<AlarmEntry> log_;
    unsigned               next_seq_ = 1;
};

} // namespace oml
