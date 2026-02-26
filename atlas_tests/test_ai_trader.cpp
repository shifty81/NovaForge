/**
 * Tests for AITraderStateMachine — autonomous trader AI with
 * state transitions: Idle → EvaluateMarket → TravelToBuy → BuyGoods →
 * TravelToSell → SellGoods → repeat.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AITraderStateMachine.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_trader_defaults() {
    AITraderStateMachine sm;
    assert(sm.TraderCount() == 0);
    assert(approxEq(sm.TotalProfit(), 0.0f));
    assert(sm.TotalTrades() == 0);
    assert(approxEq(sm.DefaultDistance(), 500.0f));
}

void test_trader_add() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 500.0f;
    cfg.startingCredits = 5000.0f;
    assert(sm.AddTrader(cfg));
    assert(sm.TraderCount() == 1);
    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt != nullptr);
    assert(rt->state == TraderState::Idle);
    assert(approxEq(rt->credits, 5000.0f));
}

void test_trader_add_duplicate_rejected() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    assert(sm.AddTrader(cfg));
    assert(!sm.AddTrader(cfg));
    assert(sm.TraderCount() == 1);
}

void test_trader_remove() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    sm.AddTrader(cfg);
    assert(sm.RemoveTrader(1));
    assert(sm.TraderCount() == 0);
}

void test_trader_remove_nonexistent() {
    AITraderStateMachine sm;
    assert(!sm.RemoveTrader(999));
}

// ══════════════════════════════════════════════════════════════════
// State transitions without listings
// ══════════════════════════════════════════════════════════════════

void test_trader_idle_without_listings() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    sm.AddTrader(cfg);
    // No listings → should cycle Idle → EvaluateMarket → Idle
    sm.Tick(0.1f);
    sm.Tick(0.1f);
    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt->state == TraderState::Idle || rt->state == TraderState::EvaluateMarket);
}

// ══════════════════════════════════════════════════════════════════
// Full trade cycle
// ══════════════════════════════════════════════════════════════════

void test_trader_full_cycle() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 100.0f;
    cfg.travelSpeed = 500.0f;     // fast travel
    cfg.startingCredits = 10000.0f;
    sm.AddTrader(cfg);
    sm.SetDefaultDistance(500.0f);

    // Station A: buy at 10, sell at 8; Station B: buy at 15, sell at 20
    // Best trade: buy at A (10), sell at B (20) → margin 10 per m³
    sm.SetMarketListings({
        {100, 10.0f, 8.0f,  500.0f, 250.0f},  // station 100
        {101, 15.0f, 20.0f, 300.0f, 250.0f}   // station 101
    });

    for (int i = 0; i < 200; ++i) {
        sm.Tick(0.1f);
    }

    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt->tradesCompleted >= 1);
    assert(rt->totalProfit > 0.0f);
}

void test_trader_travel_progress() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 100.0f;
    cfg.startingCredits = 10000.0f;
    sm.AddTrader(cfg);
    sm.SetMarketListings({
        {100, 10.0f, 8.0f,  500.0f, 500.0f},  // 500 units away
        {101, 15.0f, 20.0f, 300.0f, 500.0f}    // 500 units away
    });

    // Idle → EvaluateMarket → TravelToBuy
    sm.Tick(0.01f);
    sm.Tick(0.01f);

    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt->state == TraderState::TravelToBuy);

    // Travel: 500 / 100 = 5 seconds, tick 2.5s
    sm.Tick(2.5f);
    rt = sm.GetTrader(1);
    assert(rt->state == TraderState::TravelToBuy);
    assert(approxEq(rt->travelProgress, 0.5f, 0.05f));
}

void test_trader_finds_best_trade() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.cargoCapacity = 100.0f;
    cfg.travelSpeed = 50000.0f;
    cfg.startingCredits = 100000.0f;
    sm.AddTrader(cfg);
    sm.SetMarketListings({
        {100, 10.0f, 5.0f,  500.0f, 100.0f},   // buy 10, sell 5
        {101, 5.0f,  25.0f, 500.0f, 100.0f},    // buy 5, sell 25 ← best buy
        {102, 8.0f,  30.0f, 500.0f, 100.0f}     // buy 8, sell 30 ← best sell
    });

    // Run several ticks to find trade and start traveling
    for (int i = 0; i < 5; ++i) sm.Tick(0.1f);

    const TraderRuntime* rt = sm.GetTrader(1);
    // Best trade: buy at station 101 (5), sell at station 102 (30) → margin 25
    assert(rt->buyStationId == 101);
    assert(rt->sellStationId == 102);
}

// ══════════════════════════════════════════════════════════════════
// Damage and death
// ══════════════════════════════════════════════════════════════════

void test_trader_damage() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 100.0f;
    sm.AddTrader(cfg);

    assert(sm.DamageTrader(1, 30.0f));
    const TraderRuntime* rt = sm.GetTrader(1);
    assert(approxEq(rt->currentHP, 70.0f));
    assert(rt->state != TraderState::Dead);
}

void test_trader_damage_kills() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 50.0f;
    sm.AddTrader(cfg);

    assert(sm.DamageTrader(1, 60.0f));
    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt->state == TraderState::Dead);
    assert(approxEq(rt->currentHP, 0.0f));
    assert(approxEq(rt->cargoFill, 0.0f));
}

void test_trader_damage_nonexistent() {
    AITraderStateMachine sm;
    assert(!sm.DamageTrader(999, 10.0f));
}

void test_trader_kill() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 5.0f;
    sm.AddTrader(cfg);

    assert(sm.KillTrader(1));
    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt->state == TraderState::Dead);
    assert(approxEq(rt->cargoFill, 0.0f));
}

void test_trader_kill_nonexistent() {
    AITraderStateMachine sm;
    assert(!sm.KillTrader(999));
}

void test_trader_respawn() {
    AITraderStateMachine sm;
    TraderConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 80.0f;
    cfg.startingCredits = 5000.0f;
    cfg.respawnDelay = 2.0f;
    sm.AddTrader(cfg);

    sm.KillTrader(1);
    assert(sm.GetTrader(1)->state == TraderState::Dead);

    sm.Tick(3.0f);
    const TraderRuntime* rt = sm.GetTrader(1);
    assert(rt->state != TraderState::Dead);
    assert(approxEq(rt->currentHP, 80.0f));
    assert(approxEq(rt->credits, 5000.0f));
}

// ══════════════════════════════════════════════════════════════════
// Count and aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_trader_count_in_state() {
    AITraderStateMachine sm;
    TraderConfig c1;
    c1.entityId = 1;
    TraderConfig c2;
    c2.entityId = 2;
    sm.AddTrader(c1);
    sm.AddTrader(c2);

    assert(sm.CountInState(TraderState::Idle) == 2);
    sm.KillTrader(1);
    assert(sm.CountInState(TraderState::Dead) == 1);
    assert(sm.CountInState(TraderState::Idle) == 1);
}

void test_trader_default_distance() {
    AITraderStateMachine sm;
    sm.SetDefaultDistance(800.0f);
    assert(approxEq(sm.DefaultDistance(), 800.0f));
}

void test_trader_multiple_traders() {
    AITraderStateMachine sm;
    for (uint32_t i = 1; i <= 5; ++i) {
        TraderConfig cfg;
        cfg.entityId = i;
        sm.AddTrader(cfg);
    }
    assert(sm.TraderCount() == 5);
}
