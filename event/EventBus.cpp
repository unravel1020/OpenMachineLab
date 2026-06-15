#include "event/EventBus.h"

#include <utility>
#include <vector>

namespace oml {

int EventBus::Subscribe(Handler handler) {
    const std::lock_guard<std::mutex> lock(mutex_);
    const int token = next_token_++;
    handlers_.emplace(token, std::move(handler));
    return token;
}

void EventBus::Unsubscribe(int token) {
    const std::lock_guard<std::mutex> lock(mutex_);
    handlers_.erase(token);
}

void EventBus::Publish(const Event& event) {
    // Snapshot under the lock, deliver outside it so handlers can re-enter the
    // bus (Publish/Subscribe/Unsubscribe) without dead-locking.
    // 在锁内快照，在锁外投递，使 handler 可重入总线（Publish/Subscribe/Unsubscribe）
    // 而不会死锁。
    std::vector<Handler> snapshot;
    snapshot.reserve(handlers_.size());
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [token, handler] : handlers_) snapshot.push_back(handler);
    }
    for (const auto& handler : snapshot) handler(event);
}

} // namespace oml
