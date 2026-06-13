#pragma once

#include <string>
#include <string_view>

namespace oml {

// Alarm severity. Warning (and below) only notifies; Fault and Critical drive the
// machine into the Fault state.
enum class Severity { Info, Warning, Fault, Critical };

// An alarm: the CAUSE of a condition ("vacuum loss", "axis over-travel",
// "e-stop"), distinct from the machine STATE it may produce (e.g. Fault). One or
// more alarms can be active; clearing them (e.g. on Reset) lets the machine
// recover.
struct Alarm {
    int         code = 0;
    Severity    severity = Severity::Warning;
    std::string name;
    std::string message;
};

// Severity name and inverse, for persistence.
inline std::string_view ToString(Severity s) {
    switch (s) {
        case Severity::Info:     return "Info";
        case Severity::Warning:  return "Warning";
        case Severity::Fault:    return "Fault";
        case Severity::Critical: return "Critical";
    }
    return "Warning";
}
inline Severity SeverityFromString(std::string_view name) {
    if (name == "Info")     return Severity::Info;
    if (name == "Fault")    return Severity::Fault;
    if (name == "Critical") return Severity::Critical;
    return Severity::Warning;
}

} // namespace oml
