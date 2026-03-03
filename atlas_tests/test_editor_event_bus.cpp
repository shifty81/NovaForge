/**
 * Tests for EditorEventBus.
 *
 * Validates:
 * - Subscribe and receive events
 * - Multiple subscribers on same event
 * - Different events are independent
 * - Unsubscribe by handle
 * - Unsubscribe unknown handle returns false
 * - Publish returns handler count
 * - Publish to no subscribers returns 0
 * - SubscriberCount
 * - TotalSubscriptions
 * - Clear removes all
 * - Payload passes through correctly
 * - Dispatch copies subscriber list (safe during modification)
 * - Event name passed to handler
 */

#include "../cpp_client/include/editor/editor_event_bus.h"
#include <cassert>
#include <string>
#include <vector>
#include <cstdint>

using namespace atlas::editor;

// ── Tests ───────────────────────────────────────────────────────────

void test_eeb_subscribe_and_publish() {
    EditorEventBus bus;
    int received = 0;
    bus.Subscribe("test.event", [&](const std::string&, const std::any& p) {
        received = std::any_cast<int>(p);
    });
    bus.Publish("test.event", 42);
    assert(received == 42);
}

void test_eeb_multiple_subscribers() {
    EditorEventBus bus;
    int count = 0;
    bus.Subscribe("ping", [&](const std::string&, const std::any&) { ++count; });
    bus.Subscribe("ping", [&](const std::string&, const std::any&) { ++count; });
    bus.Publish("ping");
    assert(count == 2);
}

void test_eeb_different_events_independent() {
    EditorEventBus bus;
    int a = 0, b = 0;
    bus.Subscribe("eventA", [&](const std::string&, const std::any&) { ++a; });
    bus.Subscribe("eventB", [&](const std::string&, const std::any&) { ++b; });
    bus.Publish("eventA");
    assert(a == 1);
    assert(b == 0);
}

void test_eeb_unsubscribe() {
    EditorEventBus bus;
    int count = 0;
    auto id = bus.Subscribe("x", [&](const std::string&, const std::any&) { ++count; });
    bus.Publish("x");
    assert(count == 1);

    assert(bus.Unsubscribe(id));
    bus.Publish("x");
    assert(count == 1);  // handler no longer fires
}

void test_eeb_unsubscribe_unknown() {
    EditorEventBus bus;
    assert(!bus.Unsubscribe(999));
}

void test_eeb_publish_returns_count() {
    EditorEventBus bus;
    bus.Subscribe("e", [](const std::string&, const std::any&) {});
    bus.Subscribe("e", [](const std::string&, const std::any&) {});
    assert(bus.Publish("e") == 2);
}

void test_eeb_publish_no_subscribers() {
    EditorEventBus bus;
    assert(bus.Publish("nobody") == 0);
}

void test_eeb_subscriber_count() {
    EditorEventBus bus;
    assert(bus.SubscriberCount("e") == 0);
    bus.Subscribe("e", [](const std::string&, const std::any&) {});
    assert(bus.SubscriberCount("e") == 1);
    bus.Subscribe("e", [](const std::string&, const std::any&) {});
    assert(bus.SubscriberCount("e") == 2);
}

void test_eeb_total_subscriptions() {
    EditorEventBus bus;
    assert(bus.TotalSubscriptions() == 0);
    bus.Subscribe("a", [](const std::string&, const std::any&) {});
    bus.Subscribe("b", [](const std::string&, const std::any&) {});
    bus.Subscribe("a", [](const std::string&, const std::any&) {});
    assert(bus.TotalSubscriptions() == 3);
}

void test_eeb_clear() {
    EditorEventBus bus;
    bus.Subscribe("x", [](const std::string&, const std::any&) {});
    bus.Subscribe("y", [](const std::string&, const std::any&) {});
    bus.Clear();
    assert(bus.TotalSubscriptions() == 0);
    assert(bus.Publish("x") == 0);
}

void test_eeb_payload_passthrough() {
    EditorEventBus bus;
    std::string received;
    bus.Subscribe("msg", [&](const std::string&, const std::any& p) {
        received = std::any_cast<std::string>(p);
    });
    bus.Publish("msg", std::string("hello"));
    assert(received == "hello");
}

void test_eeb_dispatch_copies_list() {
    EditorEventBus bus;
    int count = 0;
    SubscriptionID selfID = 0;
    // A handler that unsubscribes itself during dispatch.
    selfID = bus.Subscribe("self", [&](const std::string&, const std::any&) {
        ++count;
        bus.Unsubscribe(selfID);
    });
    // Should not crash; handler fires once.
    bus.Publish("self");
    assert(count == 1);
    // Second publish should have no subscribers.
    assert(bus.Publish("self") == 0);
}

void test_eeb_event_name_in_handler() {
    EditorEventBus bus;
    std::string receivedEvent;
    bus.Subscribe("named.event", [&](const std::string& event, const std::any&) {
        receivedEvent = event;
    });
    bus.Publish("named.event");
    assert(receivedEvent == "named.event");
}
