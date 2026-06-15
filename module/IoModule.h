#pragma once

#include "module/Module.h"
#include "resource/DigitalIO.h"

#include <string>

namespace oml {

// A module that owns the device's digital I/O block. Exercises the full Phase 2
// module lifecycle against a DigitalIO resource: Configure/Initialize set a safe
// state and sample the e-stop, Start/Stop drive the ready light, Reset re-samples
// the e-stop after a fault.
// 拥有设备数字 I/O 模块的模块。在 DigitalIO 资源上执行完整的第二阶段模块生命周期：
// Configure/Initialize 设置安全状态并采样急停，Start/Stop 驱动就绪灯，
// Reset 在故障后重新采样急停。
class IoModule : public Module {
public:
    explicit IoModule(DigitalIO& io) : io_(io) {}

    std::string Name() const override { return "IoModule"; }

    void Configure() override {} // no parameters in the stub 桩中无参数

    void Initialize() override { SafeState(); }

    void Start() override { io_.Write(kReadyLight, true); }  // running -> light on  运行中→灯亮
    void Stop() override  { io_.Write(kReadyLight, false); } // stopped -> light off 停止→灯灭

    void Reset() override { SafeState(); }

    bool EstopPressed() const { return estop_pressed_; }

private:
    // Outputs off, then sample the e-stop input. The known-safe baseline.
    // 输出全关，然后采样急停输入。已知的安全基线。
    void SafeState() {
        io_.Write(kReadyLight, false);
        estop_pressed_ = io_.Read(kEStop);
    }

    // Well-known lines on this device's I/O block. DI and DO are separate line
    // spaces, so a DI channel and a DO channel may share a number.
    // 本设备 I/O 模块上的已知线路。DI 和 DO 是独立的线路空间，
    // DI 通道和 DO 通道可以共用一个编号。
    static constexpr int kEStop      = 0; // DI: emergency stop 急停
    static constexpr int kReadyLight = 0; // DO: "ready" indicator 就绪指示灯

    DigitalIO& io_;
    bool       estop_pressed_ = false;
};

} // namespace oml
