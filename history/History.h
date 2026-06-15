#pragma once

#include "state/MachineState.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace oml {

class EventBus; // forward decl: History subscribes to it (see Attach)
                // 前向声明：History 订阅它（见 Attach）

// One recorded lifecycle event.
// 一条记录的生命周期事件。
struct HistoryEntry {
    unsigned     seq;    // monotonic sequence number, starting at 1  单调序列号，从 1 开始
    MachineState state;  // the state that was entered  进入的状态
    std::string  note;   // optional human-readable detail (e.g. a fault reason) 可选的可读详情（如故障原因）
};

// An append-only journal of a machine's lifecycle events: state transitions and
// faults. It can be serialized to / parsed from a stream, so a device's history
// can be saved (audit/diagnostics) and reloaded. This is the persistence layer
// for "state/history" (ADR-0007's fault-history foundation).
// 机器生命周期事件的只追加日志：状态转换和故障。可序列化到流/从流解析，
// 使设备历史可保存（审计/诊断）和重新加载。这是"状态/历史"的持久化层
// （ADR-0007 的故障历史基础）。
//
// A History can Attach to an EventBus and journal every StateChanged itself, so
// the event bus is the single source of truth and Machine no longer journals
// directly.
// History 可 Attach 到 EventBus，自行记录每条 StateChanged，
// 使事件总线成为唯一事实源，Machine 不再直接记录。
class History {
public:
    History() = default;
    ~History();
    History(const History&)            = delete;
    History& operator=(const History&) = delete;

    // Append an event. Returns the assigned entry.
    // 追加一条事件。返回被分配的条目。
    const HistoryEntry& Record(MachineState state, std::string note = "");

    const std::vector<HistoryEntry>& Entries() const { return entries_; }
    std::size_t Size() const { return entries_.size(); }
    void Clear();

    // Text format, one event per line: "<seq> <StateName> <note...>".
    // 文本格式，每行一条："<seq> <StateName> <note...>"。
    void Save(std::ostream& out) const;
    void Load(std::istream& in);

    // Subscribe to a bus: journal every StateChanged as it is published. Detaches
    // from any previous bus; auto-detaches on destruction. Lifetime rule: this
    // History must be destroyed (or Detached) before the bus it is attached to.
    // 订阅总线：每条 StateChanged 发布时记录。先脱离之前的总线；析构时自动脱离。
    // 生命周期规则：此 History 必须在其总线之前销毁（或 Detach）。
    void Attach(EventBus& bus);
    void Detach();

private:
    std::vector<HistoryEntry> entries_;
    unsigned                  next_seq_ = 1;
    EventBus*                 bus_   = nullptr;
    int                       token_ = 0;
};

} // namespace oml
