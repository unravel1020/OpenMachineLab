#include "workflow/Workflow.h"

#include <iostream>

namespace oml {

void Workflow::Run() const {
    std::cout << "    Workflow " << name_ << " Started\n";
    for (const Step& step : steps_) {
        std::cout << "        Step " << step.Name() << "\n";
        step.Execute();
    }
}

} // namespace oml
