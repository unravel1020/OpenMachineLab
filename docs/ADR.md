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

