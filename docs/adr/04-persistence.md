# ADRs — Persistence

Persistable layers for state/history, recipes, and device configuration.
(See the [ADR index](../ADR.md).)

## ADR-0014 — Persist state/history via a History journal

**Status:** accepted
**Date:** 2026-06-13

**Context.** A device's lifecycle — the states it passed through, and especially
*why* it faulted — is valuable for audit, diagnostics, and resume. Phases 1-2
only emitted a text trace (Logger); there was no structured, saveable record.
This is the fault-history foundation hinted at in ADR-0007.

**Decision.** Add a `History` (`history/History.h`): an append-only journal of
`{seq, state, note}` entries. A `Machine` publishes every transition as a
`StateChanged` event; a `History` attached to the bus (`History::Attach`) journals
them, with the fault reason as the note. `History` serializes to / parses from a
stream (`Save`/`Load`), and a state name round-trips via `FromString`.

**Consequences.** State and fault history are now first-class and persistable
(save to a file, reload, audit). `Machine` no longer depends on `History` at all
— it just publishes events; a `History` subscribes (decoupled, opt-in). The
journal records transitions + fault reasons; per-cycle/per-step detail still
lives in the Logger trace (separation of concerns: structured history vs. text
trace).

## ADR-0015 — Persist recipes via named actions + a serializable spec

**Status:** accepted
**Date:** 2026-06-13

**Context.** A runtime `Workflow` holds `Step`s whose actions are
`std::function` callables — which cannot be serialized. So a recipe could not be
saved to or loaded from a file, even though "edit the recipe without
recompiling" is a basic industrial need.

**Decision.** Split the recipe into two layers:
- An `ActionRegistry` (`workflow/ActionRegistry.h`): maps names to callables.
- A `RecipeSpec` (`workflow/RecipeSpec.h`): a serializable description (named
  steps, each referencing an action *by name*). `SaveRecipe`/`LoadRecipe` handle
  the text format; `BuildWorkflow(spec, registry)` rebuilds a runtime `Workflow`
  by resolving each name (returns `nullptr` if any name is unknown — no partial
  build).

Devices that want file-driven recipes register their actions and author recipes
as specs; devices that hardcode recipes (`DieBonder`/`PickAndPlace` today) keep
using direct lambdas.

**Consequences.** Recipes are persistable and editable without recompile, and the
callable-vs-serializable tension is resolved cleanly (names on disk, lambdas in
the registry). Cost: a registry-based recipe must register its actions and
reference them by name — opted into per device. The format is a trivial text line
format (no JSON/TOML dependency); a richer format can replace it later behind the
same `SaveRecipe`/`LoadRecipe` interface.

## ADR-0016 — Persist device configuration via DeviceConfig

**Status:** accepted
**Date:** 2026-06-13

**Context.** A device's installation-specific values (I/O channel mappings, axis
positions, flags) were hardcoded as `constexpr` in the device headers. Changing a
deployment meant recompiling. Config is distinct from recipe (what to run) and
history (what happened): it is *how a device is wired*.

**Decision.** Add a `DeviceConfig` (`config/DeviceConfig.h`): a key/value store
with typed getters (string/long/bool, each with a fallback) and `Save`/`Load` to
a text format ("`key = value`" per line; `#` comments and blank lines allowed).
A device reads its values from a `DeviceConfig`, falling back to built-in
defaults. The `DieBonder` is retrofitted: its channels and axis positions come
from a config (defaults preserve the old behavior); omit the argument to use
defaults.

**Consequences.** Deployments can override a device's wiring without recompiling
(load a config file, construct the device with it). The format is trivial text
(no JSON/TOML dependency); a richer format can replace it behind the same
interface. Config is opt-in per device (`DieBonder` uses it; `PickAndPlace` keeps
hardcoded values for now) — consistent with how recipes opted in (ADR-0015).
