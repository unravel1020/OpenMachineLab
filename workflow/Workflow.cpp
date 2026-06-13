#include "workflow/Workflow.h"

#include <iostream>

namespace oml {

Workflow::Result Workflow::Run() const {
    Result result;
    std::cout << "    Workflow " << name_ << " Started\n";
    for (const Step& step : steps_) {
        std::cout << "        Step " << step.Name() << "\n";
        if (!step.Execute()) {
            std::cout << "        Step " << step.Name() << " FAILED\n";
            result.success     = false;
            result.failed_step = step.Name();
            break;
        }
    }
    return result;
}

} // namespace oml
