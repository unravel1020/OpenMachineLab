// event_test - the EventBus spine: Machine publishes StateChanged, subscribers
// receive it, and Unsubscribe stops delivery.
// event_test - EventBus 脊梁：Machine 发布 StateChanged，订阅者接收，
// Unsubscribe 停止投递。
#include "oml_test.h"

#include "event/Event.h"
#include "event/EventBus.h"
#include "machine/Machine.h"
#include "module/Module.h"

#include <cstddef>
#include <memory>
#include <variant>
#include <vector>

using namespace oml;
using namespace oml::test;

namespace {

class IdleModule : public Module {
public:
    std::string Name() const override { return "Idle"; }
};

} // namespace

class EventTest : public TestBase {
public:
    std::string Name() const override { return "event"; }

    void Run() override {
        SilentLog silence;

        Machine                       m;
        std::vector<StateChanged> seen;
        const int token = m.Bus().Subscribe([&](const Event& e) {
            if (const auto* sc = std::get_if<StateChanged>(&e)) seen.push_back(*sc);
        });
        Invariant(token >= 1, "Subscribe returns a token");

        m.AddModule(std::make_unique<IdleModule>());
        m.Initialize(); // Created -> Initializing -> Ready

        Invariant(seen.size() == 2, "two transitions published");
        Invariant(seen[0].from == MachineState::Created
                      && seen[0].to == MachineState::Initializing,
                  "first event: Created -> Initializing");
        Invariant(seen[1].to == MachineState::Ready, "second event -> Ready");

        // Unsubscribe stops delivery.
        m.Bus().Unsubscribe(token);
        const std::size_t before = seen.size();
        m.Shutdown(); // -> Stopping -> Stopped
        Invariant(seen.size() == before, "unsubscribed handler receives nothing");
    }
};

int main() {
    return RunAll(std::make_unique<EventTest>());
}
