#pragma once
/**
 * @file editor_event_bus.h
 * @brief Lightweight publish/subscribe event bus for editor panels.
 *
 * Allows editor panels and tools to broadcast state-change notifications
 * without direct coupling.  Subscribers register a callback for a named
 * event; publishers fire events by name with an optional payload.
 *
 * Events are delivered synchronously (immediate dispatch) so that
 * handlers can react within the same frame.
 */

#include <any>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::editor {

/** Callback type: receives the event name and an optional payload. */
using EventCallback = std::function<void(const std::string& event, const std::any& payload)>;

/**
 * Simple event bus for editor-side communication.
 *
 * Usage:
 *   EditorEventBus bus;
 *   auto id = bus.Subscribe("entity.selected", [](auto& e, auto& p) {
 *       auto eid = std::any_cast<uint32_t>(p);
 *       // update inspector panel...
 *   });
 *
 *   bus.Publish("entity.selected", entityID);
 *
 *   bus.Unsubscribe(id);
 */
class EditorEventBus {
public:
    /** Opaque subscription handle for unsubscribing. */
    using SubscriptionID = uint64_t;

    /**
     * Subscribe to a named event.
     * @return A handle that can be passed to Unsubscribe().
     */
    SubscriptionID Subscribe(const std::string& event, EventCallback callback);

    /** Remove a subscription by handle. Returns true if found and removed. */
    bool Unsubscribe(SubscriptionID id);

    /** Publish an event with an optional payload. Invokes all matching subscribers synchronously. */
    void Publish(const std::string& event, const std::any& payload = {});

    /** Number of subscribers for a specific event. */
    size_t SubscriberCount(const std::string& event) const;

    /** Total number of active subscriptions. */
    size_t TotalSubscriptions() const;

    /** Remove all subscriptions. */
    void Clear();

private:
    struct Subscription {
        SubscriptionID id;
        EventCallback callback;
    };

    std::unordered_map<std::string, std::vector<Subscription>> m_subscribers;
    SubscriptionID m_nextID = 1;
};

} // namespace atlas::editor
