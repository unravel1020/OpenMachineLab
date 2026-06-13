// persistence_test - save/load round-trips for state/history and recipes.
//
// Built on oml_test::TestBase. Covers:
//   - History: a machine journals transitions + fault reason; Save -> Load
//     reproduces every entry.
//   - Recipe: an ActionRegistry + RecipeSpec; Save -> Load -> BuildWorkflow
//     reproduces the recipe and runs it; an unknown action name yields no build.
#include "oml_test.h"

#include "config/DeviceConfig.h"
#include "history/History.h"
#include "machine/Machine.h"
#include "module/Module.h"
#include "workflow/ActionRegistry.h"
#include "workflow/RecipeSpec.h"
#include "workflow/Workflow.h"

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>

using namespace oml;
using namespace oml::test;

namespace {

class IdleModule : public Module {
public:
    std::string Name() const override { return "IdleModule"; }
};

} // namespace

class PersistenceTest : public TestBase {
public:
    std::string Name() const override { return "persistence"; }

    void Run() override {
        TestHistoryRoundTrip();
        TestRecipeRoundTrip();
        TestConfigRoundTrip();
    }

private:
    void TestHistoryRoundTrip() {
        SilentLog silence;

        // Declare Machine before History: destruction is reverse, so History
        // detaches (in its dtor) while the bus is still alive.
        Machine  m;
        History  hist;
        hist.Attach(m.Bus()); // journal StateChanged from the bus
        m.AddModule(std::make_unique<IdleModule>());

        auto wf = std::make_unique<Workflow>("w");
        wf->AddStep("ok",   [] { return true; });
        wf->AddStep("boom", [] { return false; });
        m.AddWorkflow(std::move(wf));

        m.Initialize(); // -> Initializing, Ready
        m.Run();        // -> Running, then Fault (boom fails)
        m.Reset();      // -> Recovering, Ready
        m.Shutdown();   // -> Stopping, Stopped

        Invariant(hist.Size() > 0, "history recorded events");

        const HistoryEntry* fault = nullptr;
        for (const auto& e : hist.Entries()) {
            if (e.state == MachineState::Fault) fault = &e;
        }
        Invariant(fault != nullptr, "a Fault event was recorded");
        Invariant(fault->note.find("boom") != std::string::npos,
                  "fault note names the failed step");

        std::ostringstream out;
        hist.Save(out);
        History reloaded;
        std::istringstream in(out.str());
        reloaded.Load(in);

        Invariant(reloaded.Size() == hist.Size(), "reload preserves event count");
        const auto& a = hist.Entries();
        const auto& b = reloaded.Entries();
        bool match = a.size() == b.size();
        for (std::size_t i = 0; match && i < a.size(); ++i) {
            match = a[i].seq == b[i].seq && a[i].state == b[i].state && a[i].note == b[i].note;
        }
        Invariant(match, "reload preserves seq/state/note of every entry");
    }

    void TestRecipeRoundTrip() {
        SilentLog silence;

        ActionRegistry registry;
        int bond_count = 0;
        registry.Register("load",   [] { return true; });
        registry.Register("align",  [] { return true; });
        registry.Register("bond",   [&] { ++bond_count; return true; });
        registry.Register("unload", [] { return true; });

        const RecipeSpec spec{"Recipe",
                              {{"LoadFrame", "load"},
                               {"Align",     "align"},
                               {"Bond",      "bond"},
                               {"Unload",    "unload"}}};

        // Save -> Load preserves the spec.
        std::ostringstream out;
        SaveRecipe(out, spec);
        RecipeSpec loaded;
        std::istringstream in(out.str());
        Invariant(LoadRecipe(in, loaded), "recipe loaded");
        Invariant(loaded.name == spec.name && loaded.steps.size() == spec.steps.size(),
                  "reload preserves name + step count");
        Invariant(loaded.steps[2].action == "bond", "reload preserves action names");

        // Build a runtime workflow from the spec + registry and run it.
        auto wf = BuildWorkflow(loaded, registry);
        Invariant(wf != nullptr, "workflow built from spec + registry");
        const auto result = wf->Run();
        Invariant(result.success, "built workflow runs successfully");
        Invariant(bond_count == 1, "named 'bond' action executed");

        // An unknown action name must not produce a partial workflow.
        const RecipeSpec bad{"Bad", {{"X", "does_not_exist"}}};
        Invariant(BuildWorkflow(bad, registry) == nullptr,
                  "unknown action name -> no workflow");
    }

    void TestConfigRoundTrip() {
        DeviceConfig cfg;
        cfg.Set("part_present_di", "7");
        cfg.Set("load_pos", "123");
        cfg.Set("use_vision", "true");

        std::ostringstream out;
        cfg.Save(out);
        DeviceConfig loaded;
        std::istringstream in(out.str());
        loaded.Load(in);

        Invariant(loaded.GetLong("part_present_di", 0) == 7, "long value round-trips");
        Invariant(loaded.GetLong("load_pos", 0) == 123, "long value round-trips");
        Invariant(loaded.GetBool("use_vision", false), "bool value round-trips");
        Invariant(loaded.GetString("load_pos") == "123", "string value round-trips");
        Invariant(loaded.GetLong("missing_key", 99) == 99, "missing key -> fallback");

        // '#' comments and blank lines are ignored on Load.
        std::istringstream file("# die bonder config\n\npart_present_di = 5\nload_pos = 999\n");
        DeviceConfig from_file;
        from_file.Load(file);
        Invariant(from_file.GetLong("part_present_di", 0) == 5, "comments/blank lines ignored");
        Invariant(from_file.GetLong("load_pos", 0) == 999, "'key = value' with spaces parsed");
    }
};

int main() {
    return RunAll(std::make_unique<PersistenceTest>());
}
