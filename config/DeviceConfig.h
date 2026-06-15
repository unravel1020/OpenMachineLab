#pragma once

#include <iosfwd>
#include <map>
#include <string>

namespace oml {

// A device's configuration: named parameters (channels, axis positions, flags,
// ...) stored as key/value strings, with typed getters and Save/Load. This is
// the persistence layer for device configuration — a device reads its
// installation-specific values from a config instead of hardcoding them, so a
// deployment can be changed without recompiling.
// 设备配置：命名参数（通道、轴位置、标志……）以 key/value 字符串存储，
// 带类型访问器和 Save/Load。这是设备配置的持久化层——设备从配置读取
// 装机相关值而非硬编码，使部署可变更而无需重编译。
class DeviceConfig {
public:
    void Set(const std::string& key, std::string value);
    bool Has(const std::string& key) const { return items_.count(key) != 0; }

    // Typed accessors; return `fallback` when the key is missing or unparseable.
    // 类型访问器；key 缺失或无法解析时返回 `fallback`。
    std::string GetString(const std::string& key, const std::string& fallback = "") const;
    long        GetLong(const std::string& key, long fallback = 0) const;
    bool        GetBool(const std::string& key, bool fallback = false) const;

    const std::map<std::string, std::string>& Items() const { return items_; }
    void Clear() { items_.clear(); }

    // Text format: one "key = value" per line. '#' comments and blank lines are
    // allowed (ignored on Load).
    // 文本格式：每行一个 "key = value"。允许 '#' 注释和空行（Load 时忽略）。
    void Save(std::ostream& out) const;
    void Load(std::istream& in);

private:
    std::map<std::string, std::string> items_;
};

} // namespace oml
