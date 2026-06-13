# ADRs — Events & alarms

An EventBus spine for observability, and an Alarm system where alarms are the
*causes* of conditions (distinct from the Fault state). (See the
[ADR index](../ADR.md).)

## ADR-0017 — An EventBus spine for observability

**Status:** accepted
**Date:** 2026-06-13

**Context.** Observability was split across Logger (text trace) and History
(structured journal), each wired directly. There was no single spine a host, UI,
or future alarm system could tap; every new observer meant touching the emitters.
Phase 1 explicitly deferred an event bus; the Alarm work (next) makes the need
concrete — this is the justified moment (ADR-0003).

**Decision.** Add a minimal `EventBus` (`event/EventBus.h`): `Subscribe(handler)`
returns a token, `Unsubscribe`, `Publish`. Events are a `std::variant`
(`StateChanged` first; alarm events added with the Alarm system). Each `Machine`
owns a bus (`Machine::Bus()`) and publishes `StateChanged` on every transition.
Subscribers (History, Logger, a UI, a host) react without the emitter knowing
them.

`Publish` snapshots handlers under the lock and invokes them outside it, so a
handler may re-enter the bus without dead-locking; the bus is thread-safe for the
concurrency work.

**Consequences.** One decoupled spine for all observers; adding an observer no
longer touches emitters. The cost is an indirection per event (negligible).
History still journals directly for now; it can become a subscriber later (single
source of truth) without changing emitters.

## ADR-0018 — Alarms are the causes; Fault is just a state

**Status:** accepted
**Date:** 2026-06-13

**Context.** ADR-0007 added a `Fault` state but noted "error details belong to a
future alarm or diagnostic system." Without alarms the machine could enter Fault
but recorded only a free-text note for why. Real equipment needs structured
causes — "vacuum loss", "axis over-travel", "e-stop" — which may be active
together and must clear individually or all at once on recovery. Crucially, an
alarm is the CAUSE; `Fault` is the resulting STATE — they are not the same thing.

**Decision.** Add an `AlarmManager` (`alarm/AlarmManager.h`) holding active
`Alarm`s (each: code, `Severity`, name, message), publishing `AlarmRaised`/
`AlarmCleared` on the EventBus. Severity `Warning` and below only notify; `Fault`
and `Critical` drive the machine into the `Fault` state (Machine asks
`AlarmManager::HasFault()`). On a failing step the Machine raises a `PROCESS_FAIL`
alarm (the cause) and then enters `Fault` (the state); `Reset` clears all alarms
before recovering.

**Consequences.** The cause and the state are now separate, first-class concepts:
operators see WHY (active alarms) alongside WHERE the machine is (`Fault` state);
recovery clears causes. Multiple alarms can be active at once. The `AlarmManager`
extends the `Event` variant (`AlarmRaised`/`AlarmCleared`), so any subscriber
(History, UI, host) sees alarm lifecycle without touching emitters. Only one
alarm source (recipe step failure) exists today; hardware-sourced alarms
(e-stop, over-travel) plug into the same `Raise` path when needed.
