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

Right now resources and modules are names. Give them a reason to exist without
betting on an architecture.

- [ ] Canonical resource stubs: `Axis`, `Camera`, `DigitalIO` (simulated, no
      hardware) so a module can actually *use* a resource.
- [ ] Richer `Module` lifecycle: `Configure` / `Start` / `Stop` / `Reset`,
      added only as the example demands.
- [ ] Workflow control: failure of a step, conditional branching — only if a
      concrete recipe needs it.
- [ ] Tests: a tiny harness around the lifecycle trace.

## Phase 3 — Concurrency and the execution model

A real device runs modules concurrently and shares resources. This is where an
execution abstraction earns its place.

Phase 1 already has a single-threaded run loop with a stop-request signal
(ADR-0006). This phase is about genuine parallelism — several modules or
workflows running at once — which the single-cycle loop does not yet do.

- [ ] Decide between cooperative steps, a thread per module, or an actor model
      — **driven by a real bottleneck**, then documented as an ADR.
- [ ] Resource arbitration (who owns the shared axis right now?).
- [ ] Cancellation and `Stopping` semantics that actually abort in-flight work.

## Phase 4 — Toward concrete devices

With a proven execution model, specialize it.

- [ ] A first real device profile (e.g. a generic pick-and-place) as an
      instance of the model, not a fork of it.
- [ ] Persistence: recipes, configuration, persisted state.
- [ ] Interfacing: SECS/GEM, PLC, host integration — each added only when a
      target device requires it.

## Standing rule

Before adding any abstraction from Phase 2 onward, ask: *what recurring pain
does this remove?* If the answer is "it might be useful later," defer it.
