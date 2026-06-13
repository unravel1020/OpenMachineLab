// lifecycle_test - a tiny, framework-free harness around the machine lifecycle.
//
// Covers the state transitions and the Phase 2 paths: a failing workflow step
// sends the machine to Fault, and Reset() recovers through Recovering back to
// Ready. A RecordingModule verifies the lifecycle hooks (Configure/Start/Stop/
// Reset) actually fire.
#include "machine/Machine.h"
#include "module/Module.h"
#include "workflow/Workflow.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace oml;

namespace {

int g_failures = 0;

void check(bool condition, const std::string& msg) {
    if (condition) {
        std::cout << "  ok:   " << msg << "\n";
    } else {
        std::cout << "  FAIL: " << msg << "\n";
        ++g_failures;
    }
}

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

// A workflow that fails on its "boom" step.
std::unique_ptr<Workflow> FailingWorkflow() {
    auto wf = std::make_unique<Workflow>("w");
    wf->AddStep("ok",   [] { return true; });
    wf->AddStep("boom", [] { return false; });
    return wf;
}

void test_failure_path() {
    std::cout << "[test] failure -> Fault -> Reset -> Ready -> Stopped\n";
    Machine machine;
    check(machine.State() == MachineState::Created, "starts Created");

    auto rec = std::make_unique<RecordingModule>();
    RecordingModule* rec_ptr = rec.get();
    machine.AddModule(std::move(rec));
    machine.AddWorkflow(FailingWorkflow());

    machine.Initialize();
    check(machine.State() == MachineState::Ready, "Initialize -> Ready");
    check(rec_ptr->configure == 1 && rec_ptr->initialize == 1,
          "Configure + Initialize fired");

    machine.Run(); // returns on its own: the workflow fails
    check(machine.State() == MachineState::Fault, "failed run -> Fault");
    check(rec_ptr->start == 1 && rec_ptr->stop == 1,
          "Start + Stop fired around the run");

    machine.Reset();
    check(machine.State() == MachineState::Ready, "Reset -> Ready (via Recovering)");
    check(rec_ptr->reset == 1, "Reset fired");

    machine.Shutdown();
    check(machine.State() == MachineState::Stopped, "Shutdown -> Stopped");
}

void test_happy_path() {
    std::cout << "[test] happy path: Run blocks until Stop\n";
    Machine machine;
    machine.AddModule(std::make_unique<RecordingModule>());

    auto wf = std::make_unique<Workflow>("w");
    wf->AddStep("ok", [] { return true; });
    machine.AddWorkflow(std::move(wf));

    machine.Initialize();
    check(machine.State() == MachineState::Ready, "Initialize -> Ready");

    // Run() blocks until Stop() is requested; ask for it from another thread.
    std::thread stopper([&machine] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        machine.Stop();
    });
    machine.Run();
    stopper.join();
    check(machine.State() == MachineState::Running, "normal stop leaves state Running");

    machine.Shutdown();
    check(machine.State() == MachineState::Stopped, "Shutdown -> Stopped");
}

} // namespace

int main() {
    test_failure_path();
    test_happy_path();

    if (g_failures == 0) {
        std::cout << "\nall tests passed\n";
        return 0;
    }
    std::cout << "\n" << g_failures << " test(s) FAILED\n";
    return 1;
}
