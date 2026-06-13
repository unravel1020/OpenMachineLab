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
    // checks whether Stop() has flipped the request flag and either ticks again
    // or exits.
    stop_requested_.store(false);
    unsigned long long cycle = 0;
    while (!stop_requested_.load()) {
        ++cycle;
        std::cout << "    cycle " << cycle << "\n";
        for (const auto& workflow : workflows_) {
            workflow->Run();
        }
        std::this_thread::sleep_for(kCycleTick);
    }
    std::cout << "    stop requested after " << cycle << " cycle(s)\n";

    // Stop every module as cyclic operation ends.
    for (const auto& module : modules_) {
        module->Stop();
        std::cout << "    Module " << module->Name() << " Stopped\n";
    }
}

void Machine::Stop() {
    stop_requested_.store(true);
}

void Machine::Reset() {
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
