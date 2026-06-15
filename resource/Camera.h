#pragma once

#include "resource/Resource.h"

#include <string>

namespace oml {

// A simulated camera (Phase 2 stub). Tracks whether it has been opened and how
// many frames it has captured, so a module/recipe can exercise a real interface.
// 模拟相机（第二阶段桩）。记录是否已打开、已触发多少帧，
// 让模块/配方可以调用真实的资源接口。
class Camera : public Resource {
public:
    std::string Name() const override { return "Camera"; }

    void Open() { open_ = true; }
    void Close() { open_ = false; }
    bool IsOpen() const { return open_; }

    // Simulate grabbing a frame (e.g. for alignment). Counts captures so a
    // recipe/observer can see the camera being used during production.
    // 模拟抓取一帧（如用于对位）。计数触发次数，配方/观察者可看到
    // 生产过程中相机被使用的次数。
    void Trigger() { ++captures_; }
    long Captures() const { return captures_; }

private:
    bool open_      = false;
    long captures_  = 0;
};

} // namespace oml
