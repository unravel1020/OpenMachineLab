# ADRs — Devices, hosting & concurrency

A Device facade over a Machine, a Host managing many devices, and the recorded
direction for concurrency. (See the [ADR index](../ADR.md).)

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

## ADR-0019 — A DeviceFactory creates devices by type name

**Status:** accepted
**Date:** 2026-06-15

**Context.** Devices were constructed directly (`make_unique<DieBonder>(cfg)`) at
each call site, so a host that loads a device list (or a line config) cannot name
a type without knowing the concrete class. Creation and management were tangled.

**Decision.** Add a `DeviceFactory` (`device/DeviceFactory.h`): a registry mapping
a type name to a `Creator` (`DeviceConfig` -> `unique_ptr<Device>`). `Register`
types; `Create(type, cfg)` builds one (or `nullptr` if unknown). It is standalone
— the `Host` does not own it: the application registers its concrete device types,
creates devices via the factory, and hands them to the `Host` to manage. Creation
and management stay decoupled.

**Consequences.** A host or config can instantiate devices by type name + config
without knowing concrete classes — the seam for a file-driven device line. The
factory is a thin registry (no plugin/dependency-injection framework); concrete
types live in application code and register themselves once at startup. Cost: one
registration per type per process.
