#pragma once

#include <functional>
#include <string>
#include <utility>

namespace oml {

// A single named action inside a workflow. The action returns bool: true on
// success, false on failure. A workflow stops at the first step that fails.
// 工作流中的一个命名动作。动作返回 bool：true = 成功，false = 失败。
// 工作流在第一个失败的步骤处停止。
class Step {
public:
    using Action = std::function<bool()>;

    Step(std::string name, Action action)
        : name_(std::move(name)), action_(std::move(action)) {}

    const std::string& Name() const { return name_; }

    // Run the action. Returns true on success (including when there is no
    // action), false if the action reports failure.
    // 运行动作。成功返回 true（包括没有动作时），失败返回 false。
    bool Execute() const { return action_ ? action_() : true; }

private:
    std::string name_;
    Action      action_;
};

} // namespace oml
