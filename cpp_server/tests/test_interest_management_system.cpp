// Tests for: InterestManagementSystem Tests, InterestManagementPriority Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/interest_management_priority_system.h"
#include "systems/interest_management_system.h"

using namespace atlas;

// ==================== InterestManagementSystem Tests ====================

static void testInterestRegisterClient() {
    std::cout << "\n=== Interest: Register Client ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);

    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;

    ims.registerClient(1, "player_1");
    assertTrue(ims.getClientCount() == 1, "One client registered");
}

static void testInterestNearEntityIncluded() {
    std::cout << "\n=== Interest: Near Entity Included ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("npc_1");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 50.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "npc_1"), "Near entity is relevant");
    assertTrue(ims.isRelevant(1, "player_1"), "Own entity is always relevant");
}

static void testInterestFarEntityExcluded() {
    std::cout << "\n=== Interest: Far Entity Excluded ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("npc_far");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 200.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.update(0.0f);

    assertTrue(!ims.isRelevant(1, "npc_far"), "Far entity excluded");
}

static void testInterestForceVisible() {
    std::cout << "\n=== Interest: Force Visible ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("fleet_member");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 500.0f; npos->y = 0.0f; npos->z = 0.0f;  // beyond range

    ims.registerClient(1, "player_1");
    ims.addForceVisible(1, "fleet_member");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "fleet_member"),
               "Force-visible entity included despite distance");
}

static void testInterestForceVisibleRemove() {
    std::cout << "\n=== Interest: Remove Force Visible ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("fleet_member");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 500.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.addForceVisible(1, "fleet_member");
    ims.update(0.0f);
    assertTrue(ims.isRelevant(1, "fleet_member"), "Force visible before remove");

    ims.removeForceVisible(1, "fleet_member");
    ims.update(0.0f);
    assertTrue(!ims.isRelevant(1, "fleet_member"), "Not relevant after removing force visible");
}

static void testInterestUnregisterClient() {
    std::cout << "\n=== Interest: Unregister Client ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);

    ims.registerClient(1, "player_1");
    assertTrue(ims.getClientCount() == 1, "Client registered");

    ims.unregisterClient(1);
    assertTrue(ims.getClientCount() == 0, "Client unregistered");
    assertTrue(ims.getRelevantCount(1) == 0, "No relevant entities for unregistered client");
}

static void testInterestMultipleClients() {
    std::cout << "\n=== Interest: Multiple Clients ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* p1 = world.createEntity("player_1");
    auto* p1pos = addComp<components::Position>(p1);
    p1pos->x = 0.0f; p1pos->y = 0.0f; p1pos->z = 0.0f;

    auto* p2 = world.createEntity("player_2");
    auto* p2pos = addComp<components::Position>(p2);
    p2pos->x = 200.0f; p2pos->y = 0.0f; p2pos->z = 0.0f;

    auto* npc = world.createEntity("npc_1");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 50.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.registerClient(2, "player_2");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "npc_1"), "NPC near player 1 is relevant to client 1");
    assertTrue(!ims.isRelevant(2, "npc_1"), "NPC far from player 2 is not relevant to client 2");
}

static void testInterestEntityNoPosition() {
    std::cout << "\n=== Interest: Entity Without Position Included ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);

    // System entity without position
    world.createEntity("system_entity");

    ims.registerClient(1, "player_1");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "system_entity"),
               "Entity without position is always relevant");
}


// ==================== InterestManagementPriority Tests ====================

static void testInterestPriorityCreate() {
    std::cout << "\n=== InterestManagementPriority: Create ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    assertTrue(sys.createPriority("ip1"), "Create priority succeeds");
    auto* ip = e->getComponent<components::InterestPriority>();
    assertTrue(ip != nullptr, "Component exists");
    assertTrue(ip->active, "Active by default");
    assertTrue(ip->priority_tier == 2, "Default tier is Medium (2)");
    assertTrue(ip->needs_update, "Needs update by default");
    assertTrue(ip->total_updates == 0, "No updates yet");
}

static void testInterestPrioritySetTier() {
    std::cout << "\n=== InterestManagementPriority: SetTier ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    world.createEntity("ip1");
    sys.createPriority("ip1");
    assertTrue(sys.setPriorityTier("ip1", 0), "Set Critical tier");
    assertTrue(sys.getPriorityTier("ip1") == 0, "Tier is Critical (0)");
    assertTrue(sys.getBandwidthWeight("ip1") == 1.0f, "Critical bandwidth weight is 1.0");
    assertTrue(sys.setPriorityTier("ip1", 3), "Set Low tier");
    assertTrue(sys.getBandwidthWeight("ip1") == 0.25f, "Low bandwidth weight is 0.25");
}

