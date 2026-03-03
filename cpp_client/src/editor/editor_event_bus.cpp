#include "editor/editor_event_bus.h"
#include <algorithm>

namespace atlas::editor {

SubscriptionID EditorEventBus::Subscribe(const std::string& event, Handler handler) {
    SubscriptionID id = m_nextID++;
    m_subs[event].push_back({id, std::move(handler)});
    return id;
}

bool EditorEventBus::Unsubscribe(SubscriptionID id) {
    for (auto& [event, subs] : m_subs) {
        auto it = std::find_if(subs.begin(), subs.end(),
                               [id](const Subscription& s) { return s.id == id; });
        if (it != subs.end()) {
            subs.erase(it);
            return true;
        }
    }
    return false;
}

size_t EditorEventBus::Publish(const std::string& event, const std::any& payload) {
    auto it = m_subs.find(event);
    if (it == m_subs.end()) return 0;

    // Copy the subscriber list to guard against modification during dispatch.
    auto subscribers = it->second;
    for (const auto& sub : subscribers) {
        sub.handler(event, payload);
    }
    return subscribers.size();
}

size_t EditorEventBus::SubscriberCount(const std::string& event) const {
    auto it = m_subs.find(event);
    if (it == m_subs.end()) return 0;
    return it->second.size();
}

size_t EditorEventBus::TotalSubscriptions() const {
    size_t total = 0;
    for (const auto& [event, subs] : m_subs) {
        total += subs.size();
    }
    return total;
}

void EditorEventBus::Clear() {
    m_subs.clear();
}

} // namespace atlas::editor
