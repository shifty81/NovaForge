// Tests for: Convoy Ambush System tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/convoy_ambush_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Convoy Ambush System tests ====================

static void testConvoyRegisterRoute() {
    std::cout << "\n=== Convoy Register Route ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    std::string routeId = convoysSys.registerRoute("Jita", "Amarr", "Tritanium",
                                                    500000000.0, 0.9f);
    assertTrue(!routeId.empty(), "Route registered successfully");
    assertTrue(convoysSys.getRouteRisk(routeId) > 0.0f, "Route has pirate interest");
}

static void testConvoyPlanAmbush() {
    std::cout << "\n=== Convoy Plan Ambush ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    std::string routeId = convoysSys.registerRoute("Jita", "Amarr", "Tritanium",
                                                    500000000.0, 0.5f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    assertTrue(!ambushId.empty(), "Ambush planned");
    assertTrue(convoysSys.getAmbushState(ambushId) == "planned", "Ambush state is planned");
    assertTrue(convoysSys.getPlannedAmbushes().size() == 1, "One planned ambush");
}

static void testConvoyExecuteAmbushLowSec() {
    std::cout << "\n=== Convoy Execute Ambush Low-Sec ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    // Low-sec route (security < 0.7) — ambush should succeed
    std::string routeId = convoysSys.registerRoute("Amamake", "Rancer", "Plex",
                                                    1000000000.0, 0.3f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    bool success = convoysSys.executeAmbush(ambushId);
    assertTrue(success, "Ambush succeeded on low-sec route");
    assertTrue(convoysSys.getAmbushState(ambushId) == "active", "Ambush state is active");
}

static void testConvoyExecuteAmbushHighSec() {
    std::cout << "\n=== Convoy Execute Ambush High-Sec ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    // High-sec route (security >= 0.7) — ambush should fail
    std::string routeId = convoysSys.registerRoute("Jita", "Amarr", "Tritanium",
                                                    500000000.0, 0.9f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    bool success = convoysSys.executeAmbush(ambushId);
    assertTrue(!success, "Ambush failed on high-sec route");
    assertTrue(convoysSys.getAmbushState(ambushId) == "failed", "Ambush state is failed");
}

static void testConvoyDisperseAmbush() {
    std::cout << "\n=== Convoy Disperse Ambush ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    std::string routeId = convoysSys.registerRoute("Amamake", "Rancer", "Plex",
                                                    1000000000.0, 0.2f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    convoysSys.executeAmbush(ambushId);
    assertTrue(convoysSys.disperseAmbush(ambushId), "Ambush dispersed");
    assertTrue(convoysSys.getAmbushState(ambushId) == "dispersed", "State is dispersed");
}

static void testConvoyAmbushLootValue() {
    std::cout << "\n=== Convoy Ambush Loot Value ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    // Security 0.0 (fully lawless) means 50% of cargo captured
    std::string routeId = convoysSys.registerRoute("Null-A", "Null-B", "Platinum",
                                                    1000000000.0, 0.0f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    convoysSys.executeAmbush(ambushId);
    auto* ambushEnt = world.getEntity(ambushId);
    auto* ambush = ambushEnt->getComponent<components::ConvoyAmbush>();
    assertTrue(ambush->loot_value > 0.0, "Loot value captured");
    assertTrue(ambush->ships_attacked == 1, "One ship attacked");
}

static void testConvoyAmbushCannotExecuteTwice() {
    std::cout << "\n=== Convoy Ambush Cannot Execute Twice ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    std::string routeId = convoysSys.registerRoute("Amamake", "Rancer", "Plex",
                                                    1000000000.0, 0.2f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    convoysSys.executeAmbush(ambushId);
    assertTrue(!convoysSys.executeAmbush(ambushId), "Cannot execute active ambush again");
}


void run_convoy_ambush_system_tests() {
    testConvoyRegisterRoute();
    testConvoyPlanAmbush();
    testConvoyExecuteAmbushLowSec();
    testConvoyExecuteAmbushHighSec();
    testConvoyDisperseAmbush();
    testConvoyAmbushLootValue();
    testConvoyAmbushCannotExecuteTwice();
}
