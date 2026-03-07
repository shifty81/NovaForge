// Tests for: NPC Fleet Composition System
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/npc_fleet_composition_system.h"

using namespace atlas;

// ==================== NPC Fleet Composition System Tests ====================

static void testNpcFleetCompositionCreate() {
    std::cout << "\n=== NpcFleetComposition: Create ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initialize("fleet1", "template_01", "Pirate Patrol"), "Init succeeds");
    assertTrue(sys.getShipCount("fleet1") == 0, "No ships");
    assertTrue(approxEqual(sys.getTotalThreat("fleet1"), 0.0f), "0 threat");
    assertTrue(sys.getDifficulty("fleet1") == "Easy", "Default Easy");
    assertTrue(sys.getFleetsSpawned("fleet1") == 0, "0 spawned");
    assertTrue(sys.getFleetsDestroyed("fleet1") == 0, "0 destroyed");
    assertTrue(sys.isReady("fleet1"), "Ready to spawn");
}

static void testNpcFleetCompositionAddShips() {
    std::cout << "\n=== NpcFleetComposition: AddShips ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "template_01", "Pirate Patrol");

    assertTrue(sys.addShip("fleet1", "frigate", "DPS", 1.0f, false), "Add DPS frigate");
    assertTrue(sys.addShip("fleet1", "cruiser", "Tank", 2.0f, false), "Add Tank cruiser");
    assertTrue(sys.addShip("fleet1", "battlecruiser", "Commander", 3.0f, true), "Add Commander");
    assertTrue(sys.getShipCount("fleet1") == 3, "3 ships");

    sys.update(0.1f);
    // Easy difficulty = 1.0x, total base = 1+2+3 = 6
    assertTrue(approxEqual(sys.getTotalThreat("fleet1"), 6.0f), "Total threat 6.0");
}

static void testNpcFleetCompositionDifficultyScaling() {
    std::cout << "\n=== NpcFleetComposition: DifficultyScaling ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "template_01", "Elite Squad");

    sys.addShip("fleet1", "frigate", "DPS", 2.0f, false);
    sys.addShip("fleet1", "cruiser", "DPS", 3.0f, false);

    // Easy: 5.0 * 1.0 = 5.0
    sys.update(0.1f);
    assertTrue(approxEqual(sys.getTotalThreat("fleet1"), 5.0f), "Easy threat = 5.0");

    // Medium: 5.0 * 1.5 = 7.5
    sys.setDifficulty("fleet1", "Medium");
    assertTrue(approxEqual(sys.getTotalThreat("fleet1"), 7.5f), "Medium threat = 7.5");

    // Hard: 5.0 * 2.0 = 10.0
    sys.setDifficulty("fleet1", "Hard");
    assertTrue(approxEqual(sys.getTotalThreat("fleet1"), 10.0f), "Hard threat = 10.0");

    // Elite: 5.0 * 3.0 = 15.0
    sys.setDifficulty("fleet1", "Elite");
    assertTrue(approxEqual(sys.getTotalThreat("fleet1"), 15.0f), "Elite threat = 15.0");
}

static void testNpcFleetCompositionSpawnCooldown() {
    std::cout << "\n=== NpcFleetComposition: SpawnCooldown ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "template_01", "Patrol");
    sys.setSpawnCooldown("fleet1", 60.0f);

    assertTrue(sys.isReady("fleet1"), "Ready initially");
    assertTrue(sys.requestSpawn("fleet1"), "Spawn succeeds");
    assertTrue(sys.getFleetsSpawned("fleet1") == 1, "1 spawned");
    assertTrue(!sys.isReady("fleet1"), "Not ready during cooldown");
    assertTrue(!sys.requestSpawn("fleet1"), "Cannot spawn during cooldown");

    // Partial cooldown
    sys.update(30.0f);
    assertTrue(!sys.isReady("fleet1"), "Still on cooldown at 30s");
    assertTrue(sys.getCooldownRemaining("fleet1") > 0.0f, "Cooldown remaining");

    // Full cooldown
    sys.update(35.0f);
    assertTrue(sys.isReady("fleet1"), "Ready after cooldown");
    assertTrue(sys.requestSpawn("fleet1"), "Second spawn succeeds");
    assertTrue(sys.getFleetsSpawned("fleet1") == 2, "2 spawned");
}

static void testNpcFleetCompositionMaxShips() {
    std::cout << "\n=== NpcFleetComposition: MaxShips ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "template_01", "Patrol");

    // Default max is 5
    for (int i = 0; i < 5; i++) {
        assertTrue(sys.addShip("fleet1", "frigate", "DPS", 1.0f, false),
                  "Add ship " + std::to_string(i));
    }
    assertTrue(!sys.addShip("fleet1", "frigate", "DPS", 1.0f, false), "6th ship rejected");
    assertTrue(sys.getShipCount("fleet1") == 5, "Max 5 ships");
}

static void testNpcFleetCompositionDestroyed() {
    std::cout << "\n=== NpcFleetComposition: Destroyed ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "template_01", "Patrol");

    sys.requestSpawn("fleet1");
    assertTrue(sys.recordDestroyed("fleet1"), "Record destroy");
    assertTrue(sys.getFleetsDestroyed("fleet1") == 1, "1 destroyed");

    sys.recordDestroyed("fleet1");
    assertTrue(sys.getFleetsDestroyed("fleet1") == 2, "2 destroyed");
}

static void testNpcFleetCompositionMissing() {
    std::cout << "\n=== NpcFleetComposition: Missing ===" << std::endl;
    ecs::World world;
    systems::NpcFleetCompositionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "t", "n"), "Init fails on missing");
    assertTrue(!sys.addShip("nonexistent", "f", "DPS", 1.0f, false), "AddShip fails on missing");
    assertTrue(!sys.setSecurityLevel("nonexistent", 0.5f), "SetSecurity fails on missing");
    assertTrue(!sys.setDifficulty("nonexistent", "Hard"), "SetDifficulty fails on missing");
    assertTrue(!sys.setSpawnCooldown("nonexistent", 60.0f), "SetCooldown fails on missing");
    assertTrue(!sys.requestSpawn("nonexistent"), "Spawn fails on missing");
    assertTrue(!sys.recordDestroyed("nonexistent"), "Destroyed fails on missing");
    assertTrue(sys.getShipCount("nonexistent") == 0, "0 ships on missing");
    assertTrue(approxEqual(sys.getTotalThreat("nonexistent"), 0.0f), "0 threat on missing");
    assertTrue(sys.getDifficulty("nonexistent") == "Unknown", "Unknown difficulty on missing");
    assertTrue(sys.getFleetsSpawned("nonexistent") == 0, "0 spawned on missing");
    assertTrue(sys.getFleetsDestroyed("nonexistent") == 0, "0 destroyed on missing");
    assertTrue(approxEqual(sys.getCooldownRemaining("nonexistent"), 0.0f), "0 cooldown on missing");
    assertTrue(!sys.isReady("nonexistent"), "Not ready on missing");
}

void run_npc_fleet_composition_system_tests() {
    testNpcFleetCompositionCreate();
    testNpcFleetCompositionAddShips();
    testNpcFleetCompositionDifficultyScaling();
    testNpcFleetCompositionSpawnCooldown();
    testNpcFleetCompositionMaxShips();
    testNpcFleetCompositionDestroyed();
    testNpcFleetCompositionMissing();
}
