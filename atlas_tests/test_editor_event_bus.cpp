/**
 * Tests for EditorEventBus — publish/subscribe event bus for editor panels.
 *
 * Validates:
 * - Subscribe and receive events
 * - Publish with payload
 * - Unsubscribe by handle
 * - Multiple subscribers on same event
 * - SubscriberCount and TotalSubscriptions
 * - Publish to no subscribers is safe
 * - Clear removes all subscriptions
 * - Null callback rejected
 */

#include "../cpp_client/include/editor/editor_event_bus.h"
#include <cassert>
#include <cstdint>
#include <string>

using namespace atlas::editor;

// ── Tests ───────────────────────────────────────────────────────────

void test_evbus_empty_by_default() {
    EditorEventBus bus;
    assert(bus.TotalSubscriptions() == 0);
    assert(bus.SubscriberCount("any") == 0);
}

void test_evbus_subscribe_increments_count() {
    EditorEventBus bus;
    bus.Subscribe("test", [](auto&, auto&) {});
    assert(bus.SubscriberCount("test") == 1);
    assert(bus.TotalSubscriptions() == 1);
}

void test_evbus_subscribe_null_rejected() {
    EditorEventBus bus;
    auto id = bus.Subscribe("test", nullptr);
    assert(id == 0);
    assert(bus.TotalSubscriptions() == 0);
}

void test_evbus_publish_fires_callback() {
    EditorEventBus bus;
    int received = 0;
    bus.Subscribe("ping", [&](const std::string& event, const std::any&) {
        ++received;
        assert(event == "ping");
    });
    bus.Publish("ping");
    assert(received == 1);
}

void test_evbus_publish_with_payload() {
    EditorEventBus bus;
    uint32_t receivedID = 0;
    bus.Subscribe("entity.selected", [&](auto&, const std::any& payload) {
        receivedID = std::any_cast<uint32_t>(payload);
    });
    bus.Publish("entity.selected", uint32_t(42));
    assert(receivedID == 42);
}

void test_evbus_multiple_subscribers() {
    EditorEventBus bus;
    int countA = 0, countB = 0;
    bus.Subscribe("update", [&](auto&, auto&) { ++countA; });
    bus.Subscribe("update", [&](auto&, auto&) { ++countB; });
    assert(bus.SubscriberCount("update") == 2);

    bus.Publish("update");
    assert(countA == 1);
    assert(countB == 1);
}

void test_evbus_unsubscribe() {
    EditorEventBus bus;
    int count = 0;
    auto id = bus.Subscribe("test", [&](auto&, auto&) { ++count; });
    assert(bus.Unsubscribe(id));
    assert(bus.SubscriberCount("test") == 0);

    bus.Publish("test");
    assert(count == 0);  // not called after unsubscribe
}

void test_evbus_unsubscribe_invalid_id() {
    EditorEventBus bus;
    assert(!bus.Unsubscribe(0));
    assert(!bus.Unsubscribe(999));
}

void test_evbus_publish_no_subscribers_safe() {
    EditorEventBus bus;
    bus.Publish("nonexistent");  // should not crash
}

void test_evbus_different_events_independent() {
    EditorEventBus bus;
    int alphaCount = 0, betaCount = 0;
    bus.Subscribe("alpha", [&](auto&, auto&) { ++alphaCount; });
    bus.Subscribe("beta", [&](auto&, auto&) { ++betaCount; });

    bus.Publish("alpha");
    assert(alphaCount == 1);
    assert(betaCount == 0);

    bus.Publish("beta");
    assert(alphaCount == 1);
    assert(betaCount == 1);
}

void test_evbus_clear() {
    EditorEventBus bus;
    bus.Subscribe("a", [](auto&, auto&) {});
    bus.Subscribe("b", [](auto&, auto&) {});
    bus.Subscribe("b", [](auto&, auto&) {});
    assert(bus.TotalSubscriptions() == 3);

    bus.Clear();
    assert(bus.TotalSubscriptions() == 0);
    assert(bus.SubscriberCount("a") == 0);
    assert(bus.SubscriberCount("b") == 0);
}

void test_evbus_string_payload() {
    EditorEventBus bus;
    std::string received;
    bus.Subscribe("message", [&](auto&, const std::any& payload) {
        received = std::any_cast<std::string>(payload);
    });
    bus.Publish("message", std::string("hello"));
    assert(received == "hello");
}

void test_evbus_subscription_ids_unique() {
    EditorEventBus bus;
    auto id1 = bus.Subscribe("test", [](auto&, auto&) {});
    auto id2 = bus.Subscribe("test", [](auto&, auto&) {});
    auto id3 = bus.Subscribe("other", [](auto&, auto&) {});
    assert(id1 != id2);
    assert(id2 != id3);
    assert(id1 != id3);
}
