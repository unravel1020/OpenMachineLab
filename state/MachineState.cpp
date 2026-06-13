#include "state/MachineState.h"

namespace oml {

std::string_view ToString(MachineState state) {
    switch (state) {
        case MachineState::Created:      return "Created";
        case MachineState::Initializing: return "Initializing";
        case MachineState::Ready:        return "Ready";
        case MachineState::Running:      return "Running";
        case MachineState::Stopping:     return "Stopping";
        case MachineState::Stopped:      return "Stopped";
        case MachineState::Fault:        return "Fault";
        case MachineState::Paused:       return "Paused";
        case MachineState::Recovering:   return "Recovering";
    }
    return "Unknown";
}

MachineState FromString(std::string_view name) {
    if (name == "Created")      return MachineState::Created;
    if (name == "Initializing") return MachineState::Initializing;
    if (name == "Ready")        return MachineState::Ready;
    if (name == "Running")      return MachineState::Running;
    if (name == "Stopping")     return MachineState::Stopping;
    if (name == "Stopped")      return MachineState::Stopped;
    if (name == "Fault")        return MachineState::Fault;
    if (name == "Paused")       return MachineState::Paused;
    if (name == "Recovering")   return MachineState::Recovering;
    return MachineState::Created; // unknown name
}

} // namespace oml
