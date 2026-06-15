#pragma once

#include "workflow/Step.h"

#include <string>
#include <utility>
#include <vector>

namespace oml {

// An ordered sequence of steps a machine executes. A workflow can model a
// lifecycle phase (Initialize / Run / Shutdown) or a product recipe
// (LoadFrame -> Align -> Bond -> Unload). Steps run in insertion order and the
// workflow stops at the first step that reports failure.
// 机器执行的有序步骤序列。工作流可以建模生命周期阶段（Initialize/Run/Shutdown）
// 或产品配方（LoadFrame → Align → Bond → Unload）。步骤按插入顺序执行，
// 工作流在第一个报告失败的步骤处停止。
class Workflow {
public:
    // Outcome of a run: success, or the name of the step that failed.
    // 运行结果：成功，或失败步骤的名字。
    struct Result {
        bool        success     = true;
        std::string failed_step; // empty unless success == false 除非失败否则为空
    };

    explicit Workflow(std::string name) : name_(std::move(name)) {}

    const std::string& Name() const { return name_; }

    // Append a named step. Steps run in insertion order.
    // 追加一个命名步骤。按插入顺序运行。
    void AddStep(std::string name, Step::Action action) {
        steps_.emplace_back(std::move(name), std::move(action));
    }

    // Execute every step in order, stopping at the first failure.
    // 按顺序执行每个步骤，遇到第一个失败即停止。
    Result Run() const;

private:
    std::string      name_;
    std::vector<Step> steps_;
};

} // namespace oml
