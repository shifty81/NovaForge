#include "editor/editor_event_bus.h"
#include <algorithm>

namespace atlas::editor {

EditorEventBus::SubscriptionID EditorEventBus::Subscribe(
        const std::string& event, EventCallback callback) {
    if (!callback) return 0;
    SubscriptionID id = m_nextID++;
    m_subscribers[event].push_back({id, std::move(callback)});
    return id;
}

bool EditorEventBus::Unsubscribe(SubscriptionID id) {
    if (id == 0) return false;
    for (auto& [event, subs] : m_subscribers) {
        auto it = std::find_if(subs.begin(), subs.end(),
            [id](const Subscription& s) { return s.id == id; });
        if (it != subs.end()) {
            subs.erase(it);
            return true;
        }
    }
    return false;
}

void EditorEventBus::Publish(const std::string& event, const std::any& payload) {
    auto it = m_subscribers.find(event);
    if (it == m_subscribers.end()) return;
    // Copy the subscriber list in case a callback modifies subscriptions.
    auto subs = it->second;
    for (const auto& sub : subs) {
        sub.callback(event, payload);
    }
}

size_t EditorEventBus::SubscriberCount(const std::string& event) const {
    auto it = m_subscribers.find(event);
    if (it == m_subscribers.end()) return 0;
    return it->second.size();
}

size_t EditorEventBus::TotalSubscriptions() const {
    size_t total = 0;
    for (const auto& [event, subs] : m_subscribers) {
        total += subs.size();
    }
    return total;
}

void EditorEventBus::Clear() {
    m_subscribers.clear();
}

} // namespace atlas::editor
