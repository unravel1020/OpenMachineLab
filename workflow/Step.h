#pragma once

#include <functional>
#include <string>
#include <utility>

namespace oml {

// A single named action inside a workflow. The action returns bool: true on
// success, false on failure. A workflow stops at the first step that fails.
class Step {
public:
    using Action = std::function<bool()>;

    Step(std::string name, Action action)
        : name_(std::move(name)), action_(std::move(action)) {}

    const std::string& Name() const { return name_; }

    // Run the action. Returns true on success (including when there is no
    // action), false if the action reports failure.
    bool Execute() const { return action_ ? action_() : true; }

private:
    std::string name_;
    Action      action_;
};

} // namespace oml
