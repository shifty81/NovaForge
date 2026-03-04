// Tests for: Myth Boss System Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/myth_boss_system.h"
#include "systems/propaganda_system.h"

using namespace atlas;

// ==================== Myth Boss System Tests ====================

static void testMythBossDefaults() {
    std::cout << "\n=== Myth Boss Defaults ===" << std::endl;
    ecs::World world;
    systems::MythBossSystem bossSys(&world);
    assertTrue(bossSys.getActiveBossCount() == 0, "No active bosses initially");
}

static void testMythBossGenerateEncounter() {
    std::cout << "\n=== Myth Boss Generate Encounter ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("ancient_pilot", "Solari", "heroic", "Battle of the Ancients");
    std::string encId = bossSys.generateEncounter(mythId, "system_alpha");
    assertTrue(!encId.empty(), "Encounter ID returned");
    assertTrue(bossSys.isEncounterActive(encId), "Encounter is active");
    assertTrue(bossSys.getActiveBossCount() == 1, "One active boss");
}

static void testMythBossDifficulty() {
    std::cout << "\n=== Myth Boss Difficulty ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("legend", "Veyren", "villainous");
    // Spread the myth to increase difficulty
    propSys.spreadMyth(mythId, "sys1");
    propSys.spreadMyth(mythId, "sys2");
    propSys.spreadMyth(mythId, "sys3");
    std::string encId = bossSys.generateEncounter(mythId, "system_beta");
    float diff = bossSys.getBossDifficulty(encId);
    assertTrue(diff > 1.0f, "Difficulty scales with myth spread");
}

static void testMythBossComplete() {
    std::cout << "\n=== Myth Boss Complete ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("hero", "Aurelian", "mysterious");
    std::string encId = bossSys.generateEncounter(mythId, "system_gamma");
    assertTrue(bossSys.completeEncounter(encId, true), "Complete succeeds");
    assertTrue(!bossSys.isEncounterActive(encId), "No longer active after completion");
    assertTrue(bossSys.getActiveBossCount() == 0, "Zero active bosses after completion");
}

static void testMythBossExpiry() {
    std::cout << "\n=== Myth Boss Expiry ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("fading_one", "Keldari", "exaggerated");
    std::string encId = bossSys.generateEncounter(mythId, "system_delta");
    assertTrue(bossSys.isEncounterActive(encId), "Active before expiry");

    // Simulate time past max duration
    bossSys.update(3601.0f);
    assertTrue(!bossSys.isEncounterActive(encId), "Expired after max duration");
}

static void testMythBossTypeName() {
    std::cout << "\n=== Myth Boss Type Name ===" << std::endl;
    assertTrue(systems::MythBossSystem::getBossTypeName(0) == "Guardian", "Type 0 is Guardian");
    assertTrue(systems::MythBossSystem::getBossTypeName(1) == "Destroyer", "Type 1 is Destroyer");
    assertTrue(systems::MythBossSystem::getBossTypeName(2) == "Phantom", "Type 2 is Phantom");
    assertTrue(systems::MythBossSystem::getBossTypeName(3) == "Colossus", "Type 3 is Colossus");
    assertTrue(systems::MythBossSystem::getBossTypeName(4) == "Mirage", "Type 4 is Mirage");
    assertTrue(systems::MythBossSystem::getBossTypeName(99) == "Unknown", "Unknown type returns Unknown");
}

static void testMythBossSourceMyth() {
    std::cout << "\n=== Myth Boss Source Myth ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("tracker", "Solari", "fabricated");
    std::string encId = bossSys.generateEncounter(mythId, "system_epsilon");
    assertTrue(bossSys.getEncounterMythId(encId) == mythId, "Source myth ID matches");
}


void run_myth_boss_system_tests() {
    testMythBossDefaults();
    testMythBossGenerateEncounter();
    testMythBossDifficulty();
    testMythBossComplete();
    testMythBossExpiry();
    testMythBossTypeName();
    testMythBossSourceMyth();
}
