#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace oml {

// Maps names to recipe actions (callables). A recipe references actions *by
// name*, so it can be serialized (names, not lambdas) and rebuilt at load time
// by looking the names up here. This is what makes recipes persistable.
class ActionRegistry {
public:
    using Action = std::function<bool()>; // true = success, false = fault the step

    void Register(const std::string& name, Action action) {
        actions_[name] = std::move(action);
    }

    bool Has(const std::string& name) const { return actions_.count(name) != 0; }

    // The action for a name, or nullptr if not registered.
    const Action* Find(const std::string& name) const {
        const auto it = actions_.find(name);
        return it != actions_.end() ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, Action> actions_;
};

} // namespace oml
