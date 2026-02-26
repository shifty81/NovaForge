/**
 * Tests for AIIndustrialistStateMachine — autonomous industrialist AI with
 * state transitions: Idle → SelectBlueprint → AcquireMaterials →
 * Manufacturing → DeliverGoods → SellGoods → repeat.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AIIndustrialistStateMachine.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_industrialist_defaults() {
    AIIndustrialistStateMachine sm;
    assert(sm.IndustrialistCount() == 0);
    assert(approxEq(sm.TotalProfit(), 0.0f));
    assert(sm.TotalJobs() == 0);
    assert(approxEq(sm.MarketDistance(), 400.0f));
}

void test_industrialist_add() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.startingCredits = 10000.0f;
    assert(sm.AddIndustrialist(cfg));
    assert(sm.IndustrialistCount() == 1);
    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt != nullptr);
    assert(rt->state == IndustrialistState::Idle);
    assert(approxEq(rt->credits, 10000.0f));
}

void test_industrialist_add_duplicate_rejected() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    assert(sm.AddIndustrialist(cfg));
    assert(!sm.AddIndustrialist(cfg));
    assert(sm.IndustrialistCount() == 1);
}

void test_industrialist_remove() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    sm.AddIndustrialist(cfg);
    assert(sm.RemoveIndustrialist(1));
    assert(sm.IndustrialistCount() == 0);
}

void test_industrialist_remove_nonexistent() {
    AIIndustrialistStateMachine sm;
    assert(!sm.RemoveIndustrialist(999));
}

// ══════════════════════════════════════════════════════════════════
// State transitions without blueprints
// ══════════════════════════════════════════════════════════════════

void test_industrialist_idle_without_blueprints() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    sm.AddIndustrialist(cfg);
    // No blueprints → should cycle Idle → SelectBlueprint → Idle
    sm.Tick(0.1f);
    sm.Tick(0.1f);
    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->state == IndustrialistState::Idle || rt->state == IndustrialistState::SelectBlueprint);
}

// ══════════════════════════════════════════════════════════════════
// Full manufacturing cycle
// ══════════════════════════════════════════════════════════════════

void test_industrialist_full_cycle() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.productionCapacity = 100.0f;
    cfg.travelSpeed = 400.0f;
    cfg.startingCredits = 10000.0f;
    sm.AddIndustrialist(cfg);
    sm.SetMarketDistance(400.0f);

    // Blueprint: costs 50 credits/unit, sells for 120 credits/unit, 5s build time
    sm.SetAvailableBlueprints({{1, 50.0f, 120.0f, 5.0f, 1.0f}});

    // Run enough ticks to complete a full cycle
    for (int i = 0; i < 200; ++i) {
        sm.Tick(0.1f);
    }

    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->jobsCompleted >= 1);
    assert(rt->totalProfit > 0.0f);
}

void test_industrialist_build_progress() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.startingCredits = 10000.0f;
    cfg.travelSpeed = 100.0f;
    sm.AddIndustrialist(cfg);
    sm.SetAvailableBlueprints({{1, 50.0f, 120.0f, 10.0f, 1.0f}}); // 10s build time

    // Idle → SelectBlueprint → AcquireMaterials → Manufacturing
    sm.Tick(0.01f);
    sm.Tick(0.01f);
    sm.Tick(0.01f);

    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->state == IndustrialistState::Manufacturing);

    // Build: 10s build time, tick 5s
    sm.Tick(5.0f);
    rt = sm.GetIndustrialist(1);
    assert(rt->state == IndustrialistState::Manufacturing);
    assert(approxEq(rt->buildProgress, 0.5f, 0.05f));
}

void test_industrialist_picks_best_blueprint() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.startingCredits = 100000.0f;
    cfg.travelSpeed = 50000.0f;
    sm.AddIndustrialist(cfg);
    sm.SetAvailableBlueprints({
        {10, 50.0f,  60.0f, 5.0f, 1.0f},   // margin 10
        {20, 30.0f, 120.0f, 5.0f, 1.0f},   // margin 90 ← best
        {30, 80.0f, 100.0f, 5.0f, 1.0f}    // margin 20
    });

    // Run ticks until blueprint is selected
    for (int i = 0; i < 5; ++i) sm.Tick(0.01f);

    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->activeBlueprintId == 20);
}

void test_industrialist_cant_afford() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.startingCredits = 10.0f; // Very poor
    sm.AddIndustrialist(cfg);
    sm.SetAvailableBlueprints({{1, 500.0f, 1000.0f, 5.0f, 1.0f}}); // way too expensive

    // Should stay idle since can't afford materials
    for (int i = 0; i < 10; ++i) sm.Tick(0.1f);

    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->jobsCompleted == 0);
}

// ══════════════════════════════════════════════════════════════════
// Death and respawn
// ══════════════════════════════════════════════════════════════════

void test_industrialist_kill() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 5.0f;
    sm.AddIndustrialist(cfg);

    assert(sm.KillIndustrialist(1));
    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->state == IndustrialistState::Dead);
    assert(approxEq(rt->outputFill, 0.0f));
    assert(approxEq(rt->buildProgress, 0.0f));
}

void test_industrialist_kill_nonexistent() {
    AIIndustrialistStateMachine sm;
    assert(!sm.KillIndustrialist(999));
}

void test_industrialist_respawn() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig cfg;
    cfg.entityId = 1;
    cfg.startingCredits = 10000.0f;
    cfg.respawnDelay = 2.0f;
    sm.AddIndustrialist(cfg);

    sm.KillIndustrialist(1);
    assert(sm.GetIndustrialist(1)->state == IndustrialistState::Dead);

    sm.Tick(3.0f);
    const IndustrialistRuntime* rt = sm.GetIndustrialist(1);
    assert(rt->state != IndustrialistState::Dead);
    assert(approxEq(rt->credits, 10000.0f));
}

// ══════════════════════════════════════════════════════════════════
// Count and aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_industrialist_count_in_state() {
    AIIndustrialistStateMachine sm;
    IndustrialistConfig c1;
    c1.entityId = 1;
    IndustrialistConfig c2;
    c2.entityId = 2;
    sm.AddIndustrialist(c1);
    sm.AddIndustrialist(c2);

    assert(sm.CountInState(IndustrialistState::Idle) == 2);
    sm.KillIndustrialist(1);
    assert(sm.CountInState(IndustrialistState::Dead) == 1);
    assert(sm.CountInState(IndustrialistState::Idle) == 1);
}

void test_industrialist_market_distance() {
    AIIndustrialistStateMachine sm;
    sm.SetMarketDistance(800.0f);
    assert(approxEq(sm.MarketDistance(), 800.0f));
}

void test_industrialist_multiple() {
    AIIndustrialistStateMachine sm;
    for (uint32_t i = 1; i <= 5; ++i) {
        IndustrialistConfig cfg;
        cfg.entityId = i;
        sm.AddIndustrialist(cfg);
    }
    assert(sm.IndustrialistCount() == 5);
}
