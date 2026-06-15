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
// 运行所有步骤，在第一个失败处停止并记录失败步骤名。
// Run all steps; stop at the first failure and record the failed step name.
} // namespace oml
