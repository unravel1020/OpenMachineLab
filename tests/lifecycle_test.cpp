// lifecycle_test - functional coverage of the machine lifecycle.
// lifecycle_test - 机器生命周期的功能覆盖。
//
// Built on oml_test::TestBase. Covers the failure -> Fault -> Reset path, the
// module lifecycle hooks (via a RecordingModule), and the happy-path Run that
// blocks until Stop.
// 基于 oml_test::TestBase。覆盖：失败 → Fault → Reset 路径、模块生命周期钩子
// （通过 RecordingModule）、以及阻塞到 Stop 的 happy-path Run。
#include "oml_test.h"

#include "machine/Machine.h"
#include "module/Module.h"
#include "workflow/Workflow.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

using namespace oml;
using namespace oml::test;

namespace {

// A module that counts how often each lifecycle hook fires.
class RecordingModule : public Module {
public:
    std::string Name() const override { return "RecordingModule"; }

    void Configure()  override { ++configure; }
    void Initialize() override { ++initialize; }
    void Start()      override { ++start; }
    void Stop()       override { ++stop; }
    void Reset()      override { ++reset; }

    int configure = 0, initialize = 0, start = 0, stop = 0, reset = 0;
};

std::unique_ptr<Workflow> FailingWorkflow() {
    auto w = std::make_unique<Workflow>("w");
    w->AddStep("ok",   [] { return true; });
    w->AddStep("boom", [] { return false; });
    return w;
}

} // namespace

class LifecycleTest : public TestBase {
public:
    std::string Name() const override { return "lifecycle"; }

    void Run() override {
        SilentLog silence; // assert on State and counts, not on the trace

        // Failure path: Created -> Ready -> (failed run) Fault -> Reset -> Ready
        //                                                     -> Stopped.
        {
            Machine machine;
            Check(machine.State() == MachineState::Created, "starts Created");

            auto rec = std::make_unique<RecordingModule>();
            RecordingModule* rec_ptr = rec.get();
            machine.AddModule(std::move(rec));
            machine.AddWorkflow(FailingWorkflow());

            machine.Initialize();
            Check(machine.State() == MachineState::Ready, "Initialize -> Ready");
            Check(rec_ptr->configure == 1 && rec_ptr->initialize == 1,
                  "Configure + Initialize fired");

            machine.Run(); // returns on its own: the workflow fails
            Check(machine.State() == MachineState::Fault, "failed run -> Fault");
            Check(rec_ptr->start == 1 && rec_ptr->stop == 1,
                  "Start + Stop fired around the run");

            machine.Reset();
            Check(machine.State() == MachineState::Ready, "Reset -> Ready (via Recovering)");
            Check(rec_ptr->reset == 1, "Reset fired");

            machine.Shutdown();
            Check(machine.State() == MachineState::Stopped, "Shutdown -> Stopped");
        }

        // Happy path: Run blocks until Stop; a normal stop leaves state Running.
        {
            Machine machine;
            machine.AddModule(std::make_unique<RecordingModule>());

            auto wf = std::make_unique<Workflow>("w");
            wf->AddStep("ok", [] { return true; });
            machine.AddWorkflow(std::move(wf));

            machine.Initialize();
            Check(machine.State() == MachineState::Ready, "happy: Initialize -> Ready");

            std::thread stopper([&machine] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                machine.Stop();
            });
            machine.Run();
            stopper.join();
            Check(machine.State() == MachineState::Running,
                  "happy: normal stop leaves Running");

            machine.Shutdown();
            Check(machine.State() == MachineState::Stopped, "happy: Shutdown -> Stopped");
        }
    }
};

int main() {
    return RunAll(std::make_unique<LifecycleTest>());
}
