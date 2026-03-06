// Tests for: Fleet Readiness System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/fleet_readiness_system.h"

using namespace atlas;

// ==================== Fleet Readiness System Tests ====================

static void testFleetReadinessCreate() {
    std::cout << "\n=== FleetReadiness: Create ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    assertTrue(sys.initialize("f1", "fleet_alpha"), "Init succeeds");
    assertTrue(sys.getMemberCount("f1") == 0, "No members");
    assertTrue(!sys.isFleetReady("f1"), "Empty fleet not ready");
    assertTrue(approxEqual(sys.getFleetDPS("f1"), 0.0f), "0 DPS");
    assertTrue(approxEqual(sys.getFleetEHP("f1"), 0.0f), "0 EHP");
    assertTrue(approxEqual(sys.getReadinessPercentage("f1"), 0.0f), "0% readiness");
}

static void testFleetReadinessAddMembers() {
    std::cout << "\n=== FleetReadiness: AddMembers ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    assertTrue(sys.addMember("f1", "m1", "Rifter", 150.0f, 5000.0f, 300.0f), "Add Rifter");
    assertTrue(sys.addMember("f1", "m2", "Thorax", 400.0f, 15000.0f, 800.0f), "Add Thorax");
    assertTrue(sys.getMemberCount("f1") == 2, "2 members");
    // Duplicate
    assertTrue(!sys.addMember("f1", "m1", "Rifter2", 100.0f, 4000.0f, 200.0f), "Duplicate rejected");
    assertTrue(sys.getMemberCount("f1") == 2, "Still 2");
}

static void testFleetReadinessMemberMax() {
    std::cout << "\n=== FleetReadiness: MemberMax ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    auto* entity = world.getEntity("f1");
    auto* state = entity->getComponent<components::FleetReadinessState>();
    state->max_members = 2;
    sys.addMember("f1", "m1", "Ship A", 100.0f, 5000.0f, 300.0f);
    sys.addMember("f1", "m2", "Ship B", 200.0f, 8000.0f, 500.0f);
    assertTrue(!sys.addMember("f1", "m3", "Ship C", 300.0f, 10000.0f, 700.0f), "Max members enforced");
    assertTrue(sys.getMemberCount("f1") == 2, "Still 2");
}

static void testFleetReadinessRemoveMember() {
    std::cout << "\n=== FleetReadiness: RemoveMember ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    sys.addMember("f1", "m1", "Rifter", 150.0f, 5000.0f, 300.0f);
    sys.addMember("f1", "m2", "Thorax", 400.0f, 15000.0f, 800.0f);
    assertTrue(sys.removeMember("f1", "m1"), "Remove Rifter");
    assertTrue(sys.getMemberCount("f1") == 1, "1 member");
    assertTrue(!sys.removeMember("f1", "unknown"), "Unknown removal fails");
}

static void testFleetReadinessMemberReady() {
    std::cout << "\n=== FleetReadiness: MemberReady ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    sys.addMember("f1", "m1", "Rifter", 150.0f, 5000.0f, 300.0f);
    sys.addMember("f1", "m2", "Thorax", 400.0f, 15000.0f, 800.0f);
    assertTrue(!sys.isMemberReady("f1", "m1"), "m1 not ready by default");
    assertTrue(sys.setMemberReady("f1", "m1", true), "Set m1 ready");
    assertTrue(sys.isMemberReady("f1", "m1"), "m1 now ready");
    assertTrue(!sys.isMemberReady("f1", "m2"), "m2 still not ready");
    assertTrue(sys.getReadyCount("f1") == 1, "1 ready");
    assertTrue(sys.getNotReadyCount("f1") == 1, "1 not ready");
    assertTrue(!sys.setMemberReady("f1", "unknown", true), "Unknown member fails");
}

static void testFleetReadinessAggregateStats() {
    std::cout << "\n=== FleetReadiness: AggregateStats ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    sys.addMember("f1", "m1", "Rifter", 150.0f, 5000.0f, 300.0f);
    sys.addMember("f1", "m2", "Thorax", 400.0f, 15000.0f, 800.0f);
    sys.addMember("f1", "m3", "Drake", 350.0f, 45000.0f, 1200.0f);
    assertTrue(approxEqual(sys.getFleetDPS("f1"), 900.0f), "900 DPS");
    assertTrue(approxEqual(sys.getFleetEHP("f1"), 65000.0f), "65000 EHP");
    assertTrue(approxEqual(sys.getFleetCapacitor("f1"), 2300.0f), "2300 capacitor");
}

