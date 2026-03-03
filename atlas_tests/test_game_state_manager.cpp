/**
 * Tests for GameStateManager.
 *
 * Validates:
 * - Default state
 * - Phase transitions and callbacks
 * - Player entity / star system / station tracking
 * - Credits and spending
 * - Inventory management
 * - Reset
 * - Phase name strings
 */

#include "../engine/core/GameStateManager.h"
#include <cassert>
#include <string>
#include <cstring>

using namespace atlas;

// ── Phase tests ─────────────────────────────────────────────────────

void test_gsm_default_phase() {
    GameStateManager gsm;
    assert(gsm.Phase() == GamePhase::MainMenu);
}

void test_gsm_set_phase() {
    GameStateManager gsm;
    gsm.SetPhase(GamePhase::InSpace);
    assert(gsm.Phase() == GamePhase::InSpace);
}

void test_gsm_phase_callback_fires() {
    GameStateManager gsm;
    GamePhase oldP = GamePhase::MainMenu, newP = GamePhase::MainMenu;
    int callCount = 0;
    gsm.OnPhaseChange([&](GamePhase o, GamePhase n) {
        oldP = o;
        newP = n;
        ++callCount;
    });

    gsm.SetPhase(GamePhase::Loading);
    assert(callCount == 1);
    assert(oldP == GamePhase::MainMenu);
    assert(newP == GamePhase::Loading);
}

void test_gsm_phase_same_no_callback() {
    GameStateManager gsm;
    int callCount = 0;
    gsm.OnPhaseChange([&](GamePhase, GamePhase) { ++callCount; });

    gsm.SetPhase(GamePhase::MainMenu);  // Same as default — should not fire.
    assert(callCount == 0);
}

void test_gsm_phase_names() {
    assert(std::strcmp(GamePhaseName(GamePhase::MainMenu), "MainMenu") == 0);
    assert(std::strcmp(GamePhaseName(GamePhase::Loading),  "Loading") == 0);
    assert(std::strcmp(GamePhaseName(GamePhase::InSpace),  "InSpace") == 0);
    assert(std::strcmp(GamePhaseName(GamePhase::Docked),   "Docked") == 0);
    assert(std::strcmp(GamePhaseName(GamePhase::InWarp),   "InWarp") == 0);
    assert(std::strcmp(GamePhaseName(GamePhase::Paused),   "Paused") == 0);
}

// ── Player / system / station ───────────────────────────────────────

void test_gsm_player_entity() {
    GameStateManager gsm;
    assert(gsm.PlayerEntity() == 0);
    gsm.SetPlayerEntity(42);
    assert(gsm.PlayerEntity() == 42);
}

void test_gsm_current_system() {
    GameStateManager gsm;
    assert(gsm.CurrentSystem() == 0);
    gsm.SetCurrentSystem(7);
    assert(gsm.CurrentSystem() == 7);
}

void test_gsm_docked_station() {
    GameStateManager gsm;
    assert(gsm.DockedStation() == 0);
    gsm.SetDockedStation(10);
    assert(gsm.DockedStation() == 10);
}

// ── Credits ─────────────────────────────────────────────────────────

void test_gsm_credits_default_zero() {
    GameStateManager gsm;
    assert(gsm.Credits() == 0);
}

void test_gsm_credits_add() {
    GameStateManager gsm;
    gsm.AddCredits(1000);
    assert(gsm.Credits() == 1000);
    gsm.AddCredits(500);
    assert(gsm.Credits() == 1500);
}

void test_gsm_credits_spend_success() {
    GameStateManager gsm;
    gsm.SetCredits(500);
    assert(gsm.Spend(200));
    assert(gsm.Credits() == 300);
}

void test_gsm_credits_spend_fail() {
    GameStateManager gsm;
    gsm.SetCredits(100);
    assert(!gsm.Spend(200));
    assert(gsm.Credits() == 100);  // Unchanged.
}

void test_gsm_credits_can_afford() {
    GameStateManager gsm;
    gsm.SetCredits(100);
    assert(gsm.CanAfford(100));
    assert(gsm.CanAfford(50));
    assert(!gsm.CanAfford(101));
}

// ── Inventory ───────────────────────────────────────────────────────

void test_gsm_inventory_empty() {
    GameStateManager gsm;
    assert(gsm.InventorySize() == 0);
    assert(!gsm.HasItem(1));
}

void test_gsm_inventory_add_and_has() {
    GameStateManager gsm;
    gsm.AddItem(10);
    gsm.AddItem(20);
    assert(gsm.InventorySize() == 2);
    assert(gsm.HasItem(10));
    assert(gsm.HasItem(20));
    assert(!gsm.HasItem(30));
}

void test_gsm_inventory_remove() {
    GameStateManager gsm;
    gsm.AddItem(5);
    gsm.AddItem(10);
    assert(gsm.RemoveItem(5));
    assert(gsm.InventorySize() == 1);
    assert(!gsm.HasItem(5));
    assert(gsm.HasItem(10));
}

void test_gsm_inventory_remove_nonexistent() {
    GameStateManager gsm;
    assert(!gsm.RemoveItem(999));
}

// ── Reset ───────────────────────────────────────────────────────────

void test_gsm_reset() {
    GameStateManager gsm;
    gsm.SetPhase(GamePhase::InSpace);
    gsm.SetPlayerEntity(42);
    gsm.SetCurrentSystem(7);
    gsm.SetDockedStation(3);
    gsm.SetCredits(5000);
    gsm.AddItem(1);

    gsm.Reset();

    assert(gsm.Phase() == GamePhase::MainMenu);
    assert(gsm.PlayerEntity() == 0);
    assert(gsm.CurrentSystem() == 0);
    assert(gsm.DockedStation() == 0);
    assert(gsm.Credits() == 0);
    assert(gsm.InventorySize() == 0);
}

void test_gsm_reset_preserves_callbacks() {
    GameStateManager gsm;
    int callCount = 0;
    gsm.OnPhaseChange([&](GamePhase, GamePhase) { ++callCount; });

    gsm.SetPhase(GamePhase::InSpace);
    assert(callCount == 1);

    gsm.Reset();
    // After reset, phase is MainMenu. SetPhase to Loading should fire callback.
    gsm.SetPhase(GamePhase::Loading);
    assert(callCount == 2);
}
