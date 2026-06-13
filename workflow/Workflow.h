#pragma once

#include "workflow/Step.h"

#include <string>
#include <utility>
#include <vector>

namespace oml {

// An ordered sequence of steps a machine executes. A workflow can model a
// lifecycle phase (Initialize / Run / Shutdown) or a product recipe
// (LoadFrame -> Align -> Bond -> Unload). Steps run in insertion order and the
// workflow stops at the first step that reports failure.
class Workflow {
public:
    // Outcome of a run: success, or the name of the step that failed.
    struct Result {
        bool        success     = true;
        std::string failed_step; // empty unless success == false
    };

    explicit Workflow(std::string name) : name_(std::move(name)) {}

    const std::string& Name() const { return name_; }

    // Append a named step. Steps run in insertion order.
    void AddStep(std::string name, Step::Action action) {
        steps_.emplace_back(std::move(name), std::move(action));
    }

    // Execute every step in order, stopping at the first failure.
    Result Run() const;

private:
    std::string      name_;
    std::vector<Step> steps_;
};

} // namespace oml
