// Tests for: NPC Rerouting System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/npc_rerouting_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== NPC Rerouting System Tests ====================

static void testNPCReroutingNoDanger() {
    std::cout << "\n=== NPC Rerouting No Danger ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("sys_safe");
    auto* sys_state = addComp<components::SimStarSystemState>(sys_entity);
    sys_state->threat_level = 0.2f;

    auto* npc = world.createEntity("npc1");
    auto* route = addComp<components::NPCRouteState>(npc);
    route->planned_route = {"sys_safe"};

    systems::NPCReroutingSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!route->rerouting, "No rerouting when threat is low");
    assertTrue(route->planned_route.size() == 1, "Route unchanged");
}

static void testNPCReroutingDangerousSystem() {
    std::cout << "\n=== NPC Rerouting Dangerous System ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("sys_danger");
    auto* sys_state = addComp<components::SimStarSystemState>(sys_entity);
    sys_state->threat_level = 0.9f;

    auto* npc = world.createEntity("npc2");
    auto* route = addComp<components::NPCRouteState>(npc);
    route->planned_route = {"sys_danger"};

    systems::NPCReroutingSystem sys(&world);
    sys.update(1.0f);
    assertTrue(route->rerouting, "Rerouting triggered by dangerous system");
    assertTrue(route->planned_route.empty(), "Dangerous system removed from route");
}

static void testNPCReroutingCooldown() {
    std::cout << "\n=== NPC Rerouting Cooldown ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("sys_mid");
    auto* sys_state = addComp<components::SimStarSystemState>(sys_entity);
    sys_state->threat_level = 0.9f;

    auto* npc = world.createEntity("npc3");
    auto* route = addComp<components::NPCRouteState>(npc);
    route->planned_route = {"sys_mid"};
    route->reroute_cooldown = 100.0f;

    systems::NPCReroutingSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!route->rerouting, "Cooldown prevents rerouting");
}


void run_npc_rerouting_system_tests() {
    testNPCReroutingNoDanger();
    testNPCReroutingDangerousSystem();
    testNPCReroutingCooldown();
}
