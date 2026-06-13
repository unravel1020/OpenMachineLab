#include "workflow/RecipeSpec.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <utility>

namespace oml {

void SaveRecipe(std::ostream& out, const RecipeSpec& spec) {
    out << spec.name << '\n';
    for (const auto& s : spec.steps) {
        out << s.step << ' ' << s.action << '\n';
    }
}

bool LoadRecipe(std::istream& in, RecipeSpec& spec) {
    spec.steps.clear();
    if (!std::getline(in, spec.name) || spec.name.empty()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        StepSpec s;
        if (!(ls >> s.step >> s.action)) return false; // malformed step line
        spec.steps.push_back(std::move(s));
    }
    return true;
}

std::unique_ptr<Workflow> BuildWorkflow(const RecipeSpec& spec, const ActionRegistry& registry) {
    auto wf = std::make_unique<Workflow>(spec.name);
    for (const auto& s : spec.steps) {
        const ActionRegistry::Action* action = registry.Find(s.action);
        if (action == nullptr || !*action) return nullptr; // unknown / empty action
        wf->AddStep(s.step, *action);
    }
    return wf;
}

} // namespace oml
