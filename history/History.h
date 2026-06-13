#pragma once

#include "state/MachineState.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace oml {

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
class History {
public:
    // Append an event. Returns the assigned entry.
    const HistoryEntry& Record(MachineState state, std::string note = "");

    const std::vector<HistoryEntry>& Entries() const { return entries_; }
    std::size_t Size() const { return entries_.size(); }
    void Clear();

    // Text format, one event per line: "<seq> <StateName> <note...>".
    void Save(std::ostream& out) const;
    void Load(std::istream& in);

private:
    std::vector<HistoryEntry> entries_;
    unsigned                  next_seq_ = 1;
};

} // namespace oml
