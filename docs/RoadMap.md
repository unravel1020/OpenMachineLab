# RoadMap

OpenMachineLab grows the runtime model one real concern at a time. Each phase
must be justified by a concrete need, not by anticipation.

## Phase 1 — Verify the model runs ✅

Done. `Machine`, `Resource`, `Module`, `Workflow`, `State` exist as minimal
abstractions and the `minimal_machine` example runs the full lifecycle.

- [x] Concept model (ADR-0001, ADR-0002)
- [x] Plain-enum lifecycle (ADR-0005)
- [x] CMake (Ninja) + clangd toolchain (ADR-0004)
- [x] Acceptance demo with a state trace
- [x] Run is a loop that exits on `Stop()` (ADR-0006)

## Phase 2 — Make the abstractions do something

Resources and modules were bare names; they now have minimal behavior and
modules use resources (`MotionModule`→`Axis`, `VisionModule`→`Camera`), promoted
into the library (ADR-0008). The items below extend this without betting on an
architecture.

- [x] Canonical resource stubs: `Axis`, `Camera`, `DigitalIO` (simulated) — done;
      each is driven by a module (`MotionModule` / `VisionModule` / `IoModule`).
- [x] Richer `Module` lifecycle: `Configure` / `Initialize` / `Start` / `Stop` /
      `Reset`, driven by the Machine (IoModule exercises all of them).
- [x] Workflow control: a failing step stops the workflow and faults the machine
      (`Fault`); `Reset()` recovers through `Recovering`. Conditional branching
      still deferred until a recipe needs it.
- [x] Tests: a shared `TestBase` (`tests/oml_test.h`) backs lifecycle, perf, and
      safety suites (CTest). The safety tests already caught a lost-stop race,
      fixed by the sticky stop flag (ADR-0009).

## Phase 3 — Concurrency and the execution model

A real device runs modules concurrently and shares resources. This is where an
execution abstraction earns its place.

The run loop is single-threaded, but its stop-request signal is already
concurrency-safe (a sticky flag — ADR-0006 / ADR-0009), and the safety tests
stress it from many threads. This phase is about genuine parallelism — several
modules or workflows running at once — which the single-cycle loop does not yet
do.

- [x] Logging is already redirectable and thread-safe (`Logger`, ADR-0010) — the
      run loop, recipe, and tests all route through it.
- [ ] Concurrency via a generic **task/delegate pool**: a device submits subtasks
      (e.g. "trigger vision" + "move the axis" in parallel during PR) and the pool
      executes them. Chosen so the parallelism *mechanism* stays decoupled from
      device logic, swappable, and unit-testable with a synchronous pool — and so
      each real flow is built on its real scenario, not a guessed scheduler.
- [ ] Resource arbitration (who owns the shared axis right now?).
- [ ] Cancellation and `Stopping` semantics that actually abort in-flight work.

## Phase 4 — Toward concrete devices

With a proven execution model, specialize it.

- [x] A `Device` facade (ADR-0011) + two concrete profiles (`DieBonder`,
      `PickAndPlace`) as instances of the model, not forks of it.
- [x] A `Host` manages multiple devices uniformly (register/lifecycle/status);
      ADR-0012. (Running several concurrently waits for the Phase 3 pool.)
- [ ] Persistence: recipes, configuration, persisted state.
- [ ] Interfacing: SECS/GEM, PLC, host integration — each added only when a
      target device requires it.

## Standing rule

Before adding any abstraction from Phase 2 onward, ask: *what recurring pain
does this remove?* If the answer is "it might be useful later," defer it.