static void testInterestPriorityDistanceAuto() {
    std::cout << "\n=== InterestManagementPriority: DistanceAuto ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    world.createEntity("ip1");
    sys.createPriority("ip1");
    assertTrue(sys.setDistance("ip1", 500.0f), "Set distance 500");
    assertTrue(sys.getPriorityTier("ip1") == 0, "Distance 500 -> Critical");
    assertTrue(sys.setDistance("ip1", 3000.0f), "Set distance 3000");
    assertTrue(sys.getPriorityTier("ip1") == 1, "Distance 3000 -> High");
    assertTrue(sys.setDistance("ip1", 10000.0f), "Set distance 10000");
    assertTrue(sys.getPriorityTier("ip1") == 2, "Distance 10000 -> Medium");
    assertTrue(sys.setDistance("ip1", 25000.0f), "Set distance 25000");
    assertTrue(sys.getPriorityTier("ip1") == 3, "Distance 25000 -> Low");
}

static void testInterestPriorityUpdate() {
    std::cout << "\n=== InterestManagementPriority: Update ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 2); // Medium: 0.25s interval
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.3f);
    assertTrue(sys.needsUpdate("ip1"), "Update triggered after 0.3s >= 0.25s");
    assertTrue(sys.getTotalUpdates("ip1") == 1, "Total updates is 1");
}

static void testInterestPriorityAccumulator() {
    std::cout << "\n=== InterestManagementPriority: Accumulator ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 2); // Medium: 0.25s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.1f);
    assertTrue(!sys.needsUpdate("ip1"), "No update after 0.1s < 0.25s");
    sys.update(0.1f);
    assertTrue(!sys.needsUpdate("ip1"), "No update after 0.2s < 0.25s");
    sys.update(0.1f);
    assertTrue(sys.needsUpdate("ip1"), "Update after 0.3s >= 0.25s");
}

static void testInterestPriorityCritical() {
    std::cout << "\n=== InterestManagementPriority: Critical ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 0); // Critical: 0.05s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.05f);
    assertTrue(sys.needsUpdate("ip1"), "Critical updates every 0.05s");
    assertTrue(sys.getTotalUpdates("ip1") == 1, "One update counted");
}

static void testInterestPriorityLow() {
    std::cout << "\n=== InterestManagementPriority: Low ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 3); // Low: 0.5s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.4f);
    assertTrue(!sys.needsUpdate("ip1"), "No update at 0.4s < 0.5s");
    sys.update(0.2f);
    assertTrue(sys.needsUpdate("ip1"), "Update after 0.6s >= 0.5s");
}

static void testInterestPriorityBandwidth() {
    std::cout << "\n=== InterestManagementPriority: Bandwidth ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 0);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 100.0f, "Critical bandwidth = 100");
    sys.setPriorityTier("ip1", 1);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 75.0f, "High bandwidth = 75");
    sys.setPriorityTier("ip1", 2);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 50.0f, "Medium bandwidth = 50");
    sys.setPriorityTier("ip1", 3);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 25.0f, "Low bandwidth = 25");
}

static void testInterestPriorityInactive() {
    std::cout << "\n=== InterestManagementPriority: Inactive ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 0); // Critical: 0.05s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    ip->active = false;
    sys.update(1.0f);
    assertTrue(!sys.needsUpdate("ip1"), "Inactive entity skipped");
    assertTrue(sys.getTotalUpdates("ip1") == 0, "No updates on inactive");
}

static void testInterestPriorityMissing() {
    std::cout << "\n=== InterestManagementPriority: Missing ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    assertTrue(!sys.createPriority("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setClientId("nonexistent", 1), "SetClientId fails on missing");
    assertTrue(!sys.setPriorityTier("nonexistent", 0), "SetTier fails on missing");
    assertTrue(!sys.setDistance("nonexistent", 100.0f), "SetDistance fails on missing");
    assertTrue(!sys.needsUpdate("nonexistent"), "needsUpdate false on missing");
    assertTrue(sys.getPriorityTier("nonexistent") == -1, "-1 tier on missing");
    assertTrue(sys.getBandwidthWeight("nonexistent") == 0.0f, "0 bandwidth on missing");
    assertTrue(sys.getTotalUpdates("nonexistent") == 0, "0 updates on missing");
    assertTrue(sys.getEstimatedBandwidth("nonexistent") == 0.0f, "0 est bandwidth on missing");
}


void run_interest_management_system_tests() {
    testInterestRegisterClient();
    testInterestNearEntityIncluded();
    testInterestFarEntityExcluded();
    testInterestForceVisible();
    testInterestForceVisibleRemove();
    testInterestUnregisterClient();
    testInterestMultipleClients();
    testInterestEntityNoPosition();
    testInterestPriorityCreate();
    testInterestPrioritySetTier();
    testInterestPriorityDistanceAuto();
    testInterestPriorityUpdate();
    testInterestPriorityAccumulator();
    testInterestPriorityCritical();
    testInterestPriorityLow();
    testInterestPriorityBandwidth();
    testInterestPriorityInactive();
    testInterestPriorityMissing();
}
