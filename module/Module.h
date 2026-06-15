#pragma once

#include <string>

namespace oml {

// A functional unit of a machine: loader, vision, bond head, wafer table, ...
// 机器的功能单元：上下料、视觉、键合头、晶圆台……
//
// The lifecycle hooks below are invoked by the Machine at the matching points of
// its lifecycle. Each defaults to no-op, so a module overrides only what it
// needs. Phase 2 generalizes the original single Initialize() hook into a small
// set; richer behavior is added only as concrete modules require it.
// 以下生命周期钩子由 Machine 在对应的生命周期点调用。每个默认空操作，
// 模块只覆盖需要的。第二阶段将原来的单个 Initialize() 扩展为一组钩子；
// 更丰富的行为仅在实际模块需要时才添加。
class Module {
public:
    virtual ~Module() = default;

    virtual std::string Name() const = 0;

    // --- lifecycle hooks (all optional) -----------------------------------
    // --- 生命周期钩子（全部可选）-------------------------------------------
    virtual void Configure() {}   // apply settings         (during Initializing)  应用设置（Initializing 阶段）
    virtual void Initialize() {}  // bring resource online  (during Initializing)  上线资源（Initializing 阶段）
    virtual void Start() {}       // begin cyclic operation (entering Running)      开始循环运行（进入 Running）
    virtual void Stop() {}        // cease operation        (leaving Running)      停止运行（离开 Running）
    virtual void Reset() {}       // clear a fault          (during Recovering)    清除故障（Recovering 阶段）
};

} // namespace oml
