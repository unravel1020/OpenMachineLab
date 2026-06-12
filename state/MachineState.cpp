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
        case MachineState::Error:        return "Error";
    }
    return "Unknown";
}

} // namespace oml
