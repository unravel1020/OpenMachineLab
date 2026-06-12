#include "machine/Machine.h"

#include <iostream>
#include <utility>

namespace oml {

Machine::Machine() {
    // The constructor prints the initial state so the trace always starts with
    // "Created" on its own line; every later change prepends the arrow.
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
        module->Initialize();
        std::cout << "    Module " << module->Name() << " Initialized\n";
    }

    TransitionTo(MachineState::Ready);
}

void Machine::Run() {
    TransitionTo(MachineState::Running);
    for (const auto& workflow : workflows_) {
        workflow->Run();
    }
}

void Machine::Shutdown() {
    TransitionTo(MachineState::Stopping);
    TransitionTo(MachineState::Stopped);
}

void Machine::TransitionTo(MachineState next) {
    state_ = next;
    std::cout << " \xe2\x86\x93\n" << ToString(state_) << "\n";
}

} // namespace oml
