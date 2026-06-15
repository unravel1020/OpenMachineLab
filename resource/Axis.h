#pragma once

#include "resource/Resource.h"

#include <string>

namespace oml {

// A simulated linear axis (Phase 2 stub). No hardware — it just tracks a
// logical position so the runtime model has a resource with a real, exercisable
// interface rather than a bare name.
// 模拟线性轴（第二阶段桩）。没有真实硬件——只记录一个逻辑位置，
// 让运行模型有一个可真正调用的资源接口，而不是一个空名字。
class Axis : public Resource {
public:
    std::string Name() const override { return "Axis"; }

    // Park the axis at its home position.
    // 将轴回零到原点位置。
    void Home() { position_ = 0; }
    // Command an absolute move, in counts.
    // 命令绝对移动，单位为脉冲计数。
    void MoveTo(long position) { position_ = position; }
    long Position() const { return position_; }

private:
    long position_ = 0;
};

} // namespace oml
