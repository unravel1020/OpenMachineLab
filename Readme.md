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
| 4 | a per-part recipe (`LoadFrame → Align → Bond → Unload`) |
| 5 | runs in a loop until the operator issues `Stop()`, then `Stopping → Stopped` |

The lifecycle is `Created → Initializing → Ready → Running → Stopping → Stopped`.
`Running` is not a single pass: the machine loops, running the recipe once per
cycle, until `Stop()` is requested — that is what makes it a machine rather than
a script. `Stopping` is the transient "shutting down" state (parking resources,
closing modules) before the machine is fully `Stopped`; in Phase 1 it is a
placeholder, with real stop work deferred to a later phase.

Expected output (the loop runs until you press Enter):

```
Created
 v
Initializing
    Resource AxisResource
    Resource CameraResource
    Module ModuleA Initialized
    Module ModuleB Initialized
 v
Ready
 v
Running
    cycle 1
    Workflow Recipe Started
        Step LoadFrame
        Step Align
        Step Bond
        Step Unload
    cycle 2
        ...                          <- keeps cycling, once per cycle
    stop requested after N cycle(s)  <- Stop() was called
 v
Stopping
 v
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

Running `minimal_machine` blocks in its run loop — **press Enter to stop it**
and watch it go `Stopping → Stopped`.

## Editor setup

`.vscode/settings.json` points clangd at `C:/msys64/ucrt64/bin/clangd.exe` and
at `build/compile_commands.json`. After the first configure, open any source
file and clangd provides navigation, diagnostics, and completions.

> If your global `clangd.path` still points at `D:/SoftWareStall/msys64/...`,
> that path does not exist on this machine — use `C:/msys64/ucrt64/bin/clangd.exe`.

See [docs/ADR.md](docs/ADR.md) for the design decisions and
[docs/RoadMap.md](docs/RoadMap.md) for what comes after Phase 1.

---

# OpenMachineLab（中文版）

工业设备软件的通用**运行模型**。

> 第一阶段**不是**模拟焊线机、点胶机或固晶机。它是能*承载*这些设备的最小软件模型——
> 在运动控制、视觉、PLC、SECS/GEM 出现之前，一个工业设备程序所需的抽象。

## 概念模型

抽丝剥茧到本质，一个设备程序只需要四个概念加一个生命周期：

```
Machine
 ├── Resource   - 设备拥有的能力（轴、相机、IO、光源）
 ├── Module     - 功能单元（上下料、视觉、键合头）
 ├── Workflow   - 设备执行的有序步骤序列
 └── State      - 设备处于生命周期的哪个阶段
```

其它一切（事件总线、调度器、插件系统、IoC、DDD 分层）都被刻意省略。
它们应从后续阶段的真实痛点中长出来，而不是凭空臆测。

## 状态 —— 第一阶段（MVP）

第一阶段的目标只是**验证模型能跑起来**。它回答：*OpenMachineLab 到底在模拟什么？*
——工业设备的运行模型，而不是任何单一设备。

验收示例（`examples/minimal_machine`）：

| 用例 | 展示内容 |
|------|----------|
| 1 | `Machine machine;` 创建一个虚拟设备 |
| 2 | 两个资源（`AxisResource`、`CameraResource`） |
| 3 | 两个模块（`ModuleA`、`ModuleB`） |
| 4 | 单工件配方（`LoadFrame → Align → Bond → Unload`） |
| 5 | 循环运行，直到操作员下达 `Stop()`，然后 `Stopping → Stopped` |

生命周期为 `Created → Initializing → Ready → Running → Stopping → Stopped`。
`Running` 不是跑一遍：设备会循环，每个 cycle 执行一次配方，直到收到 `Stop()` 请求——
这才是"机器"而非"脚本"。`Stopping` 是"正在关机"的瞬态（回零资源、关闭模块），之后才完全
`Stopped`；第一阶段里它只是占位，真正的停止逻辑推迟到后续阶段。

预期输出（循环会一直运行，直到你按回车）：

```
Created
 v
Initializing
    Resource AxisResource
    Resource CameraResource
    Module ModuleA Initialized
    Module ModuleB Initialized
 v
Ready
 v
Running
    cycle 1
    Workflow Recipe Started
        Step LoadFrame
        Step Align
        Step Bond
        Step Unload
    cycle 2
        ...                          <- 每个 cycle 循环一次
    stop requested after N cycle(s)  <- 收到 Stop()
 v
Stopping
 v
Stopped
```

## 目录结构

```
OpenMachineLab
├── machine/      Machine - 运行模型根
├── module/       Module  - 功能单元接口
├── resource/     Resource - 能力接口
├── workflow/     Workflow + Step - 有序工作
├── state/        MachineState - 生命周期
├── examples/
│   └── minimal_machine/   第一阶段验收示例
└── docs/         ADR.md, RoadMap.md
```

## 工具链

第一阶段基于 **msys2 UCRT64** clang 工具链（clang + clangd）：

- `C:/msys64/ucrt64/bin/clang.exe`、`clang++.exe`、`clangd.exe`
- CMake + Ninja（用 `pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja` 安装）

## 构建与运行

**方式 A —— 在 MSYS2 "UCRT64" 终端里**（clang 是默认编译器）：

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/examples/minimal_machine/minimal_machine.exe
```

**方式 B —— 在 Git Bash / 任意终端里**，手动指向 ucrt64 工具：

```sh
export PATH="/c/msys64/ucrt64/bin:$PATH"
CC=clang CXX=clang++ cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/examples/minimal_machine/minimal_machine.exe
```

配置过程还会生成 `build/compile_commands.json`，clangd 据此获得每个文件的精确编译参数。

运行 `minimal_machine` 会阻塞在运行循环里——**按回车停止**，观察它进入 `Stopping → Stopped`。

设计决策见 [docs/ADR.md](docs/ADR.md)，第一阶段之后的规划见 [docs/RoadMap.md](docs/RoadMap.md)。
