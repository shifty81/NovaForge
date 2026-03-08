// Tests for: Resource Respawn System
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/resource_respawn_system.h"

using namespace atlas;

// ==================== Resource Respawn System Tests ====================

static void testResourceRespawnCreate() {
    std::cout << "\n=== ResourceRespawn: Create ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    assertTrue(sys.initialize("zone1", "asteroid_belt_1"), "Init succeeds");
    assertTrue(sys.getZoneId("zone1") == "asteroid_belt_1", "Zone ID matches");
    assertTrue(sys.getResourceCount("zone1") == 0, "0 resources initially");
    assertTrue(sys.getTotalRespawns("zone1") == 0, "0 respawns initially");
    assertTrue(sys.getTotalDepletions("zone1") == 0, "0 depletions initially");
}

static void testResourceRespawnInitValidation() {
    std::cout << "\n=== ResourceRespawn: InitValidation ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    assertTrue(!sys.initialize("zone1", ""), "Empty zone_id rejected");
    assertTrue(!sys.initialize("nonexistent", "zone_a"), "Missing entity rejected");
}

static void testResourceRespawnAddResource() {
    std::cout << "\n=== ResourceRespawn: AddResource ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    assertTrue(sys.addResource("zone1", "rock_1", "asteroid", 60.0f), "Add asteroid");
    assertTrue(sys.addResource("zone1", "site_1", "site", 120.0f), "Add site");
    assertTrue(sys.addResource("zone1", "npc_1", "npc_spawn", 30.0f), "Add npc spawn");
    assertTrue(sys.getResourceCount("zone1") == 3, "3 resources");
}

static void testResourceRespawnAddValidation() {
    std::cout << "\n=== ResourceRespawn: AddValidation ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    assertTrue(!sys.addResource("zone1", "", "asteroid", 60.0f), "Empty resource_id rejected");
    assertTrue(!sys.addResource("zone1", "r1", "", 60.0f), "Empty type rejected");
    assertTrue(!sys.addResource("zone1", "r1", "asteroid", 0.0f), "Zero cooldown rejected");
    assertTrue(!sys.addResource("zone1", "r1", "asteroid", -5.0f), "Negative cooldown rejected");
    assertTrue(!sys.addResource("nonexistent", "r1", "asteroid", 60.0f), "Missing entity rejected");
}

static void testResourceRespawnDuplicate() {
    std::cout << "\n=== ResourceRespawn: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    assertTrue(sys.addResource("zone1", "rock_1", "asteroid", 60.0f), "Add first");
    assertTrue(!sys.addResource("zone1", "rock_1", "site", 30.0f), "Duplicate rejected");
}

static void testResourceRespawnRemove() {
    std::cout << "\n=== ResourceRespawn: Remove ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    sys.addResource("zone1", "rock_1", "asteroid", 60.0f);
    sys.addResource("zone1", "rock_2", "asteroid", 60.0f);
    assertTrue(sys.removeResource("zone1", "rock_1"), "Remove rock_1");
    assertTrue(sys.getResourceCount("zone1") == 1, "1 resource left");
    assertTrue(!sys.removeResource("zone1", "nonexistent"), "Cannot remove nonexistent");
}

static void testResourceRespawnDeplete() {
    std::cout << "\n=== ResourceRespawn: Deplete ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    sys.addResource("zone1", "rock_1", "asteroid", 60.0f);
    assertTrue(sys.depleteResource("zone1", "rock_1"), "Deplete succeeds");
    assertTrue(sys.isResourceDepleted("zone1", "rock_1"), "Resource is depleted");
    assertTrue(!sys.isResourceRespawned("zone1", "rock_1"), "Resource not yet respawned");
    assertTrue(sys.getDepletedCount("zone1") == 1, "1 depleted");
    assertTrue(sys.getTotalDepletions("zone1") == 1, "1 total depletion");
    assertTrue(approxEqual(sys.getCooldownRemaining("zone1", "rock_1"), 60.0f), "60s cooldown");
    // Cannot deplete already-depleted
    assertTrue(!sys.depleteResource("zone1", "rock_1"), "Double deplete rejected");
}

