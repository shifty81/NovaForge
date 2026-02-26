/**
 * Tests for AIHaulerStateMachine — autonomous hauler AI with
 * state transitions: Idle → FindCargo → TravelToPickup → LoadCargo →
 * TravelToStation → UnloadCargo → repeat.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AIHaulerStateMachine.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_hauler_defaults() {
    AIHaulerStateMachine sm;
    assert(sm.HaulerCount() == 0);
    assert(approxEq(sm.TotalEarnings(), 0.0f));
    assert(sm.TotalTrips() == 0);
    assert(approxEq(sm.StationDistance(), 400.0f));
}

void test_hauler_add() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 1000.0f;
    assert(sm.AddHauler(cfg));
    assert(sm.HaulerCount() == 1);
    const HaulerRuntime* rt = sm.GetHauler(1);
    assert(rt != nullptr);
    assert(rt->state == HaulerState::Idle);
}

void test_hauler_add_duplicate_rejected() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    assert(sm.AddHauler(cfg));
    assert(!sm.AddHauler(cfg));
    assert(sm.HaulerCount() == 1);
}

void test_hauler_remove() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    sm.AddHauler(cfg);
    assert(sm.RemoveHauler(1));
    assert(sm.HaulerCount() == 0);
}

void test_hauler_remove_nonexistent() {
    AIHaulerStateMachine sm;
    assert(!sm.RemoveHauler(999));
}

// ══════════════════════════════════════════════════════════════════
// State transitions without cargo
// ══════════════════════════════════════════════════════════════════

void test_hauler_idle_without_cargo() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    sm.AddHauler(cfg);
    // No cargo requests → should cycle Idle → FindCargo → Idle
    sm.Tick(0.1f);
    sm.Tick(0.1f);
    const HaulerRuntime* rt = sm.GetHauler(1);
    assert(rt->state == HaulerState::Idle || rt->state == HaulerState::FindCargo);
}

// ══════════════════════════════════════════════════════════════════
// Full haul cycle
// ══════════════════════════════════════════════════════════════════

void test_hauler_full_cycle() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 200.0f;
    cfg.travelSpeed = 400.0f;    // 400/400 = 1s to station
    cfg.loadRate = 1000.0f;      // Very fast loading
    cfg.sellPricePerUnit = 12.0f;
    sm.AddHauler(cfg);
    sm.SetStationDistance(400.0f);
    sm.SetCargoRequests({{100, 500.0f, 200.0f}}); // source 100, 500m³, 200 units away

    // Run enough ticks to complete a full cycle
    for (int i = 0; i < 200; ++i) {
        sm.Tick(0.1f);
    }

    const HaulerRuntime* rt = sm.GetHauler(1);
    assert(rt->tripsCompleted >= 1);
    assert(rt->totalEarnings > 0.0f);
}

void test_hauler_travel_progress() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 100.0f;
    sm.AddHauler(cfg);
    sm.SetCargoRequests({{100, 500.0f, 500.0f}}); // 500 units away

    // Idle → FindCargo → TravelToPickup
    sm.Tick(0.01f);
    sm.Tick(0.01f);

    const HaulerRuntime* rt = sm.GetHauler(1);
    assert(rt->state == HaulerState::TravelToPickup);

    // Travel: 500 / 100 = 5 seconds, tick 2.5s
    sm.Tick(2.5f);
    rt = sm.GetHauler(1);
    assert(rt->state == HaulerState::TravelToPickup);
    assert(approxEq(rt->travelProgress, 0.5f, 0.05f));
}

void test_hauler_picks_best_cargo() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 500.0f;
    cfg.travelSpeed = 10000.0f;
    cfg.loadRate = 10000.0f;
    sm.AddHauler(cfg);
    sm.SetCargoRequests({
        {100, 100.0f, 200.0f},  // small cargo
        {101, 800.0f, 300.0f},  // large cargo (should pick this)
        {102, 50.0f, 100.0f}    // tiny cargo
    });

    // Run several ticks
    for (int i = 0; i < 10; ++i) sm.Tick(0.1f);

    const HaulerRuntime* rt = sm.GetHauler(1);
    // Should have targeted source 101 (most available)
    assert(rt->targetSourceId == 101);
}

// ══════════════════════════════════════════════════════════════════
// Death and respawn
// ══════════════════════════════════════════════════════════════════

void test_hauler_kill() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 5.0f;
    sm.AddHauler(cfg);

    assert(sm.KillHauler(1));
    const HaulerRuntime* rt = sm.GetHauler(1);
    assert(rt->state == HaulerState::Dead);
    assert(approxEq(rt->cargoFill, 0.0f));
}

void test_hauler_kill_nonexistent() {
    AIHaulerStateMachine sm;
    assert(!sm.KillHauler(999));
}

void test_hauler_respawn() {
    AIHaulerStateMachine sm;
    HaulerConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 2.0f;
    sm.AddHauler(cfg);

    sm.KillHauler(1);
    assert(sm.GetHauler(1)->state == HaulerState::Dead);

    sm.Tick(3.0f);
    assert(sm.GetHauler(1)->state != HaulerState::Dead);
}

// ══════════════════════════════════════════════════════════════════
// Count and aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_hauler_count_in_state() {
    AIHaulerStateMachine sm;
    HaulerConfig c1;
    c1.entityId = 1;
    HaulerConfig c2;
    c2.entityId = 2;
    sm.AddHauler(c1);
    sm.AddHauler(c2);

    assert(sm.CountInState(HaulerState::Idle) == 2);
    sm.KillHauler(1);
    assert(sm.CountInState(HaulerState::Dead) == 1);
    assert(sm.CountInState(HaulerState::Idle) == 1);
}

void test_hauler_station_distance() {
    AIHaulerStateMachine sm;
    sm.SetStationDistance(800.0f);
    assert(approxEq(sm.StationDistance(), 800.0f));
}

void test_hauler_multiple_haulers() {
    AIHaulerStateMachine sm;
    for (uint32_t i = 1; i <= 5; ++i) {
        HaulerConfig cfg;
        cfg.entityId = i;
        sm.AddHauler(cfg);
    }
    assert(sm.HaulerCount() == 5);
}
