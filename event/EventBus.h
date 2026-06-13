#pragma once

#include "event/Event.h"

#include <functional>
#include <map>
#include <mutex>

namespace oml {

// A minimal publish/subscribe bus for Events. Subscribers register a handler and
// get a token; publishers call Publish. This is the "spine" of observability:
// Machine (and later AlarmManager) publish, and any subscriber (History, Logger,
// a UI, a host) reacts without the emitter knowing who they are.
//
// Thread-safe. Publish snapshots the handlers under the lock and invokes them
// outside it, so a handler may itself Publish/Subscribe/Unsubscribe without
// dead-locking.
class EventBus {
public:
    using Handler = std::function<void(const Event&)>;

    // Register a handler; returns a token for Unsubscribe (tokens are >= 1).
    int Subscribe(Handler handler);
    void Unsubscribe(int token);
    void Publish(const Event& event);

private:
    std::mutex          mutex_;
    int                 next_token_ = 1;
    std::map<int, Handler> handlers_;
};

} // namespace oml
