#pragma once

#include <string>

namespace oml {

// A functional unit of a machine: loader, vision, bond head, wafer table, ...
//
// The lifecycle hooks below are invoked by the Machine at the matching points of
// its lifecycle. Each defaults to no-op, so a module overrides only what it
// needs. Phase 2 generalizes the original single Initialize() hook into a small
// set; richer behavior is added only as concrete modules require it.
class Module {
public:
    virtual ~Module() = default;

    virtual std::string Name() const = 0;

    // --- lifecycle hooks (all optional) -----------------------------------
    virtual void Configure() {}   // apply settings         (during Initializing)
    virtual void Initialize() {}  // bring resource online  (during Initializing)
    virtual void Start() {}       // begin cyclic operation (entering Running)
    virtual void Stop() {}        // cease operation        (leaving Running)
    virtual void Reset() {}       // clear a fault          (during Recovering)
};

} // namespace oml
