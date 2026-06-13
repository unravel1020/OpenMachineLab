#pragma once

#include <iosfwd>
#include <map>
#include <string>

namespace oml {

// A device's configuration: named parameters (channels, axis positions, flags,
// ...) stored as key/value strings, with typed getters and Save/Load. This is
// the persistence layer for device configuration - a device reads its
// installation-specific values from a config instead of hardcoding them, so a
// deployment can be changed without recompiling.
class DeviceConfig {
public:
    void Set(const std::string& key, std::string value);
    bool Has(const std::string& key) const { return items_.count(key) != 0; }

    // Typed accessors; return `fallback` when the key is missing or unparseable.
    std::string GetString(const std::string& key, const std::string& fallback = "") const;
    long        GetLong(const std::string& key, long fallback = 0) const;
    bool        GetBool(const std::string& key, bool fallback = false) const;

    const std::map<std::string, std::string>& Items() const { return items_; }
    void Clear() { items_.clear(); }

    // Text format: one "key = value" per line. '#' comments and blank lines are
    // allowed (ignored on Load).
    void Save(std::ostream& out) const;
    void Load(std::istream& in);

private:
    std::map<std::string, std::string> items_;
};

} // namespace oml
