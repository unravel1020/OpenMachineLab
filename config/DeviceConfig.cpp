#include "config/DeviceConfig.h"

#include <istream>
#include <ostream>
#include <utility>

namespace oml {

namespace {
std::string Trim(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return "";
    const auto b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}
} // namespace

void DeviceConfig::Set(const std::string& key, std::string value) {
    items_[key] = std::move(value);
}

std::string DeviceConfig::GetString(const std::string& key, const std::string& fallback) const {
    const auto it = items_.find(key);
    return it != items_.end() ? it->second : fallback;
}

long DeviceConfig::GetLong(const std::string& key, long fallback) const {
    const auto it = items_.find(key);
    if (it == items_.end()) return fallback;
    try {
        return std::stol(it->second);
    } catch (...) {
        return fallback; // not a number
    }
}

bool DeviceConfig::GetBool(const std::string& key, bool fallback) const {
    const auto it = items_.find(key);
    if (it == items_.end()) return fallback;
    const std::string& v = it->second;
    if (v == "true" || v == "1") return true;
    if (v == "false" || v == "0") return false;
    return fallback;
}

void DeviceConfig::Save(std::ostream& out) const {
    for (const auto& [key, value] : items_) {
        out << key << " = " << value << '\n';
    }
}

void DeviceConfig::Load(std::istream& in) {
    items_.clear();
    std::string line;
    while (std::getline(in, line)) {
        const std::string t = Trim(line);
        if (t.empty() || t[0] == '#') continue;
        const auto eq = t.find('=');
        if (eq == std::string::npos) continue; // not a key=value line
        const std::string key = Trim(t.substr(0, eq));
        const std::string val = Trim(t.substr(eq + 1));
        if (!key.empty()) items_[key] = val;
    }
}

} // namespace oml
