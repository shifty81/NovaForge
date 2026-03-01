/**
 * Tests for LagCompensation — server-side lag compensation for
 * hit detection using historical entity state rewind.
 */

#include <cassert>
#include <cmath>
#include <vector>
#include "../engine/net/LagCompensation.h"

using namespace atlas::net;

static bool approxEq(float a, float b, float eps = 0.02f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Construction & configuration
// ══════════════════════════════════════════════════════════════════

void test_lc_defaults() {
    LagCompensation lc;
    assert(lc.HistoryDuration() == 64);
    assert(lc.MaxRewindTicks() == 40);
    assert(lc.TrackedEntityCount() == 0);
    assert(lc.CurrentTick() == 0);
}

void test_lc_custom_config() {
    LagCompensation lc(128, 60);
    assert(lc.HistoryDuration() == 128);
    assert(lc.MaxRewindTicks() == 60);
}

void test_lc_zero_history_clamped() {
    LagCompensation lc(0, 10);
    assert(lc.HistoryDuration() == 1);
}

// ══════════════════════════════════════════════════════════════════
// StoreSnapshot basics
// ══════════════════════════════════════════════════════════════════

void test_lc_store_single() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 5.0f, 0.0f, 0.0f, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    assert(lc.TrackedEntityCount() == 1);
    assert(lc.SnapshotCount(1) == 1);
    assert(lc.CurrentTick() == 10);
}

void test_lc_store_multiple_ticks() {
    LagCompensation lc;
    for (uint32_t t = 0; t < 10; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t), 0, 0, 1.0f, 0, 0, 0};
        lc.StoreSnapshot(s);
    }
    assert(lc.SnapshotCount(1) == 10);
    assert(lc.CurrentTick() == 9);
}

void test_lc_store_multiple_entities() {
    LagCompensation lc;
    EntitySnapshot s1{1, 5, 10.0f, 0, 0, 0, 0, 0, 0};
    EntitySnapshot s2{2, 5, 20.0f, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s1);
    lc.StoreSnapshot(s2);

    assert(lc.TrackedEntityCount() == 2);
    assert(lc.SnapshotCount(1) == 1);
    assert(lc.SnapshotCount(2) == 1);
}

void test_lc_store_prunes_old() {
    LagCompensation lc(4, 40); // keep only 4 snapshots
    for (uint32_t t = 0; t < 10; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t), 0, 0, 0, 0, 0, 0};
        lc.StoreSnapshot(s);
    }
    assert(lc.SnapshotCount(1) == 4);
}

// ══════════════════════════════════════════════════════════════════
// GetStateAtTick
// ══════════════════════════════════════════════════════════════════

void test_lc_get_exact_tick() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 100.0f, 200.0f, 300.0f, 0, 0, 0, 45.0f};
    lc.StoreSnapshot(s);

    auto state = lc.GetStateAtTick(1, 10.0f);
    assert(state.valid);
    assert(approxEq(state.posX, 100.0f));
    assert(approxEq(state.posY, 200.0f));
    assert(approxEq(state.posZ, 300.0f));
    assert(approxEq(state.rotYaw, 45.0f));
}

void test_lc_get_interpolated() {
    LagCompensation lc;
    EntitySnapshot s1{1, 10, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0};
    EntitySnapshot s2{1, 20, 100.0f, 0.0f, 0.0f, 0, 0, 0, 90.0f};
    lc.StoreSnapshot(s1);
    lc.StoreSnapshot(s2);

    auto state = lc.GetStateAtTick(1, 15.0f);
    assert(state.valid);
    assert(approxEq(state.posX, 50.0f));
    assert(approxEq(state.rotYaw, 45.0f));
}

void test_lc_get_at_latest_tick() {
    LagCompensation lc;
    EntitySnapshot s1{1, 10, 10.0f, 0, 0, 0, 0, 0, 0};
    EntitySnapshot s2{1, 20, 20.0f, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s1);
    lc.StoreSnapshot(s2);

    auto state = lc.GetStateAtTick(1, 20.0f);
    assert(state.valid);
    assert(approxEq(state.posX, 20.0f));
}

void test_lc_get_beyond_latest_returns_latest() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 50.0f, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    auto state = lc.GetStateAtTick(1, 20.0f);
    assert(state.valid);
    assert(approxEq(state.posX, 50.0f));
}

void test_lc_get_before_earliest_invalid() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 50.0f, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    auto state = lc.GetStateAtTick(1, 5.0f);
    assert(!state.valid);
}

