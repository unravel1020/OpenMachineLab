#pragma once

#include "workflow/Step.h"

#include <string>
#include <utility>
#include <vector>

namespace oml {

// An ordered sequence of steps a machine executes. A workflow can model a
// lifecycle phase (Initialize / Run / Shutdown) or a product recipe
// (LoadFrame -> Align -> Bond -> Unload). Steps run in insertion order.
class Workflow {
public:
    explicit Workflow(std::string name) : name_(std::move(name)) {}

    const std::string& Name() const { return name_; }

    // Append a named step. Steps run in insertion order.
    void AddStep(std::string name, Step::Action action) {
        steps_.emplace_back(std::move(name), std::move(action));
    }

    // Execute every step in order. Called by the Machine when it Runs.
    void Run() const;

private:
    std::string      name_;
    std::vector<Step> steps_;
};

} // namespace oml
