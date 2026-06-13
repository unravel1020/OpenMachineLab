#include "machine/Machine.h"

#include "log/Logger.h"

#include <chrono>
#include <string>
#include <thread>
#include <utility>

namespace oml {

// Time the machine spends on each production cycle. Lets a human watch the loop
// tick without flooding the console; it is not a scheduling primitive.
constexpr auto kCycleTick = std::chrono::milliseconds(700);

Machine::Machine() {
    Log().Info(ToString(state_));
}

void Machine::AddResource(std::unique_ptr<Resource> resource) {
    resources_.push_back(std::move(resource));
}

void Machine::AddModule(std::unique_ptr<Module> module) {
    modules_.push_back(std::move(module));
}

void Machine::AddWorkflow(std::unique_ptr<Workflow> workflow) {
    workflows_.push_back(std::move(workflow));
}

void Machine::Initialize() {
    TransitionTo(MachineState::Initializing);

    for (const auto& resource : resources_) {
        Log().Info("    Resource " + resource->Name());
    }
    for (const auto& module : modules_) {
        module->Configure();
        module->Initialize();
        Log().Info("    Module " + module->Name() + " Initialized");
    }

    TransitionTo(MachineState::Ready);
}

void Machine::Run() {
    TransitionTo(MachineState::Running);

    // Start every module as cyclic operation begins.
    for (const auto& module : modules_) {
        module->Start();
        Log().Info("    Module " + module->Name() + " Started");
    }

    // A real machine keeps working until it is told to stop, not just once.
    // Each iteration is one production cycle: the recipe runs, then the machine
    // either ticks again or exits. The loop ends on Stop() OR on a failing step.
    // stop_requested_ is sticky: Stop() sets it and only Reset() clears it, so a
    // stop signalled before or during Run is never lost (no reset-on-start race).
    unsigned long long cycle = 0;
    bool failed = false;
    while (!stop_requested_.load() && !failed) {
        ++cycle;
        Log().Info("    cycle " + std::to_string(cycle));
        for (const auto& workflow : workflows_) {
            const auto result = workflow->Run();
            if (!result.success) {
                Log().Warn("    workflow " + workflow->Name()
                           + " failed at step " + result.failed_step);
                failed = true;
                break;
            }
        }
        if (!failed) {
            std::this_thread::sleep_for(kCycleTick);
        }
    }

    if (failed) {
        Log().Warn("    entering Fault (call Reset() to recover)");
    } else {
        Log().Info("    stop requested after " + std::to_string(cycle) + " cycle(s)");
    }

    // Stop every module as cyclic operation ends.
    for (const auto& module : modules_) {
        module->Stop();
        Log().Info("    Module " + module->Name() + " Stopped");
    }

    if (failed) {
        TransitionTo(MachineState::Fault);
    }
}

void Machine::Stop() {
    stop_requested_.store(true);
}

void Machine::Reset() {
    stop_requested_.store(false); // allow a fresh Run after recovery
    TransitionTo(MachineState::Recovering);
    for (const auto& module : modules_) {
        module->Reset();
        Log().Info("    Module " + module->Name() + " Reset");
    }
    TransitionTo(MachineState::Ready);
}

void Machine::Shutdown() {
    TransitionTo(MachineState::Stopping);
    TransitionTo(MachineState::Stopped);
}

void Machine::TransitionTo(MachineState next) {
    state_ = next;
    Log().Info(" v");
    Log().Info(std::string{ToString(state_)});
}

} // namespace oml
