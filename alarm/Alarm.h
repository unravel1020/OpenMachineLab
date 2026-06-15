#pragma once

#include <string>
#include <string_view>

namespace oml {

// Alarm severity. Warning (and below) only notifies; Fault and Critical drive the
// machine into the Fault state.
// 告警严重级别。Warning 及以下仅通知；Fault 和 Critical 驱动机器进入 Fault 状态。
enum class Severity { Info, Warning, Fault, Critical };

// An alarm: the CAUSE of a condition ("vacuum loss", "axis over-travel",
// "e-stop"), distinct from the machine STATE it may produce (e.g. Fault). One or
// more alarms can be active; clearing them (e.g. on Reset) lets the machine
// recover.
// 告警：状态的"原因"（"真空丢失""轴超程""急停"），不同于它可能产生的机器
// 状态（如 Fault）。一个或多个告警可同时活跃；清除它们（如 Reset 时）使机器恢复。
struct Alarm {
    int         code = 0;
    Severity    severity = Severity::Warning;
    std::string name;
    std::string message;
};

// Severity name and inverse, for persistence.
// 严重级别名称与逆转换，用于持久化。
inline std::string_view ToString(Severity s) {
    switch (s) {
        case Severity::Info:     return "Info";
        case Severity::Warning:  return "Warning";
        case Severity::Fault:    return "Fault";
        case Severity::Critical: return "Critical";
    }
    return "Warning";
}
// Named SeverityFromString (not FromString) to avoid a return-type-only overload
// clash with the MachineState FromString.
// 命名为 SeverityFromString（非 FromString），避免与 MachineState 的 FromString
// 产生"仅返回值类型不同"的重载冲突。
inline Severity SeverityFromString(std::string_view name) {
    if (name == "Info")     return Severity::Info;
    if (name == "Fault")    return Severity::Fault;
    if (name == "Critical") return Severity::Critical;
    return Severity::Warning;
}

} // namespace oml
