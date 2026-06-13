#include "workflow/Workflow.h"

#include "log/Logger.h"

#include <string>

namespace oml {

Workflow::Result Workflow::Run() const {
    Result result;
    Log().Info("    Workflow " + name_ + " Started");
    for (const Step& step : steps_) {
        Log().Info("        Step " + step.Name());
        if (!step.Execute()) {
            Log().Warn("        Step " + step.Name() + " FAILED");
            result.success     = false;
            result.failed_step = step.Name();
            break;
        }
    }
    return result;
}

} // namespace oml
