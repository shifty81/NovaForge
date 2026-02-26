/**
 * Tests for AIPirateStateMachine — autonomous pirate AI with
 * state transitions: Idle → Patrol → ScanForTargets → PursueTarget →
 * Attack → LootCargo → repeat, or Flee on low HP.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AIPirateStateMachine.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_pirate_defaults() {
    AIPirateStateMachine sm;
    assert(sm.PirateCount() == 0);
    assert(sm.TotalKills() == 0);
    assert(approxEq(sm.TotalLoot(), 0.0f));
    assert(approxEq(sm.PatrolDistance(), 600.0f));
}

void test_pirate_add() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 100.0f;
    assert(sm.AddPirate(cfg));
    assert(sm.PirateCount() == 1);
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt != nullptr);
    assert(rt->state == PirateState::Idle);
    assert(approxEq(rt->currentHP, 100.0f));
}

void test_pirate_add_duplicate_rejected() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    assert(sm.AddPirate(cfg));
    assert(!sm.AddPirate(cfg));
    assert(sm.PirateCount() == 1);
}

void test_pirate_remove() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    sm.AddPirate(cfg);
    assert(sm.RemovePirate(1));
    assert(sm.PirateCount() == 0);
    assert(sm.GetPirate(1) == nullptr);
}

void test_pirate_remove_nonexistent() {
    AIPirateStateMachine sm;
    assert(!sm.RemovePirate(999));
}

// ══════════════════════════════════════════════════════════════════
// State transitions without targets
// ══════════════════════════════════════════════════════════════════

void test_pirate_patrol_without_targets() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 600.0f; // fast patrol
    sm.AddPirate(cfg);
    sm.SetPatrolDistance(600.0f);

    // Idle → Patrol
    sm.Tick(0.01f);
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt->state == PirateState::Patrol);

    // Complete patrol (1s at 600 speed / 600 dist)
    for (int i = 0; i < 12; ++i) sm.Tick(0.1f);
    rt = sm.GetPirate(1);
    // Should be scanning or back to patrol (no targets)
    assert(rt->state == PirateState::ScanForTargets ||
           rt->state == PirateState::Patrol);
}

// ══════════════════════════════════════════════════════════════════
// Full attack cycle
// ══════════════════════════════════════════════════════════════════

void test_pirate_full_attack_cycle() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 100.0f;
    cfg.travelSpeed = 600.0f;
    cfg.scanRange = 300.0f;
    cfg.lootCapacity = 200.0f;
    sm.AddPirate(cfg);
    sm.SetPatrolDistance(600.0f);
    sm.SetTargetsInRange({200});

    // Run enough ticks for: Idle→Patrol→Scan→Pursue→Attack(3s)→Loot→...
    for (int i = 0; i < 200; ++i) {
        sm.Tick(0.1f);
    }

    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt->killCount >= 1);
}

void test_pirate_travel_progress() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 100.0f;
    sm.AddPirate(cfg);
    sm.SetPatrolDistance(500.0f);

    // Idle → Patrol
    sm.Tick(0.01f);
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt->state == PirateState::Patrol);

    // Travel: 500 / 100 = 5 seconds, tick 2.5s
    sm.Tick(2.5f);
    rt = sm.GetPirate(1);
    assert(rt->state == PirateState::Patrol);
    assert(approxEq(rt->travelProgress, 0.5f, 0.05f));
}

// ══════════════════════════════════════════════════════════════════
// Damage and death
// ══════════════════════════════════════════════════════════════════

void test_pirate_damage() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 100.0f;
    sm.AddPirate(cfg);

    assert(sm.DamagePirate(1, 30.0f));
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(approxEq(rt->currentHP, 70.0f));
}

void test_pirate_damage_kills() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 50.0f;
    sm.AddPirate(cfg);

    assert(sm.DamagePirate(1, 60.0f));
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt->state == PirateState::Dead);
    assert(approxEq(rt->currentHP, 0.0f));
}

void test_pirate_damage_nonexistent() {
    AIPirateStateMachine sm;
    assert(!sm.DamagePirate(999, 10.0f));
}

void test_pirate_kill() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 5.0f;
    sm.AddPirate(cfg);

    assert(sm.KillPirate(1));
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt->state == PirateState::Dead);
    assert(approxEq(rt->currentHP, 0.0f));
    assert(approxEq(rt->lootFill, 0.0f));
}

void test_pirate_kill_nonexistent() {
    AIPirateStateMachine sm;
    assert(!sm.KillPirate(999));
}

void test_pirate_respawn() {
    AIPirateStateMachine sm;
    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 100.0f;
    cfg.respawnDelay = 2.0f;
    sm.AddPirate(cfg);

    sm.KillPirate(1);
    assert(sm.GetPirate(1)->state == PirateState::Dead);

    sm.Tick(3.0f); // Wait longer than respawn delay
    const PirateRuntime* rt = sm.GetPirate(1);
    assert(rt->state != PirateState::Dead);
    assert(approxEq(rt->currentHP, 100.0f)); // HP restored
}

// ══════════════════════════════════════════════════════════════════
// Count and aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_pirate_count_in_state() {
    AIPirateStateMachine sm;
    PirateConfig c1;
    c1.entityId = 1;
    PirateConfig c2;
    c2.entityId = 2;
    sm.AddPirate(c1);
    sm.AddPirate(c2);

    assert(sm.CountInState(PirateState::Idle) == 2);
    sm.KillPirate(1);
    assert(sm.CountInState(PirateState::Dead) == 1);
    assert(sm.CountInState(PirateState::Idle) == 1);
}

void test_pirate_patrol_distance() {
    AIPirateStateMachine sm;
    sm.SetPatrolDistance(1000.0f);
    assert(approxEq(sm.PatrolDistance(), 1000.0f));
}

void test_pirate_multiple_pirates() {
    AIPirateStateMachine sm;
    for (uint32_t i = 1; i <= 5; ++i) {
        PirateConfig cfg;
        cfg.entityId = i;
        sm.AddPirate(cfg);
    }
    assert(sm.PirateCount() == 5);
}
