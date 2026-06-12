#pragma once

#include "module/Module.h"
#include "resource/Resource.h"
#include "state/MachineState.h"
#include "workflow/Workflow.h"

#include <atomic>
#include <memory>
#include <vector>

namespace oml {

// A single industrial equipment instance. This is the root of the runtime
// model: it owns resources, hosts modules, and executes workflows, all while
// tracking a minimal lifecycle state.
//
// A machine is not a script: once Running it keeps processing one cycle after
// another until Stop() is requested from the outside (operator, host, or an
// error). Run() is therefore a loop, not a one-shot.
//
// Phase 1 deliberately avoids committing to any larger architecture — no event
// bus, no scheduler, no plugin system, no IoC container. The only goal is to
// prove that the model can run, not to simulate any specific device. Those
// abstractions are expected to grow out of real pain in later phases.
class Machine {
public:
    Machine();

    // --- composition -------------------------------------------------------
    void AddResource(std::unique_ptr<Resource> resource);
    void AddModule(std::unique_ptr<Module> module);
    void AddWorkflow(std::unique_ptr<Workflow> workflow);

    // --- lifecycle ---------------------------------------------------------
    // Initialize: bring resources/modules online, end in Ready.
    void Initialize();
    // Run: a loop, not a one-shot. Executes every workflow once per cycle and
    // keeps cycling until Stop() is requested. Blocks the calling thread.
    void Run();
    // Request the run loop to exit. Safe to call from another thread while Run
    // is blocking. Idempotent.
    void Stop();
    // Shutdown: Stopping -> Stopped.
    void Shutdown();

    MachineState State() const { return state_; }

private:
    // Apply a state change and emit it on the runtime trace.
    void TransitionTo(MachineState next);

    MachineState                           state_ = MachineState::Created;
    // Cleared at the start of Run(), set by Stop(). Atomic so Stop() may be
    // called from a different thread than the one blocked in Run().
    std::atomic<bool>                      stop_requested_{false};
    std::vector<std::unique_ptr<Resource>> resources_;
    std::vector<std::unique_ptr<Module>>   modules_;
    std::vector<std::unique_ptr<Workflow>> workflows_;
};

} // namespace oml
