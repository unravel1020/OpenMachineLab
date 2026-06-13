#include "alarm/AlarmManager.h"

#include <algorithm>

namespace oml {

void AlarmManager::Raise(Alarm alarm) {
    active_.push_back(alarm);
    if (bus_ != nullptr) bus_->Publish(AlarmRaised{alarm});
}

void AlarmManager::Clear(int code) {
    const auto it = std::find_if(active_.begin(), active_.end(),
                                 [code](const Alarm& a) { return a.code == code; });
    if (it == active_.end()) return;
    active_.erase(it);
    if (bus_ != nullptr) bus_->Publish(AlarmCleared{code});
}

void AlarmManager::ClearAll() {
    // Snapshot codes and clear first, then publish, so a re-entrant callback
    // (a handler that clears again) finds nothing to do.
    std::vector<int> codes;
    codes.reserve(active_.size());
    for (const auto& a : active_) codes.push_back(a.code);
    active_.clear();
    if (bus_ != nullptr) {
        for (const int code : codes) bus_->Publish(AlarmCleared{code});
    }
}

bool AlarmManager::HasFault() const {
    for (const auto& a : active_) {
        if (a.severity == Severity::Fault || a.severity == Severity::Critical) return true;
    }
    return false;
}

} // namespace oml
