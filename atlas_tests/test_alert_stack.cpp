/**
 * Tests for AlertStack — in-game alert/notification queue with
 * priority sorting, timeout, and category filtering.
 */

#include <cassert>
#include <cmath>
#include <string>
#include "../engine/sim/AlertStack.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_alert_defaults() {
    AlertStack stack;
    assert(stack.ActiveCount() == 0);
    assert(stack.MaxAlerts() == 8);
    assert(stack.TotalPushed() == 0);
}

void test_alert_custom_max() {
    AlertStack stack(4);
    assert(stack.MaxAlerts() == 4);
}

// ══════════════════════════════════════════════════════════════════
// Push and query
// ══════════════════════════════════════════════════════════════════

void test_alert_push() {
    AlertStack stack;
    uint32_t id = stack.Push("Test message", "System");
    assert(id == 1);
    assert(stack.ActiveCount() == 1);
    assert(stack.TotalPushed() == 1);
}

void test_alert_push_multiple() {
    AlertStack stack;
    stack.Push("Alert 1", "Combat");
    stack.Push("Alert 2", "Navigation");
    stack.Push("Alert 3", "System");
    assert(stack.ActiveCount() == 3);
    assert(stack.TotalPushed() == 3);
}

void test_alert_active_sorted_by_priority() {
    AlertStack stack;
    stack.Push("Low", "System", AlertPriority::Low);
    stack.Push("Critical", "Combat", AlertPriority::Critical);
    stack.Push("Medium", "Navigation", AlertPriority::Medium);

    auto active = stack.Active();
    assert(active.size() == 3);
    assert(active[0].priority == AlertPriority::Critical);
    assert(active[1].priority == AlertPriority::Medium);
    assert(active[2].priority == AlertPriority::Low);
}

void test_alert_category_filter() {
    AlertStack stack;
    stack.Push("Combat 1", "Combat");
    stack.Push("Nav 1", "Navigation");
    stack.Push("Combat 2", "Combat");

    auto combat = stack.ActiveInCategory("Combat");
    assert(combat.size() == 2);
    auto nav = stack.ActiveInCategory("Navigation");
    assert(nav.size() == 1);
    auto empty = stack.ActiveInCategory("NonExistent");
    assert(empty.size() == 0);
}

// ══════════════════════════════════════════════════════════════════
// TTL / Tick expiry
// ══════════════════════════════════════════════════════════════════

void test_alert_tick_expiry() {
    AlertStack stack;
    stack.Push("Short", "System", AlertPriority::Medium, 2.0f);
    stack.Push("Long", "System", AlertPriority::Medium, 10.0f);
    assert(stack.ActiveCount() == 2);

    stack.Tick(3.0f); // Short expires
    assert(stack.ActiveCount() == 1);

    auto active = stack.Active();
    assert(active[0].message == "Long");
}

void test_alert_tick_ages_alerts() {
    AlertStack stack;
    stack.Push("Test", "System", AlertPriority::Medium, 10.0f);
    stack.Tick(3.0f);

    auto active = stack.Active();
    assert(active.size() == 1);
    assert(approxEq(active[0].age, 3.0f));
    assert(approxEq(active[0].ttl, 7.0f));
}

// ══════════════════════════════════════════════════════════════════
// Dismiss
// ══════════════════════════════════════════════════════════════════

void test_alert_dismiss() {
    AlertStack stack;
    uint32_t id = stack.Push("Test", "System");
    assert(stack.Dismiss(id));
    assert(stack.ActiveCount() == 0);
}

void test_alert_dismiss_nonexistent() {
    AlertStack stack;
    assert(!stack.Dismiss(999));
}

void test_alert_dismiss_category() {
    AlertStack stack;
    stack.Push("C1", "Combat");
    stack.Push("C2", "Combat");
    stack.Push("N1", "Navigation");

    size_t dismissed = stack.DismissCategory("Combat");
    assert(dismissed == 2);
    assert(stack.ActiveCount() == 1);
}

// ══════════════════════════════════════════════════════════════════
// Eviction
// ══════════════════════════════════════════════════════════════════

void test_alert_eviction() {
    AlertStack stack(3);
    stack.Push("First", "System", AlertPriority::Low, 10.0f);
    stack.Push("Second", "System", AlertPriority::Medium, 10.0f);
    stack.Push("Third", "System", AlertPriority::High, 10.0f);
    stack.Push("Fourth", "System", AlertPriority::Critical, 10.0f);

    // Max is 3, so the lowest-priority oldest ("First") should be evicted
    assert(stack.ActiveCount() == 3);
    auto active = stack.Active();
    // Should not contain "First" (Low priority)
    for (auto& a : active) {
        assert(a.message != "First");
    }
}

// ══════════════════════════════════════════════════════════════════
// Clear
// ══════════════════════════════════════════════════════════════════

void test_alert_clear() {
    AlertStack stack;
    stack.Push("A", "System");
    stack.Push("B", "Combat");
    stack.Clear();
    assert(stack.ActiveCount() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Edge cases
// ══════════════════════════════════════════════════════════════════

void test_alert_ids_monotonic() {
    AlertStack stack;
    uint32_t id1 = stack.Push("A", "System");
    uint32_t id2 = stack.Push("B", "System");
    uint32_t id3 = stack.Push("C", "System");
    assert(id1 < id2);
    assert(id2 < id3);
}

void test_alert_same_priority_ordered_by_age() {
    AlertStack stack;
    stack.Push("Old", "System", AlertPriority::High, 10.0f);
    stack.Tick(2.0f);
    stack.Push("New", "System", AlertPriority::High, 10.0f);

    auto active = stack.Active();
    assert(active.size() == 2);
    // Younger first within same priority
    assert(active[0].message == "New");
    assert(active[1].message == "Old");
}
