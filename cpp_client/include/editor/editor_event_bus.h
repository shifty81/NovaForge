#pragma once
/**
 * @file editor_event_bus.h
 * @brief Lightweight pub/sub event bus for editor panel state sync.
 *
 * Named events carry std::any payloads and are dispatched synchronously.
 * Subscribe() returns an opaque handle that can be passed to
 * Unsubscribe() to detach the listener.
 */

#include <any>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::editor {

/** Opaque subscription handle returned by Subscribe(). */
using SubscriptionID = uint64_t;

/**
 * Synchronous pub/sub event bus for the editor.
 *
 * Usage:
 *   EditorEventBus events;
 *   auto id = events.Subscribe("entity.selected",
 *       [](const std::string& event, const std::any& payload) {
 *           auto eid = std::any_cast<uint32_t>(payload);
 *       });
 *   events.Publish("entity.selected", uint32_t(42));
 *   events.Unsubscribe(id);
 */
class EditorEventBus {
public:
    using Handler = std::function<void(const std::string& event,
                                       const std::any& payload)>;

    /** Subscribe to a named event.  Returns a handle for Unsubscribe(). */
    SubscriptionID Subscribe(const std::string& event, Handler handler);

    /**
     * Remove a subscription by handle.
     * Returns true if the subscription was found and removed.
     */
    bool Unsubscribe(SubscriptionID id);

    /**
     * Publish an event to all subscribers of @p event.
     * Dispatch is synchronous; handlers run immediately.
     * Returns the number of handlers invoked.
     */
    size_t Publish(const std::string& event, const std::any& payload = {});

    /** Number of active subscriptions for a specific event. */
    size_t SubscriberCount(const std::string& event) const;

    /** Total number of active subscriptions across all events. */
    size_t TotalSubscriptions() const;

    /** Remove all subscriptions. */
    void Clear();

private:
    struct Subscription {
        SubscriptionID id;
        Handler handler;
    };

    SubscriptionID m_nextID = 1;
    std::unordered_map<std::string, std::vector<Subscription>> m_subs;
};

} // namespace atlas::editor
