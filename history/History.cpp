#include "history/History.h"

#include "event/Event.h"
#include "event/EventBus.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <utility>
#include <variant>

namespace oml {

const HistoryEntry& History::Record(MachineState state, std::string note) {
    entries_.push_back({next_seq_++, state, std::move(note)});
    return entries_.back();
}

void History::Clear() {
    entries_.clear();
    next_seq_ = 1;
}

void History::Save(std::ostream& out) const {
    for (const auto& e : entries_) {
        out << e.seq << ' ' << ToString(e.state);
        if (!e.note.empty()) out << ' ' << e.note;
        out << '\n';
    }
}

void History::Load(std::istream& in) {
    Clear();
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        unsigned seq = 0;
        std::string state_name;
        if (!(ls >> seq >> state_name)) continue; // malformed line
        std::string note;
        std::getline(ls, note);
        const auto first = note.find_first_not_of(' ');
        if (first != std::string::npos) note.erase(0, first);
        else note.clear();

        entries_.push_back({seq, FromString(state_name), std::move(note)});
        if (seq >= next_seq_) next_seq_ = seq + 1; // keep sequence monotonic
    }
}

History::~History() { Detach(); }

void History::Attach(EventBus& bus) {
    Detach(); // leave any previous bus first
    bus_   = &bus;
    token_ = bus_->Subscribe([this](const Event& event) {
        if (const auto* sc = std::get_if<StateChanged>(&event)) {
            Record(sc->to, sc->note);
        }
    });
}

void History::Detach() {
    if (bus_ != nullptr) {
        bus_->Unsubscribe(token_);
        bus_   = nullptr;
        token_ = 0;
    }
}

} // namespace oml
