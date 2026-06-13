# ADRs — Phase 2: lifecycle, faults & logging

Making the model a real machine: extended states for fault handling, concrete
reusable parts in the library, a sticky stop flag, and a redirectable logger.
(See the [ADR index](../ADR.md).)

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
the Phase 2 step from [RoadMap](../RoadMap.md): "give resources a reason to exist;
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
cout's `rdbuf`). A real device needs to route logs to a file, a host, or nothing;
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
