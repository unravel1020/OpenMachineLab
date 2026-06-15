#include "alarm/AlarmManager.h"

#include <algorithm>
#include <istream>
#include <ostream>
#include <sstream>
#include <utility>

namespace oml {

namespace {
// Trim leading/trailing whitespace. 去除首尾空白。
std::string Trim(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return "";
    const auto b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}
} // namespace

void AlarmManager::Raise(Alarm alarm) {
    // Append to the log, add to active, then publish.
    // 追加到日志，添加到活跃，然后发布。
    log_.push_back({next_seq_++, alarm});
    active_.push_back(std::move(alarm));
    if (bus_ != nullptr) bus_->Publish(AlarmRaised{active_.back()});
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
    // 先快照 code 并清除，再发布，使重入回调（再次清除的 handler）无操作可做。
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

void AlarmManager::SaveLog(std::ostream& out) const {
    // Format: "<seq> <code> <Severity> <name> <message...>"
    // 格式："<seq> <code> <Severity> <name> <message...>"
    for (const auto& e : log_) {
        out << e.seq << ' ' << e.alarm.code << ' ' << ToString(e.alarm.severity)
            << ' ' << e.alarm.name;
        if (!e.alarm.message.empty()) out << ' ' << e.alarm.message;
        out << '\n';
    }
}

void AlarmManager::LoadLog(std::istream& in) {
    log_.clear();
    next_seq_ = 1;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        unsigned    seq = 0;
        int         code = 0;
        std::string sev, name;
        if (!(ls >> seq >> code >> sev >> name)) continue; // malformed line 格式错误
        std::string message;
        std::getline(ls, message);
        message = Trim(message);

        Alarm a;
        a.code     = code;
        a.severity = SeverityFromString(sev);
        a.name     = name;
        a.message  = message;
        log_.push_back({seq, std::move(a)});
        if (seq >= next_seq_) next_seq_ = seq + 1; // keep sequence monotonic 保持序列号单调递增
    }
}

} // namespace oml
