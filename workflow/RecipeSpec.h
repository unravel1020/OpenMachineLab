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
// file - the persistence layer for recipes.
struct StepSpec {
    std::string step;   // step label shown in the trace
    std::string action; // name of the action to run (looked up in ActionRegistry)
};

struct RecipeSpec {
    std::string           name;
    std::vector<StepSpec> steps;
};

// Text format:
//   line 1: <recipe name>
//   then one "<step> <action>" per line.
void SaveRecipe(std::ostream& out, const RecipeSpec& spec);

// Parse into `spec`. Returns false on malformed input (empty/missing name or a
// bad step line).
bool LoadRecipe(std::istream& in, RecipeSpec& spec);

// Build a runtime Workflow from a spec, resolving each action name against the
// registry. Returns nullptr if any action name is unknown (no partial build).
std::unique_ptr<Workflow> BuildWorkflow(const RecipeSpec& spec, const ActionRegistry& registry);

} // namespace oml
