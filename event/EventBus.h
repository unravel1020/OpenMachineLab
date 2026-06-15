#pragma once

#include "event/Event.h"

#include <functional>
#include <map>
#include <mutex>

namespace oml {

// A minimal publish/subscribe bus for Events. Subscribers register a handler and
// get a token; publishers call Publish. This is the "spine" of observability:
// Machine (and AlarmManager) publish, and any subscriber (History, Logger, a UI,
// a host) reacts without the emitter knowing who they are (ADR-0017).
// 最小化的发布/订阅事件总线。订阅者注册 handler 获取令牌；发布者调用 Publish。
// 这是可观测性的"脊梁"：Machine（和 AlarmManager）发布，任意订阅者
// （History、Logger、UI、host）响应，emitter 不感知订阅者（ADR-0017）。
//
// Thread-safe. Publish snapshots the handlers under the lock and invokes them
// outside it, so a handler may itself Publish/Subscribe/Unsubscribe without
// dead-locking.
// 线程安全。Publish 在锁内快照 handler，在锁外调用，使 handler 可重入总线
// 而不死锁。
class EventBus {
public:
    using Handler = std::function<void(const Event&)>;

    // Register a handler; returns a token for Unsubscribe (tokens are >= 1).
    // 注册 handler；返回令牌用于 Unsubscribe（令牌 >= 1）。
    int Subscribe(Handler handler);
    void Unsubscribe(int token);
    void Publish(const Event& event);

private:
    std::mutex          mutex_;
    int                 next_token_ = 1;
    std::map<int, Handler> handlers_;
};

} // namespace oml
