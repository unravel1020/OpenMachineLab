# Architecture Decision Records

Decisions are recorded so future phases know *why* the model looks the way it
does, not just what it is.

## ADR-0001 — Build a runtime model, not a device simulator

**Status:** accepted
**Date:** 2026-06-12

**Context.** The first question for OpenMachineLab is what it is simulating.
Motion, vision, PLC, SECS/GEM, and any specific device are all downstream of a
more fundamental question: *what minimal concepts does an industrial device
program need to run?*

**Decision.** Phase 1 models the **runtime** — the abstract skeleton a device
program runs on — not any concrete device. A wire bonder and a dispenser should
both be expressible as instances of the same model.

**Consequences.** Resource/Module/Workflow stay abstract and minimal. Nothing
in the core knows about axes, cameras, or bonding. Concrete device types are
application code, which the example demonstrates.

## ADR-0002 — One folder per concept, no "core" umbrella

**Status:** accepted

**Context.** A common reflex is to add a `core/` package early.

**Decision.** Keep one folder per concept (`machine/`, `module/`, `resource/`,
`workflow/`, `state/`). There is no `core/`. The model is small enough that a
single static library (`OpenMachineLab`) holds it.

**Consequences.** Includes are uniform: `#include "machine/Machine.h"`. The
project root is the sole include directory. A `core/` layer can be introduced
later only if cross-cutting concerns demand it.

## ADR-0003 — Defer larger abstractions

**Status:** accepted

**Context.** Event bus, scheduler, state-machine framework, plugin system, IoC
container, DDD layering — all are plausible in an equipment framework.

**Decision.** None of them exist in Phase 1. They will be added only when a
concrete, recurring pain justifies them.

**Consequences.** `Module` exposes only `Name()` and an `Initialize()` hook.
`MachineState` is a plain enum switched on, not a framework. `Workflow` is an
ordered vector of callables. This keeps the surface area honest and avoids
locking the project into an architecture before it knows its requirements.

## ADR-0004 — msys2 UCRT64 clang + clangd, Ninja generator

**Status:** accepted

**Context.** The development toolchain must be consistent and clangd-driven.

**Decision.** Use the **msys2 UCRT64** clang toolchain. Generate build files
with the **Ninja** generator and export `compile_commands.json`
(`CMAKE_EXPORT_COMPILE_COMMANDS=ON`) so clangd gets exact per-file flags.

**Consequences.** `.vscode/settings.json` sets `clangd.path` to
`C:/msys64/ucrt64/bin/clangd.exe` and adds `--compile-commands-dir=build`. The
`C_Cpp` IntelliSense engine is disabled to avoid conflicting with clangd.

## ADR-0005 — State as a plain enum, no state-machine framework

**Status:** accepted

**Context.** A device lifecycle (`Created → Initializing → Ready → Running →
Stopping → Stopped`) looks like a textbook state machine. The two shutdown
states are distinct on purpose:

- **`Stopping`** — *transient*. The machine is still alive: it has been told to
  shut down and is actively parking resources, closing modules, and aborting
  in-flight work. Not yet safe to treat as halted.
- **`Stopped`** — *terminal*. Shutdown is complete; the machine is fully halted
  and its resources are released. Reaching `Running` again requires going back
  through `Initialize`.

In Phase 1 `Stopping` carries no work yet (it is a placeholder); real stop logic
lands there in a later phase.

**Decision.** Represent it as a plain `enum class MachineState` with a
`ToString` helper and simple guarded assignments in `Machine`. No transition
tables, no framework.

**Consequences.** Transitions are trivial to read and step through. If guards
or history become necessary, a real state machine can replace this later — but
only after the lifecycle is actually observed to need it.

## ADR-0006 — Run is a loop, not a one-shot

**Status:** accepted
**Date:** 2026-06-12

**Context.** The first cut of `Machine::Run()` executed the workflows once and
returned. That describes a script, not a machine: real equipment keeps producing
part after part until an operator, host, or error tells it to stop. A one-shot
`Run()` fails the basic test of "is this a machine?"