static void testFleetReadinessFleetReady() {
    std::cout << "\n=== FleetReadiness: FleetReady ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    sys.addMember("f1", "m1", "Rifter", 150.0f, 5000.0f, 300.0f);
    sys.addMember("f1", "m2", "Thorax", 400.0f, 15000.0f, 800.0f);
    assertTrue(!sys.isFleetReady("f1"), "Not all ready");
    assertTrue(approxEqual(sys.getReadinessPercentage("f1"), 0.0f), "0%");
    sys.setMemberReady("f1", "m1", true);
    assertTrue(!sys.isFleetReady("f1"), "Still not all ready");
    assertTrue(approxEqual(sys.getReadinessPercentage("f1"), 50.0f), "50%");
    sys.setMemberReady("f1", "m2", true);
    assertTrue(sys.isFleetReady("f1"), "All ready");
    assertTrue(approxEqual(sys.getReadinessPercentage("f1"), 100.0f), "100%");
}

static void testFleetReadinessSupply() {
    std::cout << "\n=== FleetReadiness: Supply ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    assertTrue(sys.setSupplyLevel("f1", "ammo", 80.0f), "Set ammo supply");
    assertTrue(sys.setSupplyLevel("f1", "fuel", 60.0f), "Set fuel supply");
    assertTrue(approxEqual(sys.getSupplyLevel("f1", "ammo"), 80.0f), "80 ammo");
    assertTrue(approxEqual(sys.getSupplyLevel("f1", "fuel"), 60.0f), "60 fuel");
    // Update existing
    assertTrue(sys.setSupplyLevel("f1", "ammo", 90.0f), "Update ammo");
    assertTrue(approxEqual(sys.getSupplyLevel("f1", "ammo"), 90.0f), "90 ammo");
    // Unknown supply
    assertTrue(approxEqual(sys.getSupplyLevel("f1", "unknown"), 0.0f), "0 unknown");
    // Adequacy check
    assertTrue(sys.isSupplyAdequate("f1", 50.0f), "Adequate at 50 threshold");
    assertTrue(!sys.isSupplyAdequate("f1", 80.0f), "Not adequate at 80 (fuel=60)");
}

static void testFleetReadinessSupplyMax() {
    std::cout << "\n=== FleetReadiness: SupplyMax ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    auto* entity = world.getEntity("f1");
    auto* state = entity->getComponent<components::FleetReadinessState>();
    state->max_supplies = 2;
    sys.setSupplyLevel("f1", "ammo", 80.0f);
    sys.setSupplyLevel("f1", "fuel", 60.0f);
    assertTrue(!sys.setSupplyLevel("f1", "nanite", 50.0f), "Max supplies enforced");
    // But existing key can still update
    assertTrue(sys.setSupplyLevel("f1", "ammo", 95.0f), "Existing key updates");
    assertTrue(approxEqual(sys.getSupplyLevel("f1", "ammo"), 95.0f), "95 ammo");
}

static void testFleetReadinessUpdate() {
    std::cout << "\n=== FleetReadiness: Update ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet_alpha");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("f1");
    auto* state = entity->getComponent<components::FleetReadinessState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testFleetReadinessMissing() {
    std::cout << "\n=== FleetReadiness: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetReadinessSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.addMember("nonexistent", "m", "s", 0, 0, 0), "addMember fails");
    assertTrue(!sys.removeMember("nonexistent", "m"), "removeMember fails");
    assertTrue(sys.getMemberCount("nonexistent") == 0, "0 members");
    assertTrue(!sys.setMemberReady("nonexistent", "m", true), "setReady fails");
    assertTrue(!sys.isMemberReady("nonexistent", "m"), "not ready");
    assertTrue(sys.getReadyCount("nonexistent") == 0, "0 ready");
    assertTrue(sys.getNotReadyCount("nonexistent") == 0, "0 not ready");
    assertTrue(approxEqual(sys.getFleetDPS("nonexistent"), 0.0f), "0 DPS");
    assertTrue(approxEqual(sys.getFleetEHP("nonexistent"), 0.0f), "0 EHP");
    assertTrue(approxEqual(sys.getFleetCapacitor("nonexistent"), 0.0f), "0 cap");
    assertTrue(!sys.isFleetReady("nonexistent"), "Not ready");
    assertTrue(approxEqual(sys.getReadinessPercentage("nonexistent"), 0.0f), "0%");
    assertTrue(!sys.setSupplyLevel("nonexistent", "a", 0), "setSupply fails");
    assertTrue(approxEqual(sys.getSupplyLevel("nonexistent", "a"), 0.0f), "0 supply");
    assertTrue(!sys.isSupplyAdequate("nonexistent", 0), "Not adequate");
}

void run_fleet_readiness_system_tests() {
    testFleetReadinessCreate();
    testFleetReadinessAddMembers();
    testFleetReadinessMemberMax();
    testFleetReadinessRemoveMember();
    testFleetReadinessMemberReady();
    testFleetReadinessAggregateStats();
    testFleetReadinessFleetReady();
    testFleetReadinessSupply();
    testFleetReadinessSupplyMax();
    testFleetReadinessUpdate();
    testFleetReadinessMissing();
}
