#pragma once

#include <functional>
#include <string>
#include <utility>

namespace oml {

// A single named action inside a workflow. The action is an arbitrary callable
// so a workflow stays decoupled from any specific module or resource — it just
// sequences work.
class Step {
public:
    using Action = std::function<void()>;

    Step(std::string name, Action action)
        : name_(std::move(name)), action_(std::move(action)) {}

    const std::string& Name() const { return name_; }

    void Execute() const {
        if (action_) action_();
    }

private:
    std::string name_;
    Action      action_;
};

} // namespace oml