**Decision.** `Run()` is a loop. Each iteration is one production cycle: the
recipe runs once, the machine checks a stop-request flag, and either ticks again
or exits. The stop request is a single `std::atomic<bool>` set by `Stop()`, which
may be called from another thread (the operator console, a host interface, ...).

**Consequences.** The model now needs the *minimal* concurrency primitive
required for a loop that exits on command — one atomic flag. This deliberately
does not pull in Phase 3's concerns: there is still no scheduler, no parallel
modules, no resource arbitration. The cycle body runs single-threaded; only the
stop *signal* crosses a thread boundary. See [RoadMap](RoadMap.md) Phase 3 for
when genuine concurrency is justified.

## ADR-0007 — Extend machine states for fault handling and recovery

**Status:** accepted
**Date:** 2026-06-12

**Context.** The initial lifecycle model defined the machine states as:

`Created → Initializing → Ready → Running → Stopping → Stopped`

This lifecycle is sufficient to describe startup, production, and shutdown, but it does not represent how real industrial equipment behaves when abnormal conditions occur.

In industrial software, faults are expected conditions that must be handled. A machine should not always terminate after an abnormal event. Some conditions require operator intervention, some require a recovery procedure, and some may allow production to continue after handling.

The previous model had no explicit representation for these intermediate conditions. Adding an `Error` state would mix two different concepts:

* **Error** — an event or reason why something went wrong.
* **State** — the current condition of the machine lifecycle.

Therefore, faults should be represented as machine states, while error details belong to a future alarm or diagnostic system.

**Decision.** Extend `MachineState` with three additional states:

* **`Paused`** — The machine temporarily stops production execution while remaining initialized. Resources are still available, and the machine can continue without a full restart after the condition is cleared.

* **`Fault`** — The machine has entered an abnormal condition that prevents normal production. The machine remains active enough for diagnosis and recovery actions.

* **`Recovering`** — The machine is executing recovery procedures after a fault or pause condition. Successful recovery returns the machine to `Ready` or `Running`. Failed recovery returns the machine to `Fault`.

The lifecycle becomes:

```
Created
    |
Initializing
    |
Ready
    |
Running
    |
Paused
    |
Fault
    |
Recovering
    |
Ready / Running
    |
Stopping
    |
Stopped
```

`MachineState` remains a plain `enum class`. No state-machine framework, transition table, or external lifecycle engine is introduced at this stage.

**Consequences.**

The runtime can now represent a more realistic industrial equipment lifecycle and provides a foundation for future features:

* Alarm and diagnostic management
* Recovery workflows
* Operator intervention
* Fault history tracking

This decision intentionally does not introduce an `AlarmManager`, event bus, scheduler, or recovery framework yet. These will only be added when concrete requirements appear.

The core model remains minimal: states describe the machine condition, while fault causes and handling logic stay in higher-level application code.

## ADR-0008 — Phase 2: concrete resources and modules live in the library

**Status:** accepted
**Date:** 2026-06-13

**Context.** Phase 1 kept the library abstract: `Resource` and `Module` were
pure interfaces, and the only concrete classes were defined inline in the
example's `main.cpp`. As more devices are modeled, each would re-derive the same
trivial `Axis`/`Camera` and motion/vision modules — pure duplication. This is
the Phase 2 step from [RoadMap](RoadMap.md): "give resources a reason to exist;
a module can actually use a resource."

**Decision.** Promote the concrete, reusable parts into the library:
`resource/Axis` and `resource/Camera` (simulated stubs with a minimal exercisable
interface), and `module/MotionModule` / `module/VisionModule` (modules that own a
resource and drive it on `Initialize`). Device-specific concerns — the recipe and
the operator console — stay in the example. `main.cpp` becomes a pure entry point
that assembles library parts and drives the lifecycle.

**Consequences.** The library now ships standard building blocks a device
composes, not only abstract interfaces. The new classes are header-only, so the
CMake source list is unchanged. Device-specific concrete types (e.g. a `BondHead`)
still belong to application code — only broadly reusable parts are promoted into
the library.

## ADR-0009 — Stop request is a sticky flag, cleared only by Reset

**Status:** accepted
**Date:** 2026-06-13

