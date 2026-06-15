#include "machine/Machine.h"

#include "log/Logger.h"

#include <chrono>
#include <string>
#include <thread>
#include <utility>

namespace oml {

// Time the machine spends on each production cycle. Lets a human watch the loop
// tick without flooding the console; it is not a scheduling primitive.
// 每个 cycle 的间隔时间。让人能看清循环节奏，不刷屏控制台；不是调度原语。
constexpr auto kCycleTick        = std::chrono::milliseconds(700);
constexpr int  kAlarmProcessFail = 1001; // raised when a workflow step fails 步骤失败时产生

Machine::Machine() {
    // The constructor prints the initial state so the trace always starts with
    // "Created" on its own line.
    // 构造函数打印初始状态，trace 总是以 "Created" 开头。
    Log().Info(ToString(state_));
}

void Machine::AddResource(std::unique_ptr<Resource> resource) {
    resources_.push_back(std::move(resource));
}

void Machine::AddModule(std::unique_ptr<Module> module) {
    modules_.push_back(std::move(module));
}

void Machine::AddWorkflow(std::unique_ptr<Workflow> workflow) {
    workflows_.push_back(std::move(workflow));
}

void Machine::Initialize() {
    TransitionTo(MachineState::Initializing);

    for (const auto& resource : resources_) {
        Log().Info("    Resource " + resource->Name());
    }
    for (const auto& module : modules_) {
        module->Configure();
        module->Initialize();
        Log().Info("    Module " + module->Name() + " Initialized");
    }

    TransitionTo(MachineState::Ready);
}

void Machine::Run() {
    TransitionTo(MachineState::Running);

    // Start every module as cyclic operation begins.
    // 循环运行开始时启动每个模块。
    for (const auto& module : modules_) {
        module->Start();
        Log().Info("    Module " + module->Name() + " Started");
    }

    // Each iteration is one production cycle: the recipe runs, then the machine
    // either ticks again or exits. The loop ends on Stop() OR a failing step.
    // stop_requested_ is sticky: Stop() sets it, only Reset() clears it, so a
    // stop signalled before or during Run is never lost (no reset-on-start race,
    // ADR-0009).
    // 每次迭代是一个 cycle：运行配方，然后继续或退出。循环在 Stop() 或步骤失败时结束。
    // stop_requested_ 是粘性的：Stop() 置位，只有 Reset() 清除，
    // 确保停止信号不丢失（无 reset-on-start 竞态，ADR-0009）。
    unsigned long long cycle = 0;
    bool        failed = false;
    std::string fault_reason;
    while (!stop_requested_.load() && !failed) {
        ++cycle;
        Log().Info("    cycle " + std::to_string(cycle));
        for (const auto& workflow : workflows_) {
            const auto result = workflow->Run();
            if (!result.success) {
                // Raise an alarm (the CAUSE) then enter Fault (the STATE).
                // 产生告警（因），然后进入 Fault（果）。
                fault_reason = "workflow " + workflow->Name()
                               + " failed at step " + result.failed_step;
                alarms_.Raise({kAlarmProcessFail, Severity::Fault, "PROCESS_FAIL", fault_reason});
                Log().Warn("    " + fault_reason);
                failed = true;
                break;
            }
        }
        if (!failed) {
            std::this_thread::sleep_for(kCycleTick);
        }
    }

    if (failed) {
        Log().Warn("    entering Fault (call Reset() to recover)");
    } else {
        Log().Info("    stop requested after " + std::to_string(cycle) + " cycle(s)");
    }

    // Stop every module as cyclic operation ends.
    // 循环结束时停止每个模块。
    for (const auto& module : modules_) {
        module->Stop();
        Log().Info("    Module " + module->Name() + " Stopped");
    }

    if (failed) {
        TransitionTo(MachineState::Fault, fault_reason);
    }
}

void Machine::Stop() {
    // Sticky stop — only Reset() clears it (ADR-0009).
    // 粘性停止——只有 Reset() 清除（ADR-0009）。
    stop_requested_.store(true);
}

void Machine::Reset() {
    stop_requested_.store(false); // allow a fresh Run after recovery 恢复后允许重新 Run
    alarms_.ClearAll();           // clear the causes before recovering 恢复前清除告警原因
    TransitionTo(MachineState::Recovering);
    for (const auto& module : modules_) {
        module->Reset();
        Log().Info("    Module " + module->Name() + " Reset");
    }
    TransitionTo(MachineState::Ready);
}

void Machine::Shutdown() {
    TransitionTo(MachineState::Stopping);
    TransitionTo(MachineState::Stopped);
}

void Machine::TransitionTo(MachineState next, std::string note) {
    // Publish StateChanged on the bus (the spine for all observers), then log.
    // 在总线上发布 StateChanged（所有观察者的脊梁），然后记录日志。
    const MachineState from = state_;
    state_ = next;
    bus_.Publish(StateChanged{from, next, std::move(note)});
    Log().Info(" v");
    Log().Info(std::string{ToString(state_)});
}

} // namespace oml
