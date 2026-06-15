#include "workflow/RecipeSpec.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <utility>

namespace oml {

void SaveRecipe(std::ostream& out, const RecipeSpec& spec) {
    // Line 1: recipe name. Following lines: "<step> <action>".
    // 第 1 行：配方名。后续行："<step> <action>"。
    out << spec.name << '\n';
    for (const auto& s : spec.steps) {
        out << s.step << ' ' << s.action << '\n';
    }
}

bool LoadRecipe(std::istream& in, RecipeSpec& spec) {
    spec.steps.clear();
    // First line is the recipe name.
    // 第 1 行是配方名。
    if (!std::getline(in, spec.name) || spec.name.empty()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        StepSpec s;
        if (!(ls >> s.step >> s.action)) return false; // malformed step line 格式错误的步骤行
        spec.steps.push_back(std::move(s));
    }
    return true;
}

std::unique_ptr<Workflow> BuildWorkflow(const RecipeSpec& spec, const ActionRegistry& registry) {
    auto wf = std::make_unique<Workflow>(spec.name);
    for (const auto& s : spec.steps) {
        const ActionRegistry::Action* action = registry.Find(s.action);
        if (action == nullptr || !*action) return nullptr; // unknown / empty action 未知/空动作
        wf->AddStep(s.step, *action);
    }
    return wf;
}

} // namespace oml
