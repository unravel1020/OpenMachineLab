#pragma once

#include <string>

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

} // namespace oml
