#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace oml {

// Maps names to recipe actions (callables). A recipe references actions *by
// name*, so it can be serialized (names, not lambdas) and rebuilt at load time
// by looking the names up here. This is what makes recipes persistable.
// 将名字映射到配方动作（可调用对象）。配方按名字引用动作，因此可以序列化
// （名字，不是 lambda）并在加载时通过此处查找名字重建。这就是配方可持久化的关键。
class ActionRegistry {
public:
    using Action = std::function<bool()>; // true = success, false = fault the step true=成功，false=该步骤故障

    void Register(const std::string& name, Action action) {
        actions_[name] = std::move(action);
    }

    bool Has(const std::string& name) const { return actions_.count(name) != 0; }

    // The action for a name, or nullptr if not registered.
    // 按名字查找动作，未注册返回 nullptr。
    const Action* Find(const std::string& name) const {
        const auto it = actions_.find(name);
        return it != actions_.end() ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, Action> actions_;
};

} // namespace oml
