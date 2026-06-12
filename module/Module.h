#pragma once

#include <string>

namespace oml {

// A functional unit of a machine: loader, vision, bond head, wafer table, ...
//
// Phase 1 keeps the contract to a name plus an Initialize hook — the minimum
// the runtime needs to demonstrate the lifecycle. A richer interface
// (Configure, Start, Stop, Reset, ...) is deliberately deferred until a real
// device forces it out.
class Module {
public:
    virtual ~Module() = default;
    virtual std::string Name() const = 0;
    virtual void Initialize() {}
};

} // namespace oml
