#pragma once

#include "state/MachineState.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace oml {

class EventBus; // forward decl: History subscribes to it (see Attach)

// One recorded lifecycle event.
struct HistoryEntry {
    unsigned     seq;    // monotonic sequence number, starting at 1
    MachineState state;  // the state that was entered
    std::string  note;   // optional human-readable detail (e.g. a fault reason)
};

// An append-only journal of a machine's lifecycle events: state transitions and
// faults. It can be serialized to / parsed from a stream, so a device's history
// can be saved (audit/diagnostics) and reloaded. This is the persistence layer
// for "state/history" (ADR-0007's fault-history foundation).
//
// A History can Attach to an EventBus and journal every StateChanged itself, so
// the event bus is the single source of truth and Machine no longer journals
// directly.
class History {
public:
    History() = default;
    ~History();
    History(const History&)            = delete;
    History& operator=(const History&) = delete;

    // Append an event. Returns the assigned entry.
    const HistoryEntry& Record(MachineState state, std::string note = "");

    const std::vector<HistoryEntry>& Entries() const { return entries_; }
    std::size_t Size() const { return entries_.size(); }
    void Clear();

    // Text format, one event per line: "<seq> <StateName> <note...>".
    void Save(std::ostream& out) const;
    void Load(std::istream& in);

    // Subscribe to a bus: journal every StateChanged as it is published. Detaches
    // from any previous bus; auto-detaches on destruction. Lifetime rule: this
    // History must be destroyed (or Detached) before the bus it is attached to.
    void Attach(EventBus& bus);
    void Detach();

private:
    std::vector<HistoryEntry> entries_;
    unsigned                  next_seq_ = 1;
    EventBus*                 bus_   = nullptr;
    int                       token_ = 0;
};

} // namespace oml
