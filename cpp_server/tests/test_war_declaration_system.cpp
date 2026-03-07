// Tests for: War Declaration System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/war_declaration_system.h"
#include <sys/stat.h>

using namespace atlas;

// ========== War Declaration System Tests ==========

static void testWarDeclare() {
    std::cout << "\n=== War Declare ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    assertTrue(!warId.empty(), "War declared successfully");
    assertTrue(warSys.getWarStatus(warId) == "pending", "War status is pending");
    assertTrue(approxEqual(aggPlayer->credits, 400000000.0), "Credits deducted");
}

static void testWarActivate() {
    std::cout << "\n=== War Activate ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    assertTrue(warSys.activateWar(warId), "War activated");
    assertTrue(warSys.getWarStatus(warId) == "active", "War status is active");
    assertTrue(!warSys.activateWar(warId), "Cannot activate already-active war");
}

static void testWarMakeMutual() {
    std::cout << "\n=== War Make Mutual ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    assertTrue(!warSys.makeMutual(warId, "aggressor1"),
               "Aggressor cannot make mutual");
    assertTrue(warSys.makeMutual(warId, "defender1"),
               "Defender makes war mutual");
    assertTrue(warSys.getWarStatus(warId) == "mutual", "War status is mutual");
}

static void testWarSurrender() {
    std::cout << "\n=== War Surrender ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    assertTrue(!warSys.surrender(warId, "aggressor1"),
               "Aggressor cannot surrender");
    assertTrue(warSys.surrender(warId, "defender1"),
               "Defender surrenders");
    assertTrue(warSys.getWarStatus(warId) == "surrendered", "War status is surrendered");
}

static void testWarRetract() {
    std::cout << "\n=== War Retract ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);

    assertTrue(!warSys.retractWar(warId, "defender1"),
               "Defender cannot retract");
    assertTrue(warSys.retractWar(warId, "aggressor1"),
               "Aggressor retracts war");
    assertTrue(warSys.getWarStatus(warId) == "retracted", "War status is retracted");
}

static void testWarRecordKill() {
    std::cout << "\n=== War Record Kill ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    assertTrue(warSys.recordKill(warId, "aggressor", 50000000.0),
               "Aggressor kill recorded");
    assertTrue(warSys.recordKill(warId, "defender", 30000000.0),
               "Defender kill recorded");

    auto* war = world.getEntity(warId)->getComponent<components::WarDeclaration>();
    assertTrue(war->aggressor_kills == 1, "Aggressor has 1 kill");
    assertTrue(war->defender_kills == 1, "Defender has 1 kill");
    assertTrue(approxEqual(war->aggressor_isc_destroyed, 50000000.0),
               "Aggressor Credits destroyed correct");
    assertTrue(approxEqual(war->defender_isc_destroyed, 30000000.0),
               "Defender Credits destroyed correct");
}

static void testWarAutoFinish() {
    std::cout << "\n=== War Auto Finish ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    auto* war = world.getEntity(warId)->getComponent<components::WarDeclaration>();
    war->duration_hours = 1.0f; // 1 hour for testing

    // Simulate 2 hours (7200 seconds)
    warSys.update(7200.0f);

    assertTrue(warSys.getWarStatus(warId) == "finished", "War auto-finished after duration");
}

static void testWarInsufficientFunds() {
    std::cout << "\n=== War Insufficient Funds ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 50000000.0; // Only 50M, need 100M

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    assertTrue(warId.empty(), "War declaration failed - insufficient funds");
    assertTrue(approxEqual(aggPlayer->credits, 50000000.0), "Credits not deducted on failure");
}


void run_war_declaration_system_tests() {
    testWarDeclare();
    testWarActivate();
    testWarMakeMutual();
    testWarSurrender();
    testWarRetract();
    testWarRecordKill();
    testWarAutoFinish();
    testWarInsufficientFunds();
}
