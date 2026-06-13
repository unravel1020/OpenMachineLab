#pragma once

#include "workflow/Workflow.h"

#include <memory>

namespace oml::example {

// The per-part recipe for the demo device: load -> align -> bond -> unload. It
// is device-specific, so it stays in the example rather than the library.
inline std::unique_ptr<Workflow> BuildRecipe() {
    auto recipe = std::make_unique<Workflow>("Recipe");
    recipe->AddStep("LoadFrame", [] { return true; });
    recipe->AddStep("Align",     [] { return true; });
    recipe->AddStep("Bond",      [] { return true; });
    recipe->AddStep("Unload",    [] { return true; });
    return recipe;
}

} // namespace oml::example
