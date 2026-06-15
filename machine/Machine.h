#pragma once

#include "alarm/AlarmManager.h"
#include "event/EventBus.h"
#include "module/Module.h"
#include "resource/Resource.h"
#include "state/MachineState.h"
#include "workflow/Workflow.h"

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace oml {

// A single industrial equipment instance. This is the root of the runtime
// model: it owns resources, hosts modules, and executes workflows, all while
// tracking a minimal lifecycle state.
// 单台工业设备实例。这是运行模型的根：持有资源、装载模块、执行工作流，
// 同时跟踪一个最小化的生命周期状态。
//
// A machine is not a script: once Running it keeps processing one cycle after
// another until Stop() is requested from the outside (operator, host, or an
// error). Run() is therefore a loop, not a one-shot.
// 机器不是脚本：一旦进入 Running 就持续逐个 cycle 加工，直到外部（操作员、
// host 或错误）请求 Stop()。因此 Run() 是循环，不是一次性的。
//
// The lifecycle drives the modules: Configure+Initialize while Initializing,
// Start when entering Running, Stop when leaving it, and Reset during recovery.
// Every transition is published as a StateChanged event on the bus (Bus()); a
// History or any other subscriber can journal it.
// 生命周期驱动模块：Initializing 时 Configure+Initialize，进入 Running 时 Start，
// 离开 Running 时 Stop，恢复时 Reset。每次状态转换通过总线（Bus()）发布
// StateChanged 事件，History 或任意订阅者可以记录它。
class Machine {
public:
    Machine();

    // --- composition -------------------------------------------------------
    // --- 组装 ---------------------------------------------------------------
    void AddResource(std::unique_ptr<Resource> resource);
    void AddModule(std::unique_ptr<Module> module);
    void AddWorkflow(std::unique_ptr<Workflow> workflow);

    // --- lifecycle ---------------------------------------------------------
    // --- 生命周期 -----------------------------------------------------------
    // Initialize: Configure + bring resources/modules online, end in Ready.
    // Initialize：Configure + 资源/模块上线，最终进入 Ready。
    void Initialize();
    // Run: a loop. Starts the modules, executes the workflows once per cycle,
    // and keeps cycling until Stop() is requested, then Stops the modules.
    // Run：循环。启动模块，每个 cycle 执行一次工作流，持续循环直到收到
    // Stop()，然后停止模块。
    void Run();
    // Request the run loop to exit. Thread-safe (may be called from the
    // operator/host thread while Run blocks). Idempotent.
    // 请求运行循环退出。线程安全（可在 Run 阻塞时从操作员/host 线程调用）。幂等。
    void Stop();
    // Recover from a fault: Reset every module via the Recovering state and
    // return to Ready. Call this after the machine has entered Fault.
    // 从故障中恢复：通过 Recovering 状态 Reset 每个模块，返回 Ready。
    // 在机器进入 Fault 后调用。
    void Reset();
    // Shutdown: Stopping -> Stopped.
    // 关机：Stopping → Stopped。
    void Shutdown();

    MachineState State() const { return state_; }

    // Publish/subscribe bus for Events (state changes; alarms come later).
    // 事件发布/订阅总线（状态变更；告警事件后续扩展）。
    EventBus& Bus() { return bus_; }

    // Active alarms (the CAUSES of conditions); a Fault-severity alarm is what
    // drives the machine into the Fault state.
    // 活跃告警（状态的原因）；Fault 级告警驱动机器进入 Fault 状态。
    AlarmManager& Alarms() { return alarms_; }

private:
    // Apply a state change: annotate the journal (if any) and emit the trace.
    // 应用状态变更：标注日志（如有）并发出 trace。
    void TransitionTo(MachineState next, std::string note = "");

    MachineState                           state_ = MachineState::Created;
    // Sticky: Stop() sets it, only Reset() clears it (ADR-0009). Atomic so
    // Stop() may be called from a different thread than the one in Run().
    // 粘性：Stop() 置位，只有 Reset() 清除（ADR-0009）。原子操作，
    // Stop() 可从不同线程调用。
    std::atomic<bool>                      stop_requested_{false};
    EventBus                               bus_;
    AlarmManager                           alarms_{&bus_};
    std::vector<std::unique_ptr<Resource>> resources_;
    std::vector<std::unique_ptr<Module>>   modules_;
    std::vector<std::unique_ptr<Workflow>> workflows_;
};

} // namespace oml
