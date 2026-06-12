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
