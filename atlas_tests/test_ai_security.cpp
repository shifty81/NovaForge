/**
 * Tests for AISecurityStateMachine — autonomous security AI with
 * state transitions: Idle → Standby → ScanForThreats → WarpToThreat →
 * EngageThreat → ReturnToPost → repeat.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AISecurityStateMachine.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_security_defaults() {
    AISecurityStateMachine sm;
    assert(sm.GuardCount() == 0);
    assert(sm.TotalNeutralized() == 0);
    assert(approxEq(sm.PostDistance(), 400.0f));
}

void test_security_add() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 200.0f;
    assert(sm.AddGuard(cfg));
    assert(sm.GuardCount() == 1);
    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt != nullptr);
    assert(rt->state == SecurityState::Idle);
    assert(approxEq(rt->currentHP, 200.0f));
}

void test_security_add_duplicate_rejected() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    assert(sm.AddGuard(cfg));
    assert(!sm.AddGuard(cfg));
    assert(sm.GuardCount() == 1);
}

void test_security_remove() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    sm.AddGuard(cfg);
    assert(sm.RemoveGuard(1));
    assert(sm.GuardCount() == 0);
    assert(sm.GetGuard(1) == nullptr);
}

void test_security_remove_nonexistent() {
    AISecurityStateMachine sm;
    assert(!sm.RemoveGuard(999));
}

// ══════════════════════════════════════════════════════════════════
// State transitions without threats
// ══════════════════════════════════════════════════════════════════

void test_security_standby_without_threats() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.responseTime = 1.0f;
    sm.AddGuard(cfg);

    // Idle → Standby
    sm.Tick(0.01f);
    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt->state == SecurityState::Standby);

    // Wait for responseTime → ScanForThreats → no threats → Standby
    for (int i = 0; i < 20; ++i) sm.Tick(0.1f);
    rt = sm.GetGuard(1);
    assert(rt->state == SecurityState::Standby ||
           rt->state == SecurityState::ScanForThreats);
}

// ══════════════════════════════════════════════════════════════════
// Full engagement cycle
// ══════════════════════════════════════════════════════════════════

void test_security_full_engagement_cycle() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 200.0f;
    cfg.travelSpeed = 400.0f;
    cfg.responseTime = 1.0f;
    sm.AddGuard(cfg);
    sm.SetPostDistance(400.0f);
    sm.SetActiveThreats({300});

    // Run enough ticks for: Idle→Standby(1s)→Scan→Warp(1s)→Engage(5s)→Return(1s)
    for (int i = 0; i < 200; ++i) {
        sm.Tick(0.1f);
    }

    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt->threatsNeutralized >= 1);
}

void test_security_warp_progress() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 100.0f;
    cfg.responseTime = 0.01f; // Very fast response
    sm.AddGuard(cfg);
    sm.SetPostDistance(500.0f);
    sm.SetActiveThreats({300});

    // Idle → Standby → ScanForThreats → WarpToThreat
    sm.Tick(0.01f); // Idle → Standby
    sm.Tick(0.02f); // Standby (responseTime met) → ScanForThreats
    sm.Tick(0.01f); // ScanForThreats → WarpToThreat

    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt->state == SecurityState::WarpToThreat);

    // Travel: 500 / 100 = 5 seconds
    sm.Tick(2.5f);
    rt = sm.GetGuard(1);
    assert(rt->state == SecurityState::WarpToThreat);
    assert(approxEq(rt->travelProgress, 0.5f, 0.05f));
}

// ══════════════════════════════════════════════════════════════════
// Damage and death
// ══════════════════════════════════════════════════════════════════

void test_security_damage() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 200.0f;
    sm.AddGuard(cfg);

    assert(sm.DamageGuard(1, 50.0f));
    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(approxEq(rt->currentHP, 150.0f));
}

void test_security_damage_kills() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 80.0f;
    sm.AddGuard(cfg);

    assert(sm.DamageGuard(1, 100.0f));
    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt->state == SecurityState::Dead);
    assert(approxEq(rt->currentHP, 0.0f));
}

void test_security_damage_nonexistent() {
    AISecurityStateMachine sm;
    assert(!sm.DamageGuard(999, 10.0f));
}

void test_security_kill() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.respawnDelay = 5.0f;
    sm.AddGuard(cfg);

    assert(sm.KillGuard(1));
    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt->state == SecurityState::Dead);
    assert(approxEq(rt->currentHP, 0.0f));
}

void test_security_kill_nonexistent() {
    AISecurityStateMachine sm;
    assert(!sm.KillGuard(999));
}

void test_security_respawn() {
    AISecurityStateMachine sm;
    SecurityConfig cfg;
    cfg.entityId = 1;
    cfg.hitPoints = 200.0f;
    cfg.respawnDelay = 2.0f;
    sm.AddGuard(cfg);

    sm.KillGuard(1);
    assert(sm.GetGuard(1)->state == SecurityState::Dead);

    sm.Tick(3.0f); // Wait longer than respawn delay
    const SecurityRuntime* rt = sm.GetGuard(1);
    assert(rt->state != SecurityState::Dead);
    assert(approxEq(rt->currentHP, 200.0f)); // HP restored
}

// ══════════════════════════════════════════════════════════════════
// Count and aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_security_count_in_state() {
    AISecurityStateMachine sm;
    SecurityConfig c1;
    c1.entityId = 1;
    SecurityConfig c2;
    c2.entityId = 2;
    sm.AddGuard(c1);
    sm.AddGuard(c2);

    assert(sm.CountInState(SecurityState::Idle) == 2);
    sm.KillGuard(1);
    assert(sm.CountInState(SecurityState::Dead) == 1);
    assert(sm.CountInState(SecurityState::Idle) == 1);
}

void test_security_post_distance() {
    AISecurityStateMachine sm;
    sm.SetPostDistance(800.0f);
    assert(approxEq(sm.PostDistance(), 800.0f));
}

void test_security_multiple_guards() {
    AISecurityStateMachine sm;
    for (uint32_t i = 1; i <= 5; ++i) {
        SecurityConfig cfg;
        cfg.entityId = i;
        sm.AddGuard(cfg);
    }
    assert(sm.GuardCount() == 5);
}
