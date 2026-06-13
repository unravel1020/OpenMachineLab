#pragma once

#include "alarm/AlarmManager.h"
#include "event/EventBus.h"
#include "module/Module.h"
#include "resource/Resource.h"
#include "state/MachineState.h"
#include "workflow/Workflow.h"

#include <atomic>
#include <memory>
#include <string>
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
// The lifecycle drives the modules: Configure+Initialize while Initializing,
// Start when entering Running, Stop when leaving it, and Reset during recovery.
// Every transition is published as a StateChanged event on the bus (Bus()); a
// History or any other subscriber can journal it.
class Machine {
public:
    Machine();

    // --- composition -------------------------------------------------------
    void AddResource(std::unique_ptr<Resource> resource);
    void AddModule(std::unique_ptr<Module> module);
    void AddWorkflow(std::unique_ptr<Workflow> workflow);

    // --- lifecycle ---------------------------------------------------------
    // Initialize: Configure + bring resources/modules online, end in Ready.
    void Initialize();
    // Run: a loop. Starts the modules, executes the workflows once per cycle,
    // and keeps cycling until Stop() is requested, then Stops the modules.
    void Run();
    // Request the run loop to exit. Thread-safe (may be called from the
    // operator/host thread while Run blocks). Idempotent.
    void Stop();
    // Recover from a fault: Reset every module via the Recovering state and
    // return to Ready. Call this after the machine has entered Fault.
    void Reset();
    // Shutdown: Stopping -> Stopped.
    void Shutdown();

    MachineState State() const { return state_; }

    // Publish/subscribe bus for Events (state changes; alarms come later).
    EventBus& Bus() { return bus_; }

    // Active alarms (the CAUSES of conditions); a Fault-severity alarm is what
    // drives the machine into the Fault state.
    AlarmManager& Alarms() { return alarms_; }

private:
    // Apply a state change: annotate the journal (if any) and emit the trace.
    void TransitionTo(MachineState next, std::string note = "");

    MachineState                           state_ = MachineState::Created;
    // Sticky: Stop() sets it, only Reset() clears it (ADR-0009). Atomic so
    // Stop() may be called from a different thread than the one in Run().
    std::atomic<bool>                      stop_requested_{false};
    EventBus                               bus_;
    AlarmManager                           alarms_{&bus_};
    std::vector<std::unique_ptr<Resource>> resources_;
    std::vector<std::unique_ptr<Module>>   modules_;
    std::vector<std::unique_ptr<Workflow>> workflows_;
};

} // namespace oml
