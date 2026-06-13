#include "machine/Machine.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

namespace oml {

// Time the machine spends on each production cycle. Lets a human watch the loop
// tick without flooding the console; it is not a scheduling primitive.
constexpr auto kCycleTick = std::chrono::milliseconds(700);

Machine::Machine() {
    // The constructor prints the initial state so the trace always starts with
    // "Created" on its own line; every later change prepends a "v" marker.
    // ASCII-only on purpose: a Unicode arrow mojibakes on non-UTF-8 consoles.
    std::cout << ToString(state_) << "\n";
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
        std::cout << "    Resource " << resource->Name() << "\n";
    }
    for (const auto& module : modules_) {
        module->Configure();
        module->Initialize();
        std::cout << "    Module " << module->Name() << " Initialized\n";
    }

    TransitionTo(MachineState::Ready);
}

void Machine::Run() {
    TransitionTo(MachineState::Running);

    // Start every module as cyclic operation begins.
    for (const auto& module : modules_) {
        module->Start();
        std::cout << "    Module " << module->Name() << " Started\n";
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
        std::cout << "    cycle " << cycle << "\n";
        for (const auto& workflow : workflows_) {
            const auto result = workflow->Run();
            if (!result.success) {
                std::cout << "    workflow " << workflow->Name()
                          << " failed at step " << result.failed_step << "\n";
                failed = true;
                break;
            }
        }
        if (!failed) {
            std::this_thread::sleep_for(kCycleTick);
        }
    }

    if (failed) {
        std::cout << "    entering Fault (call Reset() to recover)\n";
    } else {
        std::cout << "    stop requested after " << cycle << " cycle(s)\n";
    }

    // Stop every module as cyclic operation ends.
    for (const auto& module : modules_) {
        module->Stop();
        std::cout << "    Module " << module->Name() << " Stopped\n";
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
        std::cout << "    Module " << module->Name() << " Reset\n";
    }
    TransitionTo(MachineState::Ready);
}

void Machine::Shutdown() {
    TransitionTo(MachineState::Stopping);
    TransitionTo(MachineState::Stopped);
}

void Machine::TransitionTo(MachineState next) {
    state_ = next;
    std::cout << " v\n" << ToString(state_) << "\n";
}

} // namespace oml
