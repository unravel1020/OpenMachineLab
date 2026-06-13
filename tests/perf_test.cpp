// perf_test - simple performance measurements of the runtime model.
//
// Built on oml_test::TestBase. The Machine/Workflow trace is silenced so the
// numbers reflect CPU cost, not console I/O. Perf tests report numbers and
// assert only loose floors (so they never flake), to catch gross regressions.
#include "oml_test.h"

#include "machine/Machine.h"
#include "module/Module.h"
#include "workflow/Workflow.h"

#include <memory>

using namespace oml;
using namespace oml::test;

namespace {

// The cheapest possible module, for baseline measurements.
class IdleModule : public Module {
public:
    std::string Name() const override { return "IdleModule"; }
};

std::unique_ptr<Workflow> FourStepRecipe() {
    auto w = std::make_unique<Workflow>("Recipe");
    w->AddStep("LoadFrame", [] { return true; });
    w->AddStep("Align",     [] { return true; });
    w->AddStep("Bond",      [] { return true; });
    w->AddStep("Unload",    [] { return true; });
    return w;
}

} // namespace

class PerformanceTest : public TestBase {
public:
    std::string Name() const override { return "perf"; }

    void Run() override {
        constexpr int kIters = 2000;

        // Per-cycle recipe cost: Workflow::Run over 4 trivial steps.
        auto recipe  = FourStepRecipe();
        Workflow* wf = recipe.get();
        PerfResult workflow_run;
        {
            SilentCout silence;
            workflow_run = Benchmark(kIters, [wf] { wf->Run(); });
        }
        ReportPerf("Workflow::Run (4 steps)", workflow_run);
        Check(workflow_run.iterations == kIters && workflow_run.avg_ns > 0,
              "workflow benchmark completed");
        Check(workflow_run.per_second > 1000.0,
              "workflow throughput > 1k/s (loose floor)");

        // Startup cost: construct + Initialize a machine with two modules.
        // (Machine holds a std::atomic, so it is non-movable - build it in
        // place each iteration rather than returning one.)
        PerfResult initialize;
        {
            SilentCout silence;
            initialize = Benchmark(kIters, [] {
                Machine m;
                m.AddModule(std::make_unique<IdleModule>());
                m.AddModule(std::make_unique<IdleModule>());
                m.Initialize();
            });
        }
        ReportPerf("construct + Machine::Initialize (2 modules)", initialize);
        Check(initialize.avg_ns > 0, "initialize benchmark completed");
    }
};

int main() {
    return RunAll(std::make_unique<PerformanceTest>());
}