void test_lc_get_unknown_entity() {
    LagCompensation lc;
    auto state = lc.GetStateAtTick(999, 10.0f);
    assert(!state.valid);
}

void test_lc_interpolation_3d() {
    LagCompensation lc;
    EntitySnapshot s1{1, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0};
    EntitySnapshot s2{1, 10, 100.0f, 200.0f, 300.0f, 0, 0, 0, 0};
    lc.StoreSnapshot(s1);
    lc.StoreSnapshot(s2);

    auto state = lc.GetStateAtTick(1, 5.0f); // 50%
    assert(state.valid);
    assert(approxEq(state.posX, 50.0f));
    assert(approxEq(state.posY, 100.0f));
    assert(approxEq(state.posZ, 150.0f));
}

// ══════════════════════════════════════════════════════════════════
// HitTest
// ══════════════════════════════════════════════════════════════════

void test_lc_hit_exact_position() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 100.0f, 0.0f, 0.0f, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    auto result = lc.HitTest(1, 100.0f, 0.0f, 0.0f, 10, 5.0f);
    assert(result.hit);
    assert(result.entityId == 1);
    assert(result.rewindTick == 10);
    assert(approxEq(result.distance, 0.0f));
}

void test_lc_hit_within_radius() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 100.0f, 0.0f, 0.0f, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    // Fire from 3 units away, radius 5
    auto result = lc.HitTest(1, 97.0f, 0.0f, 0.0f, 10, 5.0f);
    assert(result.hit);
    assert(approxEq(result.distance, 3.0f));
}

void test_lc_miss_outside_radius() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 100.0f, 0.0f, 0.0f, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    // Fire from 10 units away, radius 5
    auto result = lc.HitTest(1, 90.0f, 0.0f, 0.0f, 10, 5.0f);
    assert(!result.hit);
    assert(approxEq(result.distance, 10.0f));
}

void test_lc_hit_at_interpolated_position() {
    LagCompensation lc;
    // Entity moves from (0,0,0) at tick 0 to (100,0,0) at tick 10
    EntitySnapshot s1{1, 0, 0.0f, 0.0f, 0.0f, 10.0f, 0, 0, 0};
    EntitySnapshot s2{1, 10, 100.0f, 0.0f, 0.0f, 10.0f, 0, 0, 0};
    lc.StoreSnapshot(s1);
    lc.StoreSnapshot(s2);

    // At tick 5, entity should be at (50, 0, 0)
    // Fire from (50, 0, 0) — should hit
    auto result = lc.HitTest(1, 50.0f, 0.0f, 0.0f, 5, 5.0f);
    assert(result.hit);
    assert(approxEq(result.distance, 0.0f, 0.1f));
}

void test_lc_hit_unknown_target() {
    LagCompensation lc;
    auto result = lc.HitTest(999, 0, 0, 0, 10, 5.0f);
    assert(!result.hit);
}

void test_lc_hit_3d_distance() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 10.0f, 10.0f, 10.0f, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    // Distance from origin (0,0,0) to (10,10,10) = sqrt(300) ≈ 17.32
    auto result = lc.HitTest(1, 0.0f, 0.0f, 0.0f, 10, 20.0f);
    assert(result.hit);
    assert(approxEq(result.distance, 17.32f, 0.1f));

    // Same with a smaller radius — should miss
    auto miss = lc.HitTest(1, 0.0f, 0.0f, 0.0f, 10, 15.0f);
    assert(!miss.hit);
}

// ══════════════════════════════════════════════════════════════════
// Rewind tick clamping
// ══════════════════════════════════════════════════════════════════

void test_lc_clamp_rewind_too_far() {
    LagCompensation lc(64, 10); // max 10 ticks rewind
    // Store ticks 0..50
    for (uint32_t t = 0; t <= 50; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t), 0, 0, 0, 0, 0, 0};
        lc.StoreSnapshot(s);
    }
    assert(lc.CurrentTick() == 50);

    // Try to rewind to tick 0 — should clamp to tick 40 (50 - 10)
    auto result = lc.HitTest(1, 40.0f, 0, 0, 0, 5.0f);
    assert(result.rewindTick == 40);
    assert(result.hit);
    assert(approxEq(result.distance, 0.0f));
}

void test_lc_clamp_rewind_within_range() {
    LagCompensation lc(64, 10);
    for (uint32_t t = 0; t <= 50; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t), 0, 0, 0, 0, 0, 0};
        lc.StoreSnapshot(s);
    }

    // Rewind to tick 45 — within allowed range
    auto result = lc.HitTest(1, 45.0f, 0, 0, 45, 5.0f);
    assert(result.rewindTick == 45);
    assert(result.hit);
}

