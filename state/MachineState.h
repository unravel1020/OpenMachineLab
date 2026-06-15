#pragma once

#include <string>
#include <string_view>

namespace oml {

// The lifecycle of a machine. Plain enum on purpose (ADR-0005): no state-machine
// framework, no transition table. Adding a state later does not require touching
// call sites.
// 机器生命周期。刻意用普通枚举（ADR-0005）：不引入状态机框架、不建转换表。
// 后续新增状态无需修改调用点。
enum class MachineState {
    Created,
    Initializing,
    Ready,
    Running,
    Stopping,
    Stopped,
    Fault,
    Paused,
    Recovering
};

// Human-readable name for a state, used by the runtime trace.
// 状态的可读名称，运行时 trace 使用。
std::string_view ToString(MachineState state);

// Inverse of ToString(): parse a state name. Unknown names map to Created.
// ToString 的逆操作：解析状态名。未知名称返回 Created。
MachineState FromString(std::string_view name);

} // namespace oml
