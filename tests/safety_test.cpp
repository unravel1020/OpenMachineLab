// safety_test - invariants, edge cases, and thread-safety of the runtime model.
// safety_test - 运行模型的不变量、边界用例和线程安全。
//
// Built on oml_test::TestBase. Covers: legal state transitions, the empty
// machine, lifecycle edge cases (Initialize without Run), the fault/recovery
// contract, Stop() idempotency, and concurrent Stop() requests (the first
// concurrency-safety check - to be expanded in RoadMap Phase 3).
// 基于 oml_test::TestBase。覆盖：合法状态转换、空机、生命周期边界（不 Run 直接
// Initialize）、故障/恢复契约、Stop() 幂等性、以及并发 Stop() 请求
// （第一个并发安全检查——RoadMap Phase 3 扩展）。
#include "oml_test.h"

#include "machine/Machine.h"
#include "module/Module.h"
#include "workflow/Workflow.h"

#include <memory>

using namespace oml;
using namespace oml::test;

namespace {

class IdleModule : public Module {
public:
    std::string Name() const override { return "IdleModule"; }
};

std::unique_ptr<Workflow> OkWorkflow() {
    auto w = std::make_unique<Workflow>("w");
    w->AddStep("ok", [] { return true; });
    return w;
}

std::unique_ptr<Workflow> FailWorkflow() {
    auto w = std::make_unique<Workflow>("w");
    w->AddStep("boom", [] { return false; });
    return w;
}

} // namespace

class SafetyTest : public TestBase {
public:
    std::string Name() const override { return "safety"; }

    void Run() override {
        SilentLog silence; // we assert on State(), not on the trace

        // --- initial state ------------------------------------------------
        {
            Machine m;
            Invariant(m.State() == MachineState::Created, "fresh machine is Created");
        }

        // --- the empty machine still traverses the lifecycle ---------------
        {
            Machine m; // no resources, modules, or workflows
            m.Initialize();
            Invariant(m.State() == MachineState::Ready, "empty machine -> Ready");
            m.Shutdown();
            Invariant(m.State() == MachineState::Stopped, "empty machine -> Stopped");
        }

        // --- Initialize -> Shutdown without ever Running is legal ----------
        {
            Machine m;
            m.AddModule(std::make_unique<IdleModule>());
            m.Initialize();
            Invariant(m.State() == MachineState::Ready, "Ready after Initialize");
            m.Shutdown();
            Invariant(m.State() == MachineState::Stopped, "Stopped without Running");
        }

        // --- fault contract: a failing step faults; Reset recovers ---------
        {
            Machine m;
            m.AddModule(std::make_unique<IdleModule>());
            m.AddWorkflow(FailWorkflow());
            m.Initialize();
            m.Run(); // returns on its own: the step failed
            Invariant(m.State() == MachineState::Fault, "failed step -> Fault");
            m.Reset();
            Invariant(m.State() == MachineState::Ready, "Reset -> Ready after fault");
            m.Shutdown();
            Invariant(m.State() == MachineState::Stopped, "Faulted machine can still Shutdown");
        }

        // --- a succeeding run must never leave the machine in Fault --------
        {
            Machine m;
            m.AddModule(std::make_unique<IdleModule>());
            m.AddWorkflow(OkWorkflow());
            m.Initialize();
            // Drive Run from a thread; request Stop so it exits.
            std::thread runner([&] { m.Run(); });
            m.Stop();
            runner.join();
            Invariant(m.State() != MachineState::Fault, "successful run stays out of Fault");
            m.Shutdown();
            Invariant(m.State() == MachineState::Stopped, "-> Stopped after success");
        }

        // --- Stop() is idempotent and harmless -----------------------------
        {
            Machine m;
            m.AddModule(std::make_unique<IdleModule>());
            m.AddWorkflow(OkWorkflow());
            m.Initialize();
            for (int i = 0; i < 10; ++i) m.Stop(); // calling before Run just sets the flag
            std::thread runner([&] { m.Run(); });
            for (int i = 0; i < 10; ++i) m.Stop(); // and after, repeatedly
            runner.join();
            Invariant(m.State() == MachineState::Running, "repeated Stop leaves Running");
            m.Shutdown();
            Invariant(m.State() == MachineState::Stopped, "-> Stopped after idempotent Stop");
        }

        // --- concurrency: many threads may request Stop during Run ---------
        {
            Machine m;
            m.AddModule(std::make_unique<IdleModule>());
            m.AddWorkflow(OkWorkflow());
            m.Initialize();
            std::thread runner([&] { m.Run(); });
            RunConcurrently(8, [&m] { m.Stop(); }); // 8 threads race on the flag
            runner.join();
            Invariant(m.State() == MachineState::Running, "concurrent Stop leaves Running");
            m.Shutdown();
            Invariant(m.State() == MachineState::Stopped, "-> Stopped after concurrent Stop");
        }
    }
};

int main() {
    return RunAll(std::make_unique<SafetyTest>());
}