void test_lc_clamp_rewind_future() {
    LagCompensation lc(64, 10);
    EntitySnapshot s{1, 10, 10.0f, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    // Try to rewind to tick 100 (in the future) — clamp to current
    auto result = lc.HitTest(1, 10.0f, 0, 0, 100, 5.0f);
    assert(result.rewindTick == 10);
    assert(result.hit);
}

// ══════════════════════════════════════════════════════════════════
// Entity removal & clear
// ══════════════════════════════════════════════════════════════════

void test_lc_remove_entity() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 5, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s);
    assert(lc.TrackedEntityCount() == 1);

    lc.RemoveEntity(1);
    assert(lc.TrackedEntityCount() == 0);
    assert(lc.SnapshotCount(1) == 0);
}

void test_lc_remove_nonexistent() {
    LagCompensation lc;
    lc.RemoveEntity(999); // should not crash
    assert(lc.TrackedEntityCount() == 0);
}

void test_lc_clear() {
    LagCompensation lc;
    for (uint32_t i = 0; i < 5; ++i) {
        EntitySnapshot s{i, 10, 0, 0, 0, 0, 0, 0, 0};
        lc.StoreSnapshot(s);
    }
    assert(lc.TrackedEntityCount() == 5);

    lc.Clear();
    assert(lc.TrackedEntityCount() == 0);
    assert(lc.CurrentTick() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Realistic scenario
// ══════════════════════════════════════════════════════════════════

void test_lc_realistic_combat_scenario() {
    // Simulate a 20 Hz server with 100ms RTT (2 tick rewind)
    LagCompensation lc(64, 40);

    // Entity 1 (target) moves along X axis at 10 units/tick
    for (uint32_t t = 0; t <= 20; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t * 10), 0, 0, 10.0f, 0, 0, 0};
        lc.StoreSnapshot(s);
    }

    // Player fires at tick 20, saw target at tick 18 (RTT/2 = 1 tick each way)
    // At tick 18, target was at x=180
    auto result = lc.HitTest(1, 180.0f, 0, 0, 18, 5.0f);
    assert(result.hit);
    assert(result.rewindTick == 18);
    assert(approxEq(result.distance, 0.0f, 0.1f));

    // Player fires at current position (tick 20, x=200) but target already moved
    // If we rewind to tick 18, target is at 180, not 200 — miss with radius 5
    auto miss = lc.HitTest(1, 200.0f, 0, 0, 18, 5.0f);
    assert(!miss.hit);
    assert(approxEq(miss.distance, 20.0f, 0.1f));
}

void test_lc_high_latency_scenario() {
    // Player with 400ms RTT at 20 Hz = 4 tick rewind
    LagCompensation lc(64, 40);

    for (uint32_t t = 0; t <= 30; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t * 5), 0, 0, 5.0f, 0, 0, 0};
        lc.StoreSnapshot(s);
    }

    // Player at tick 30 saw target at tick 26 (4 ticks ago)
    // Target at tick 26: x = 130
    auto result = lc.HitTest(1, 130.0f, 0, 0, 26, 5.0f);
    assert(result.hit);
    assert(result.rewindTick == 26);
}

// ══════════════════════════════════════════════════════════════════
// Edge cases
// ══════════════════════════════════════════════════════════════════

void test_lc_boundary_hit_radius() {
    LagCompensation lc;
    EntitySnapshot s{1, 10, 100.0f, 0, 0, 0, 0, 0, 0};
    lc.StoreSnapshot(s);

    // Exactly at the radius boundary
    auto result = lc.HitTest(1, 95.0f, 0, 0, 10, 5.0f);
    assert(result.hit); // distance = 5.0, radius = 5.0 → hit (<=)
    assert(approxEq(result.distance, 5.0f));
}

void test_lc_snapshot_count_nonexistent() {
    LagCompensation lc;
    assert(lc.SnapshotCount(42) == 0);
}

void test_lc_zero_max_rewind() {
    // Max rewind of 0 = only current tick allowed
    LagCompensation lc(64, 0);
    for (uint32_t t = 0; t <= 10; ++t) {
        EntitySnapshot s{1, t, static_cast<float>(t), 0, 0, 0, 0, 0, 0};
        lc.StoreSnapshot(s);
    }

    // Rewind to tick 5 should clamp to tick 10 (current)
    auto result = lc.HitTest(1, 10.0f, 0, 0, 5, 5.0f);
    assert(result.rewindTick == 10);
    assert(result.hit);
}
