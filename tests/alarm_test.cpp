// alarm_test - alarms are the CAUSES of conditions; a Fault-severity alarm drives
// the machine into Fault; Reset clears them. Distinct from the Fault state.
#include "oml_test.h"

#include "alarm/Alarm.h"
#include "alarm/AlarmManager.h"
#include "event/Event.h"
#include "event/EventBus.h"
#include "machine/Machine.h"
#include "module/Module.h"
#include "workflow/Workflow.h"

#include <memory>
#include <sstream>
#include <variant>

using namespace oml;
using namespace oml::test;

namespace {

class IdleModule : public Module {
public:
    std::string Name() const override { return "Idle"; }
};

} // namespace

class AlarmTest : public TestBase {
public:
    std::string Name() const override { return "alarm"; }

    void Run() override {
        SilentLog silence;

        Machine m;
        int   raised = 0, cleared = 0;
        Alarm last_raised;
        m.Bus().Subscribe([&](const Event& e) {
            if (const auto* a = std::get_if<AlarmRaised>(&e)) {
                ++raised;
                last_raised = a->alarm;
            } else if (std::get_if<AlarmCleared>(&e) != nullptr) {
                ++cleared;
            }
        });

        m.AddModule(std::make_unique<IdleModule>());
        auto wf = std::make_unique<Workflow>("w");
        wf->AddStep("boom", [] { return false; });
        m.AddWorkflow(std::move(wf));

        m.Initialize();
        m.Run(); // step fails -> alarm raised -> Fault

        // The alarm is the cause; Fault is the resulting state.
        Invariant(raised == 1, "one alarm raised on step failure");
        Invariant(last_raised.severity == Severity::Fault, "alarm is Fault severity");
        Invariant(!last_raised.message.empty(), "alarm carries the failure message");
        Invariant(m.State() == MachineState::Fault, "machine entered Fault (the state)");
        Invariant(m.Alarms().HasFault(), "an active Fault-severity alarm exists");

        m.Reset(); // clears alarms -> Recovering -> Ready
        Invariant(cleared >= 1, "alarm cleared on Reset");
        Invariant(!m.Alarms().HasFault(), "no active fault after Reset");
        Invariant(m.State() == MachineState::Ready, "machine Ready after Reset");

        // The alarm log persists (Save/Load round-trip). Reset clears active
        // alarms, not the log, so the raised alarm is still recorded here.
        std::ostringstream out;
        m.Alarms().SaveLog(out);
        AlarmManager  reloaded;
        std::istringstream in(out.str());
        reloaded.LoadLog(in);
        Invariant(reloaded.Log().size() == m.Alarms().Log().size(), "alarm log round-trips");
        if (!m.Alarms().Log().empty()) {
            const auto& orig = m.Alarms().Log()[0].alarm;
            const auto& copy = reloaded.Log()[0].alarm;
            Invariant(orig.code == copy.code && orig.severity == copy.severity
                          && orig.name == copy.name && orig.message == copy.message,
                      "log entry fields preserved on reload");
        }
    }
};

int main() {
    return RunAll(std::make_unique<AlarmTest>());
}
