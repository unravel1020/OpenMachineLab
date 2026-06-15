#pragma once

#include "workflow/ActionRegistry.h"
#include "workflow/Workflow.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace oml {

// A serializable recipe description: a named sequence of steps, each referencing
// an action by name (resolved against an ActionRegistry when built). Unlike a
// runtime Workflow, it holds no callables, so it can be saved to / loaded from a
// file — the persistence layer for recipes.
// 可序列化的配方描述：命名步骤序列，每个步骤按名字引用一个动作（构建时通过
// ActionRegistry 解析）。与运行时 Workflow 不同，它不持有可调用对象，
// 因此可保存到/从文件加载——配方的持久化层。
struct StepSpec {
    std::string step;   // step label shown in the trace  trace 中显示的步骤标签
    std::string action; // name of the action to run (looked up in ActionRegistry) 要运行的动作名（在 ActionRegistry 中查找）
};

struct RecipeSpec {
    std::string           name;
    std::vector<StepSpec> steps;
};

// Text format:
//   line 1: <recipe name>
//   then one "<step> <action>" per line.
// 文本格式：第 1 行为配方名，之后每行一个 "<step> <action>"。
void SaveRecipe(std::ostream& out, const RecipeSpec& spec);

// Parse into `spec`. Returns false on malformed input (empty/missing name or a
// bad step line).
// 解析到 `spec`。输入格式错误时返回 false。
bool LoadRecipe(std::istream& in, RecipeSpec& spec);

// Build a runtime Workflow from a spec, resolving each action name against the
// registry. Returns nullptr if any action name is unknown (no partial build).
// 从 spec 构建运行时 Workflow，通过注册表解析每个动作名。
// 任何动作名未知时返回 nullptr（不半建）。
std::unique_ptr<Workflow> BuildWorkflow(const RecipeSpec& spec, const ActionRegistry& registry);

} // namespace oml
