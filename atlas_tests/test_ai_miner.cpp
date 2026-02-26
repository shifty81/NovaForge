/**
 * Tests for AIMinerStateMachine — autonomous mining AI with
 * state transitions: Idle → SelectTarget → TravelToField → Mining →
 * CargoFull → ReturnToStation → SellOre → repeat.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AIMinerStateMachine.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_miner_defaults() {
    AIMinerStateMachine sm;
    assert(sm.MinerCount() == 0);
    assert(approxEq(sm.TotalEarnings(), 0.0f));
    assert(sm.TotalCycles() == 0);
    assert(approxEq(sm.FieldDistance(), 500.0f));
}

void test_miner_add() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 400.0f;
    assert(sm.AddMiner(cfg));
    assert(sm.MinerCount() == 1);
    const MinerRuntime* rt = sm.GetMiner(1);
    assert(rt != nullptr);
    assert(rt->state == MinerState::Idle);
    assert(approxEq(rt->cargoFill, 0.0f));
}

void test_miner_add_duplicate_rejected() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    assert(sm.AddMiner(cfg));
    assert(!sm.AddMiner(cfg));
    assert(sm.MinerCount() == 1);
}

void test_miner_remove() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    sm.AddMiner(cfg);
    assert(sm.RemoveMiner(1));
    assert(sm.MinerCount() == 0);
    assert(sm.GetMiner(1) == nullptr);
}

void test_miner_remove_nonexistent() {
    AIMinerStateMachine sm;
    assert(!sm.RemoveMiner(999));
}

// ══════════════════════════════════════════════════════════════════
// State transitions without deposits
// ══════════════════════════════════════════════════════════════════

void test_miner_idle_without_deposits() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    sm.AddMiner(cfg);
    // No deposits set → should cycle Idle → SelectTarget → Idle
    sm.Tick(0.1f);
    sm.Tick(0.1f);
    const MinerRuntime* rt = sm.GetMiner(1);
    assert(rt->state == MinerState::Idle || rt->state == MinerState::SelectTarget);
}

// ══════════════════════════════════════════════════════════════════
// Full mining cycle
// ══════════════════════════════════════════════════════════════════

void test_miner_full_cycle() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 100.0f;
    cfg.miningYieldPerSec = 50.0f;    // fills 100m³ in 2s
    cfg.travelSpeed = 500.0f;         // 500/500 = 1s travel
    cfg.sellPricePerUnit = 10.0f;
    sm.AddMiner(cfg);
    sm.SetFieldDistance(500.0f);
    sm.SetAvailableDeposits({100});

    // Tick through: Idle → SelectTarget → TravelToField (1s) → Mining (2s) →
    //   CargoFull → ReturnToStation (1s) → SellOre → Idle
    // Run enough ticks to complete a full cycle
    for (int i = 0; i < 100; ++i) {
        sm.Tick(0.1f);
    }

    const MinerRuntime* rt = sm.GetMiner(1);
    assert(rt->cyclesCompleted >= 1);
    assert(approxEq(rt->totalEarnings, 1000.0f, 1.0f)); // 100 * 10 = 1000
}

void test_miner_travel_progress() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 100.0f;
    sm.AddMiner(cfg);
    sm.SetFieldDistance(500.0f);
    sm.SetAvailableDeposits({100});

    // Idle → SelectTarget → TravelToField
    sm.Tick(0.01f); // Idle → SelectTarget
    sm.Tick(0.01f); // SelectTarget → TravelToField

    const MinerRuntime* rt = sm.GetMiner(1);
    assert(rt->state == MinerState::TravelToField);

    // Travel: 500 / 100 = 5 seconds
    sm.Tick(2.5f);
    rt = sm.GetMiner(1);
    assert(rt->state == MinerState::TravelToField);
    assert(approxEq(rt->travelProgress, 0.5f, 0.05f));
}

void test_miner_cargo_accumulation() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 1000.0f;
    cfg.miningYieldPerSec = 10.0f;
    cfg.travelSpeed = 10000.0f; // Very fast travel
    sm.AddMiner(cfg);
    sm.SetFieldDistance(100.0f);
    sm.SetAvailableDeposits({100});

    // Get to mining state quickly
    for (int i = 0; i < 10; ++i) sm.Tick(0.1f);

    const MinerRuntime* rt = sm.GetMiner(1);
    // Should be mining or have started mining
    assert(rt->state == MinerState::Mining || rt->cargoFill > 0.0f);
}

// ══════════════════════════════════════════════════════════════════
// Death and respawn
// ══════════════════════════════════════════════════════════════════

void test_miner_kill() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 5.0f;
    sm.AddMiner(cfg);
    sm.SetAvailableDeposits({100});

    assert(sm.KillMiner(1));
    const MinerRuntime* rt = sm.GetMiner(1);
    assert(rt->state == MinerState::Dead);
    assert(approxEq(rt->cargoFill, 0.0f));
}

void test_miner_kill_nonexistent() {
    AIMinerStateMachine sm;
    assert(!sm.KillMiner(999));
}

void test_miner_respawn() {
    AIMinerStateMachine sm;
    MinerConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 2.0f;
    sm.AddMiner(cfg);

    sm.KillMiner(1);
    assert(sm.GetMiner(1)->state == MinerState::Dead);

    sm.Tick(3.0f); // Wait longer than respawn delay
    assert(sm.GetMiner(1)->state != MinerState::Dead);
}

// ══════════════════════════════════════════════════════════════════
// Count and aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_miner_count_in_state() {
    AIMinerStateMachine sm;
    MinerConfig c1;
    c1.entityId = 1;
    MinerConfig c2;
    c2.entityId = 2;
    sm.AddMiner(c1);
    sm.AddMiner(c2);

    assert(sm.CountInState(MinerState::Idle) == 2);
    sm.KillMiner(1);
    assert(sm.CountInState(MinerState::Dead) == 1);
    assert(sm.CountInState(MinerState::Idle) == 1);
}

void test_miner_field_distance() {
    AIMinerStateMachine sm;
    sm.SetFieldDistance(1000.0f);
    assert(approxEq(sm.FieldDistance(), 1000.0f));
}

void test_miner_multiple_miners() {
    AIMinerStateMachine sm;
    for (uint32_t i = 1; i <= 5; ++i) {
        MinerConfig cfg;
        cfg.entityId = i;
        sm.AddMiner(cfg);
    }
    assert(sm.MinerCount() == 5);
}
