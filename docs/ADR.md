# Architecture Decision Records

Decisions are recorded so future phases know *why* the model looks the way it
does, not just what it is. ADRs are grouped into files by the phase/concern that
produced them.

## By phase

- [Phase 1 — the runtime model](adr/01-runtime-model.md) — 0001–0006
- [Phase 2 — lifecycle, faults & logging](adr/02-lifecycle-and-logging.md) — 0007–0010
- [Devices, hosting & concurrency](adr/03-devices-and-concurrency.md) — 0011–0013
- [Persistence](adr/04-persistence.md) — 0014–0016
- [Events & alarms](adr/05-events-and-alarms.md) — 0017–0018

## Index

| ADR | Title |
|-----|-------|
| 0001 | Build a runtime model, not a device simulator |
| 0002 | One folder per concept, no "core" umbrella |
| 0003 | Defer larger abstractions |
| 0004 | msys2 UCRT64 clang + clangd, Ninja generator |
| 0005 | State as a plain enum, no state-machine framework |
| 0006 | Run is a loop, not a one-shot |
| 0007 | Extend machine states for fault handling and recovery |
| 0008 | Phase 2: concrete resources and modules live in the library |
| 0009 | Stop request is a sticky flag, cleared only by Reset |
| 0010 | Log through a redirectable Logger, not std::cout |
| 0011 | A Device facade: named profiles over a Machine |
| 0012 | A Host manages multiple devices uniformly |
| 0013 | Concurrency via a task/delegate pool (proposed) |
| 0014 | Persist state/history via a History journal |
| 0015 | Persist recipes via named actions + a serializable spec |
| 0016 | Persist device configuration via DeviceConfig |
| 0017 | An EventBus spine for observability |
| 0018 | Alarms are the causes; Fault is just a state |
