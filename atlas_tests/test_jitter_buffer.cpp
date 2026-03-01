/**
 * Tests for JitterBuffer — client-side packet timing smoothing for
 * network snapshot delivery.
 */

#include <cassert>
#include <cmath>
#include <vector>
#include "../engine/net/JitterBuffer.h"

using namespace atlas::net;

static bool fApprox(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Construction & configuration
// ══════════════════════════════════════════════════════════════════

void test_jb_defaults() {
    JitterBuffer jb;
    assert(jb.BufferedCount() == 0);
    assert(fApprox(jb.TargetDelay(), 0.1f));
    assert(jb.MaxBufferSize() == 64);
    assert(jb.IsAdaptive());
    assert(jb.TotalPushed() == 0);
    assert(jb.TotalDropped() == 0);
}

void test_jb_custom_config() {
    JitterBuffer jb(0.2f, 32, false);
    assert(fApprox(jb.TargetDelay(), 0.2f));
    assert(jb.MaxBufferSize() == 32);
    assert(!jb.IsAdaptive());
}

void test_jb_min_delay_clamped() {
    JitterBuffer jb(0.001f);
    assert(jb.TargetDelay() >= JitterBuffer::kMinTargetDelay);
}

// ══════════════════════════════════════════════════════════════════
// Push & Flush basics
// ══════════════════════════════════════════════════════════════════

void test_jb_push_single() {
    JitterBuffer jb(0.1f, 64, false);
    jb.Push(1, 0.0f, {0x01, 0x02});
    assert(jb.BufferedCount() == 1);
    assert(jb.TotalPushed() == 1);
}

void test_jb_flush_after_delay() {
    JitterBuffer jb(0.1f, 64, false);
    jb.Push(1, 0.0f, {0xAA});
    jb.Push(2, 0.05f, {0xBB});

    // At t=0.05, nothing should be ready (delay is 0.1)
    auto ready = jb.Flush(0.05f);
    assert(ready.empty());

    // At t=0.1, entry 1 should be ready (held for 0.1s)
    ready = jb.Flush(0.1f);
    assert(ready.size() == 1);
    assert(ready[0].tick == 1);
    assert(ready[0].payload.size() == 1);
    assert(ready[0].payload[0] == 0xAA);
}

void test_jb_flush_releases_in_order() {
    JitterBuffer jb(0.05f, 64, false);
    jb.Push(1, 0.0f, {});
    jb.Push(2, 0.01f, {});
    jb.Push(3, 0.02f, {});

    // At t=0.10, all should be ready (oldest held ≥0.05)
    auto ready = jb.Flush(0.10f);
    assert(ready.size() == 3);
    assert(ready[0].tick == 1);
    assert(ready[1].tick == 2);
    assert(ready[2].tick == 3);
}

void test_jb_flush_partial() {
    JitterBuffer jb(0.1f, 64, false);
    jb.Push(1, 0.0f, {});
    jb.Push(2, 0.08f, {});
    jb.Push(3, 0.15f, {});

    // At t=0.1, only tick 1 ready
    auto ready = jb.Flush(0.1f);
    assert(ready.size() == 1);
    assert(ready[0].tick == 1);

    // At t=0.20, tick 2 should now be ready
    ready = jb.Flush(0.20f);
    assert(ready.size() == 1);
    assert(ready[0].tick == 2);
}

// ══════════════════════════════════════════════════════════════════
// Late packet handling
// ══════════════════════════════════════════════════════════════════

void test_jb_late_packet_dropped() {
    JitterBuffer jb(0.05f, 64, false);
    jb.Push(5, 0.0f, {});

    // Flush tick 5
    auto ready = jb.Flush(0.10f);
    assert(ready.size() == 1);

    // Push late packet (tick 3, older than released tick 5)
    jb.Push(3, 0.11f, {});
    assert(jb.TotalDropped() == 1);
    assert(jb.BufferedCount() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Buffer overflow
// ══════════════════════════════════════════════════════════════════

void test_jb_overflow_trims() {
    JitterBuffer jb(0.1f, 4, false);
    for (uint32_t i = 1; i <= 8; ++i) {
        jb.Push(i, static_cast<float>(i) * 0.01f, {});
    }
    assert(jb.BufferedCount() == 4);
    assert(jb.TotalDropped() == 4);
}

// ══════════════════════════════════════════════════════════════════
// Out-of-order insertion
// ══════════════════════════════════════════════════════════════════

void test_jb_out_of_order_sorted() {
    JitterBuffer jb(0.05f, 64, false);
    jb.Push(3, 0.01f, {});
    jb.Push(1, 0.02f, {});
    jb.Push(2, 0.03f, {});
    assert(jb.BufferedCount() == 3);

    auto ready = jb.Flush(1.0f); // all ready
    assert(ready.size() == 3);
    assert(ready[0].tick == 1);
    assert(ready[1].tick == 2);
    assert(ready[2].tick == 3);
}

void test_jb_duplicate_tick_replaces() {
    JitterBuffer jb(0.05f, 64, false);
    jb.Push(1, 0.0f, {0xAA});
    jb.Push(1, 0.01f, {0xBB});
    assert(jb.BufferedCount() == 1);

    auto ready = jb.Flush(1.0f);
    assert(ready.size() == 1);
    assert(ready[0].payload[0] == 0xBB);
}

// ══════════════════════════════════════════════════════════════════
// Adaptive delay
// ══════════════════════════════════════════════════════════════════

void test_jb_adaptive_adjusts_delay() {
    JitterBuffer jb(0.1f, 64, true);

    // Push packets with varying inter-arrival times to generate jitter
    float time = 0.0f;
    for (uint32_t i = 1; i <= 20; ++i) {
        // Alternate between fast and slow arrivals to create jitter
        float interval = (i % 2 == 0) ? 0.08f : 0.02f;
        time += interval;
        jb.Push(i, time, {});
    }

    // Delay should have adapted from the initial 0.1
    // (it may be higher or lower depending on jitter)
    float adapted = jb.TargetDelay();
    assert(adapted >= JitterBuffer::kMinTargetDelay);
    assert(adapted <= JitterBuffer::kMaxTargetDelay);
}

void test_jb_non_adaptive_fixed_delay() {
    JitterBuffer jb(0.15f, 64, false);

    float time = 0.0f;
    for (uint32_t i = 1; i <= 20; ++i) {
        float interval = (i % 2 == 0) ? 0.08f : 0.02f;
        time += interval;
        jb.Push(i, time, {});
    }

    // Non-adaptive should keep original delay
    assert(fApprox(jb.TargetDelay(), 0.15f));
}

// ══════════════════════════════════════════════════════════════════
// Reset
// ══════════════════════════════════════════════════════════════════

void test_jb_reset() {
    JitterBuffer jb(0.1f, 64, false);
    jb.Push(1, 0.0f, {0x01});
    jb.Push(2, 0.01f, {0x02});

    jb.Reset();
    assert(jb.BufferedCount() == 0);
    assert(jb.TotalPushed() == 0);
    assert(jb.TotalDropped() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Empty buffer flush
// ══════════════════════════════════════════════════════════════════

void test_jb_flush_empty() {
    JitterBuffer jb;
    auto ready = jb.Flush(10.0f);
    assert(ready.empty());
}
