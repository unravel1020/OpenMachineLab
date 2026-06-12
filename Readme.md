# OpenMachineLab

A generic **runtime model** for industrial equipment software.

> Phase 1 is **not** a simulator of a wire bonder, dispenser, or die bonder.
> It is the smallest software model that can *host* any of them — the
> abstraction an industrial device program needs before motion, vision, PLC, or
> SECS/GEM are even relevant.

## The concept model

Stripped to its essence, a device program needs exactly four concepts plus a
lifecycle:

```
Machine
 ├── Resource   - a capability the machine owns (axis, camera, IO, light)
 ├── Module     - a functional unit (loader, vision, bond head)
 ├── Workflow   - an ordered sequence of steps the machine executes
 └── State      - where the machine is in its lifecycle
```

Everything else (event bus, scheduler, plugin system, IoC, DDD layering) is
deliberately absent. Those grow out of real pain in later phases, not out of
speculation.

## Status — Phase 1 (MVP)

The goal of Phase 1 is to **verify the model runs**, nothing more. It answers:
*OpenMachineLab is simulating what?* — the runtime model of an industrial
device, not any single device.

Acceptance demo (`examples/minimal_machine`):

| Case | What it shows |
|------|----------------|
| 1 | `Machine machine;` creates a virtual device |
| 2 | two resources (`AxisResource`, `CameraResource`) |
| 3 | two modules (`ModuleA`, `ModuleB`) |
| 4 | one workflow (`Initialize → Run → Shutdown`) |
| 5 | full lifecycle: `Created → Initializing → Ready → Running → Stopping → Stopped` |

The lifecycle a Case 5 run walks is `Created → Initializing → Ready → Running →
Stopping → Stopped`. `Stopping` is the transient "shutting down" state — parking
resources and closing modules — before the machine is fully `Stopped`. In Phase 1
it is a placeholder; real stop work lands there in a later phase.

Expected output:

```
Created
 ↓
Initializing
    Resource AxisResource
    Resource CameraResource
    Module ModuleA Initialized
    Module ModuleB Initialized
 ↓
Ready
 ↓
Running
    Workflow MainWorkflow Started
        Step Initialize
        Step Run
        Step Shutdown
 ↓
Stopping
 ↓
Stopped
```

## Layout

```
OpenMachineLab
├── machine/      Machine - the runtime root
├── module/       Module  - functional unit interface
├── resource/     Resource - capability interface
├── workflow/     Workflow + Step - ordered work
├── state/        MachineState - lifecycle
├── examples/
│   └── minimal_machine/   the Phase 1 acceptance demo
└── docs/         ADR.md, RoadMap.md
```

## Toolchain

Phase 1 targets the **msys2 UCRT64** clang toolchain (clang + clangd):

- `C:/msys64/ucrt64/bin/clang.exe`, `clang++.exe`, `clangd.exe`
- CMake + Ninja (installed via `pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja`)

## Build & run

**Option A — from an MSYS2 "UCRT64" shell** (clang is the default compiler):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/examples/minimal_machine/minimal_machine.exe
```

**Option B — from Git Bash / any shell**, pointing CMake at the ucrt64 tools:

```sh
export PATH="/c/msys64/ucrt64/bin:$PATH"
CC=clang CXX=clang++ cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/examples/minimal_machine/minimal_machine.exe
```

Configuring also writes `build/compile_commands.json`, which clangd reads for
exact per-file flags.

## Editor setup

`.vscode/settings.json` points clangd at `C:/msys64/ucrt64/bin/clangd.exe` and
at `build/compile_commands.json`. After the first configure, open any source
file and clangd provides navigation, diagnostics, and completions.

> If your global `clangd.path` still points at `D:/SoftWareStall/msys64/...`,
> that path does not exist on this machine — use `C:/msys64/ucrt64/bin/clangd.exe`.

See [docs/ADR.md](docs/ADR.md) for the design decisions and
[docs/RoadMap.md](docs/RoadMap.md) for what comes after Phase 1.