**Context.** The run loop exits when `Stop()` sets `stop_requested_`. The first
implementation cleared the flag at the start of `Run()` "for a fresh run." That
creates a race: if `Stop()` is signalled just before `Run()` reaches the clear,
the clear clobbers the request and the loop never stops. The Phase 2 safety
tests (which fire `Stop()` concurrently with `Run()`) hung on exactly this.

**Decision.** `stop_requested_` is sticky: `Stop()` sets it and only `Reset()`
clears it. A stop signalled before or during `Run()` is therefore never lost.

**Consequences.** The stop signal is safe under concurrency — a prerequisite for
the Phase 3 execution model. The cost is a minor rule: a machine that has been
stopped cannot `Run()` again until it is `Reset()`. That is acceptable; a real
device does not resume production after a stop without an explicit recovery.

## ADR-0010 — Log through a redirectable Logger, not std::cout

**Status:** accepted
**Date:** 2026-06-13

**Context.** The model printed its trace directly to `std::cout`. That hard-wired
the sink and forced tests to suppress it with a `SilentCout` hack (swapping
cout's `rdbuf`). A real device needs to route logs to a file, a host, or nowhere;
and concurrency (Phase 3) needs serialized, thread-safe output.

**Decision.** Introduce a minimal `Logger` (`log/Logger.h`) with `Info`/`Warn`/
`Error` levels and a settable sink (`nullptr` discards). All model output —
Machine state transitions and lifecycle, Workflow step progress, recipe actions —
goes through the process-wide `oml::Log()` instead of `std::cout`. The sink is
atomic (lock-free fast path when silenced) and writes are mutex-guarded, so
concurrent loggers produce clean, interleaving-free output.

**Consequences.** Tests silence the trace via `SilentLog` (`Log().SetSink(nullptr)`)
instead of the cout-rdbuf hack, and perf numbers are no longer distorted by I/O.
The cost is one global instance and string-building at call sites — acceptable
for a cross-cutting concern. Only the three needed levels exist; structured
logging, filtering, and per-module sinks wait for a concrete need.

## ADR-0011 — A Device facade: named profiles over a Machine

**Status:** accepted
**Date:** 2026-06-13

**Context.** Phases 1-2 left device composition in each example's `main.cpp`.
That proved the model runs, but as more devices appear the composition is
duplicated per device and there is no uniform handle a host or tool can manage.
Two side-by-side examples would show generality but would not help future
expansion.

**Decision.** Introduce a thin `Device` facade (`device/Device.h`): a named
object that owns a `Machine`, exposes the uniform lifecycle (`Initialize`/`Run`/
`Stop`/`Reset`/`Shutdown` + `State`), and offers protected `Add*` composition
helpers. A concrete device is a `Device` subclass that populates its resources,
modules, and workflow in its own constructor. Two profiles now exist —
`DieBonder` and `PickAndPlace` — with different resource sets and recipes on the
same model.

**Consequences.** Every device has the same interface, so hosting, tooling, and
tests can treat them uniformly; adding a device is a new profile, not a fork.
The facade is intentionally thin — no plugin system, registry, config loader, or
IoC (those wait for a concrete need, per ADR-0003). `Device` holds a `Machine`,
so it is non-copyable/non-movable; it is created in place, like the devices it
models.

## ADR-0012 — A Host manages multiple devices uniformly

**Status:** accepted
**Date:** 2026-06-13

**Context.** With the Device facade (ADR-0011) a host process can create devices,
but each is managed ad hoc — there is no uniform way to register, control, or
query several at once. A real line has multiple devices; a supervisor needs one
handle over all of them.

**Decision.** Introduce a `Host` (`host/Host.h`) that owns a set of Devices,
registers them by name, and drives the non-blocking lifecycle uniformly
(`InitializeAll`/`StopAll`/`ResetAll`/`ShutdownAll`) plus a `Status()` snapshot.
Per-device access is via `Find(name)`.

**Consequences.** One handle manages any number of devices the same way — the
"hosting" layer over `Device`. `Host` deliberately does NOT centralize `Run()`
(it blocks, and running several devices concurrently is the future task/delegate
pool's job, RoadMap Phase 3). For now a single-threaded host runs one device at a
time via `Find(name)->Run()`; concurrent running waits for the pool.

## ADR-0013 — Concurrency via a task/delegate pool (direction; not yet built)

**Status:** proposed — direction recorded; implementation deferred until a
concrete scenario (e.g. PR) demands it.
**Date:** 2026-06-13

**Context.** Real devices need concurrent work:
- During PR (pattern-recognition alignment), vision and the motion axis must be
  triggered at the same time.
- A multi-station line runs stations in parallel.
- A `Host` may run several devices at once.

How do we add parallelism without baking threads into the core (`Machine`/
`Module`) — which would couple device logic to a threading model, make it hard to
test, and lock in a scheduler before we know what we need?

**Alternatives considered.**
- *Thread-per-module*: each `Module` owns a thread. Couples modules to threading;
  hard to test deterministically; no control over ordering or shared resources.
- *Actor model*: modules as message-passing actors. Heavier; overkill until
  messaging needs appear.
- *Coroutines/async runtime*: steep change; pulls in a runtime.

All of these bake a concurrency *model* into the core.

**Decision.** Add a generic **task/delegate pool** (an `Executor`): a device or
recipe *submits* independent subtasks to it, and the pool runs them — in parallel
where it can. The core stays single-threaded and free of threading knowledge;
parallelism is an explicit, opt-in delegation at the points that genuinely need
it (e.g. a PR step submits "trigger vision" + "move axis" and waits for both).

Sketch (not final):
```
class Executor {
public:
    using Task = std::function<void()>;
    Ticket Submit(Task task);   // independent unit of work; returns a handle
    void   WaitAll();           // block until all submitted tasks are done
};
```
- The real implementation is a fixed worker-thread pool.
- A **synchronous `Executor`** (runs each task inline on `Submit`) is used in
  tests, so parallel code is deterministic and race-free.

Why this shape:
- **Decoupling** — the pool owns the threading mechanism; device logic only
  declares "these subtasks are independent." Swapping the scheduler (thread pool,
  work-stealing, coroutines) changes the pool, not the devices.
- **Testability** — the synchronous pool makes parallel code unit-testable with
  no races.
- **Scenario-driven** — each parallel flow is built on its real need, not a
  guessed framework; engineers compose concurrency per device.
- **Composable** — a recipe step can submit subtasks; the `Host` can run devices
  via the pool; `Module` hooks stay single-threaded unless they delegate.

**Open questions (resolve when we build it).**
- *Resource arbitration* — if two subtasks touch the same `Axis`, who serializes?
  (Likely the resource owns a strand/lock; the pool is unaware.)
- *Cancellation* — how `Stop()`/`Fault` propagates into in-flight subtasks.
- *Failure* — a failing subtask must be able to fault the machine (extend the
  `Workflow::Result` across parallel tasks).
- *Pool sizing and affinity*.

**Consequences.** The core stays minimal and single-threaded; concurrency is an
explicit, isolatable, testable layer. The cost is that devices needing
parallelism must express it via submission (slightly more code than implicit
threading) — but that explicitness is the point. This ADR records the direction;
it becomes "accepted" when the first real scenario implements it.

## ADR-0014 — Persist state/history via a History journal

**Status:** accepted
**Date:** 2026-06-13

**Context.** A device's lifecycle — the states it passed through, and especially
*why* it faulted — is valuable for audit, diagnostics, and resume. Phases 1-2
only emitted a text trace (Logger); there was no structured, saveable record.
This is the fault-history foundation hinted at in ADR-0007.

**Decision.** Add a `History` (`history/History.h`): an append-only journal of
`{seq, state, note}` entries. A `Machine` optionally journals every transition
(attach via `SetHistory`); a fault is recorded with the failing workflow/step as
its note. `History` serializes to / parses from a stream (`Save`/`Load`), and a
state name round-trips via `FromString`.

**Consequences.** State and fault history are now first-class and persistable
(save to a file, reload, audit). The `Machine`'s dependency is one optional
pointer (null = no journal, no overhead) — minimal coupling. The journal records
transitions + fault reasons; per-cycle/per-step detail still lives in the Logger
trace (separation of concerns: structured history vs. text trace).

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