static void testResourceRespawnCooldownTick() {
    std::cout << "\n=== ResourceRespawn: CooldownTick ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    sys.addResource("zone1", "rock_1", "asteroid", 10.0f);
    sys.depleteResource("zone1", "rock_1");

    // Tick 5 seconds — still depleted
    sys.update(5.0f);
    assertTrue(sys.isResourceDepleted("zone1", "rock_1"), "Still depleted at 5s");
    assertTrue(approxEqual(sys.getCooldownRemaining("zone1", "rock_1"), 5.0f), "5s remaining");

    // Tick 6 more seconds — respawned
    sys.update(6.0f);
    assertTrue(!sys.isResourceDepleted("zone1", "rock_1"), "No longer depleted");
    assertTrue(sys.isResourceRespawned("zone1", "rock_1"), "Respawned");
    assertTrue(sys.getTotalRespawns("zone1") == 1, "1 total respawn");
    assertTrue(sys.getRespawnedCount("zone1") == 1, "1 respawned resource");
}

static void testResourceRespawnYieldMultiplier() {
    std::cout << "\n=== ResourceRespawn: YieldMultiplier ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    sys.addResource("zone1", "rock_1", "asteroid", 60.0f);
    assertTrue(sys.setYieldMultiplier("zone1", "rock_1", 1.5f), "Set yield 1.5x");
    assertTrue(!sys.setYieldMultiplier("zone1", "rock_1", 0.0f), "Zero multiplier rejected");
    assertTrue(!sys.setYieldMultiplier("zone1", "rock_1", -1.0f), "Negative multiplier rejected");
    assertTrue(!sys.setYieldMultiplier("zone1", "nonexistent", 1.0f), "Missing resource rejected");
    assertTrue(!sys.setYieldMultiplier("nonexistent", "rock_1", 1.0f), "Missing entity rejected");
}

static void testResourceRespawnMultipleResources() {
    std::cout << "\n=== ResourceRespawn: MultipleResources ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    world.createEntity("zone1");
    sys.initialize("zone1", "belt_1");
    sys.addResource("zone1", "rock_1", "asteroid", 5.0f);
    sys.addResource("zone1", "rock_2", "asteroid", 10.0f);
    sys.addResource("zone1", "site_1", "site", 20.0f);

    sys.depleteResource("zone1", "rock_1");
    sys.depleteResource("zone1", "rock_2");
    sys.depleteResource("zone1", "site_1");
    assertTrue(sys.getDepletedCount("zone1") == 3, "3 depleted");

    // After 6s, only rock_1 (5s cooldown) should respawn
    sys.update(6.0f);
    assertTrue(sys.isResourceRespawned("zone1", "rock_1"), "rock_1 respawned");
    assertTrue(sys.isResourceDepleted("zone1", "rock_2"), "rock_2 still depleted");
    assertTrue(sys.isResourceDepleted("zone1", "site_1"), "site_1 still depleted");
    assertTrue(sys.getTotalRespawns("zone1") == 1, "1 respawn so far");

    // After 5 more seconds (11s total), rock_2 respawns too
    sys.update(5.0f);
    assertTrue(sys.isResourceRespawned("zone1", "rock_2"), "rock_2 respawned");
    assertTrue(sys.isResourceDepleted("zone1", "site_1"), "site_1 still depleted");
    assertTrue(sys.getTotalRespawns("zone1") == 2, "2 respawns");
}

static void testResourceRespawnMissing() {
    std::cout << "\n=== ResourceRespawn: Missing ===" << std::endl;
    ecs::World world;
    systems::ResourceRespawnSystem sys(&world);
    assertTrue(sys.getResourceCount("x") == 0, "0 resources on missing");
    assertTrue(sys.getDepletedCount("x") == 0, "0 depleted on missing");
    assertTrue(sys.getRespawnedCount("x") == 0, "0 respawned on missing");
    assertTrue(sys.getTotalRespawns("x") == 0, "0 total respawns on missing");
    assertTrue(sys.getTotalDepletions("x") == 0, "0 depletions on missing");
    assertTrue(approxEqual(sys.getCooldownRemaining("x", "r"), 0.0f), "0 cooldown on missing");
    assertTrue(!sys.isResourceDepleted("x", "r"), "Not depleted on missing");
    assertTrue(!sys.isResourceRespawned("x", "r"), "Not respawned on missing");
    assertTrue(sys.getZoneId("x").empty(), "Empty zone on missing");
    assertTrue(!sys.depleteResource("x", "r"), "Deplete fails on missing");
    assertTrue(!sys.removeResource("x", "r"), "Remove fails on missing");
}

void run_resource_respawn_system_tests() {
    testResourceRespawnCreate();
    testResourceRespawnInitValidation();
    testResourceRespawnAddResource();
    testResourceRespawnAddValidation();
    testResourceRespawnDuplicate();
    testResourceRespawnRemove();
    testResourceRespawnDeplete();
    testResourceRespawnCooldownTick();
    testResourceRespawnYieldMultiplier();
    testResourceRespawnMultipleResources();
    testResourceRespawnMissing();
}
