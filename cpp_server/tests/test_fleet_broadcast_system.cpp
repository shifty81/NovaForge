// Tests for: FleetBroadcastSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_broadcast_system.h"

using namespace atlas;

// ==================== FleetBroadcastSystem Tests ====================

static void testFleetBroadcastInit() {
    std::cout << "\n=== FleetBroadcast: Init ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    assertTrue(sys.initialize("f1", "fleet_alpha"), "Init succeeds");
    assertTrue(sys.getActiveBroadcastCount("f1") == 0, "Zero broadcasts initially");
    assertTrue(sys.getTotalSent("f1") == 0, "Zero sent initially");
    assertTrue(sys.getTotalExpired("f1") == 0, "Zero expired initially");
    assertTrue(sys.getFleetId("f1") == "fleet_alpha", "Fleet ID set correctly");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetBroadcastSend() {
    std::cout << "\n=== FleetBroadcast: Send ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1", "fleet1");

    using BT = components::FleetBroadcastState::BroadcastType;
    assertTrue(sys.sendBroadcast("f1", "bc1", BT::Target, "pilot_a", "Rat Battleship", 30.0f),
               "Send Target broadcast");
    assertTrue(sys.sendBroadcast("f1", "bc2", BT::NeedShieldReps, "pilot_b", "Need reps!", 15.0f),
               "Send NeedShieldReps broadcast");
    assertTrue(sys.getActiveBroadcastCount("f1") == 2, "Two active broadcasts");
    assertTrue(sys.getTotalSent("f1") == 2, "Total sent is 2");
    assertTrue(sys.hasBroadcast("f1", "bc1"), "Has bc1");
    assertTrue(sys.hasBroadcast("f1", "bc2"), "Has bc2");
    assertTrue(!sys.hasBroadcast("f1", "bc3"), "No bc3");
}

static void testFleetBroadcastSendValidation() {
    std::cout << "\n=== FleetBroadcast: SendValidation ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    assertTrue(!sys.sendBroadcast("f1", "", BT::Target, "pilot", "Label", 10.0f),
               "Empty broadcast_id rejected");
    assertTrue(!sys.sendBroadcast("f1", "bc1", BT::Target, "", "Label", 10.0f),
               "Empty sender_id rejected");
    assertTrue(!sys.sendBroadcast("f1", "bc1", BT::Target, "pilot", "Label", 0.0f),
               "Zero TTL rejected");
    assertTrue(!sys.sendBroadcast("f1", "bc1", BT::Target, "pilot", "Label", -5.0f),
               "Negative TTL rejected");
    assertTrue(sys.sendBroadcast("f1", "bc1", BT::Target, "pilot", "", 10.0f),
               "Empty target_label allowed");
    assertTrue(!sys.sendBroadcast("f1", "bc1", BT::WarpTo, "pilot", "Gate", 10.0f),
               "Duplicate broadcast_id rejected");
    assertTrue(sys.getActiveBroadcastCount("f1") == 1, "Only 1 valid broadcast");
}

static void testFleetBroadcastMaxCap() {
    std::cout << "\n=== FleetBroadcast: MaxCap ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    for (int i = 0; i < 20; i++) {
        std::string id = "bc" + std::to_string(i);
        assertTrue(sys.sendBroadcast("f1", id, BT::Target, "pilot", "Target", 60.0f),
                   "Broadcast added within limit");
    }
    assertTrue(!sys.sendBroadcast("f1", "bc20", BT::Target, "pilot", "Over", 60.0f),
               "Blocked at max");
    assertTrue(sys.getActiveBroadcastCount("f1") == 20, "Count is 20");
}

static void testFleetBroadcastExpiry() {
    std::cout << "\n=== FleetBroadcast: Expiry ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    sys.sendBroadcast("f1", "bc1", BT::Target, "pilot_a", "Target 1", 5.0f);
    sys.sendBroadcast("f1", "bc2", BT::WarpTo, "pilot_b", "Gate", 15.0f);

    // Simulate 6 seconds — bc1 should expire, bc2 should remain
    sys.update(6.0f);
    assertTrue(sys.getActiveBroadcastCount("f1") == 1, "1 broadcast after expiry");
    assertTrue(!sys.hasBroadcast("f1", "bc1"), "bc1 expired");
    assertTrue(sys.hasBroadcast("f1", "bc2"), "bc2 still active");
    assertTrue(sys.getTotalExpired("f1") == 1, "1 total expired");

    // Simulate 10 more seconds — bc2 should also expire
    sys.update(10.0f);
    assertTrue(sys.getActiveBroadcastCount("f1") == 0, "0 after all expired");
    assertTrue(sys.getTotalExpired("f1") == 2, "2 total expired");
}

static void testFleetBroadcastDismiss() {
    std::cout << "\n=== FleetBroadcast: Dismiss ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    sys.sendBroadcast("f1", "bc1", BT::Target, "pilot", "Target", 30.0f);
    sys.sendBroadcast("f1", "bc2", BT::AlignTo, "pilot", "Planet", 30.0f);

    assertTrue(sys.dismissBroadcast("f1", "bc1"), "Dismiss bc1 succeeds");
    assertTrue(sys.getActiveBroadcastCount("f1") == 1, "1 remaining");
    assertTrue(!sys.hasBroadcast("f1", "bc1"), "bc1 dismissed");
    assertTrue(sys.hasBroadcast("f1", "bc2"), "bc2 still present");
    assertTrue(!sys.dismissBroadcast("f1", "bc1"), "Dismiss nonexistent fails");
    assertTrue(!sys.dismissBroadcast("f1", "unknown"), "Dismiss unknown fails");
}

static void testFleetBroadcastClear() {
    std::cout << "\n=== FleetBroadcast: Clear ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    sys.sendBroadcast("f1", "bc1", BT::Target, "pilot", "Target", 30.0f);
    sys.sendBroadcast("f1", "bc2", BT::WarpTo, "pilot", "Gate", 30.0f);

    assertTrue(sys.clearBroadcasts("f1"), "Clear succeeds");
    assertTrue(sys.getActiveBroadcastCount("f1") == 0, "0 after clear");
    assertTrue(sys.getTotalSent("f1") == 2, "Total sent preserved");
}

static void testFleetBroadcastCountByType() {
    std::cout << "\n=== FleetBroadcast: CountByType ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    sys.sendBroadcast("f1", "bc1", BT::Target, "pilot_a", "Enemy 1", 30.0f);
    sys.sendBroadcast("f1", "bc2", BT::Target, "pilot_b", "Enemy 2", 30.0f);
    sys.sendBroadcast("f1", "bc3", BT::NeedShieldReps, "pilot_c", "Need reps!", 30.0f);
    sys.sendBroadcast("f1", "bc4", BT::NeedArmorReps, "pilot_d", "Armor!", 30.0f);
    sys.sendBroadcast("f1", "bc5", BT::EnemySpotted, "pilot_e", "Hostile on scan", 30.0f);

    assertTrue(sys.getCountByType("f1", BT::Target) == 2, "2 Target broadcasts");
    assertTrue(sys.getCountByType("f1", BT::NeedShieldReps) == 1, "1 NeedShieldReps");
    assertTrue(sys.getCountByType("f1", BT::NeedArmorReps) == 1, "1 NeedArmorReps");
    assertTrue(sys.getCountByType("f1", BT::EnemySpotted) == 1, "1 EnemySpotted");
    assertTrue(sys.getCountByType("f1", BT::WarpTo) == 0, "0 WarpTo");
    assertTrue(sys.getCountByType("f1", BT::HoldPosition) == 0, "0 HoldPosition");
}

static void testFleetBroadcastGetTtl() {
    std::cout << "\n=== FleetBroadcast: GetTtl ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using BT = components::FleetBroadcastState::BroadcastType;
    sys.sendBroadcast("f1", "bc1", BT::Target, "pilot", "Target", 25.0f);

    assertTrue(approxEqual(sys.getTtl("f1", "bc1"), 25.0f), "TTL is 25");
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getTtl("f1", "bc1"), 15.0f), "TTL is 15 after 10s");
    assertTrue(approxEqual(sys.getTtl("f1", "unknown"), 0.0f), "0 TTL for unknown");
}

static void testFleetBroadcastSetFleetId() {
    std::cout << "\n=== FleetBroadcast: SetFleetId ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    assertTrue(sys.setFleetId("f1", "new_fleet"), "Set fleet ID succeeds");
    assertTrue(sys.getFleetId("f1") == "new_fleet", "Fleet ID updated");
    assertTrue(!sys.setFleetId("f1", ""), "Empty fleet ID rejected");
    assertTrue(!sys.setFleetId("nonexistent", "fleet"), "Fails on missing entity");
}

static void testFleetBroadcastMissing() {
    std::cout << "\n=== FleetBroadcast: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetBroadcastSystem sys(&world);

    using BT = components::FleetBroadcastState::BroadcastType;
    assertTrue(!sys.sendBroadcast("none", "bc1", BT::Target, "pilot", "T", 10.0f),
               "Send fails on missing");
    assertTrue(!sys.dismissBroadcast("none", "bc1"), "Dismiss fails on missing");
    assertTrue(!sys.clearBroadcasts("none"), "Clear fails on missing");
    assertTrue(!sys.setFleetId("none", "fleet"), "SetFleetId fails on missing");
    assertTrue(sys.getActiveBroadcastCount("none") == 0, "0 count on missing");
    assertTrue(sys.getTotalSent("none") == 0, "0 sent on missing");
    assertTrue(sys.getTotalExpired("none") == 0, "0 expired on missing");
    assertTrue(sys.getFleetId("none") == "", "Empty fleet ID on missing");
    assertTrue(!sys.hasBroadcast("none", "bc1"), "No broadcast on missing");
    assertTrue(sys.getCountByType("none", BT::Target) == 0, "0 type count on missing");
    assertTrue(approxEqual(sys.getTtl("none", "bc1"), 0.0f), "0 TTL on missing");
}

void run_fleet_broadcast_system_tests() {
    testFleetBroadcastInit();
    testFleetBroadcastSend();
    testFleetBroadcastSendValidation();
    testFleetBroadcastMaxCap();
    testFleetBroadcastExpiry();
    testFleetBroadcastDismiss();
    testFleetBroadcastClear();
    testFleetBroadcastCountByType();
    testFleetBroadcastGetTtl();
    testFleetBroadcastSetFleetId();
    testFleetBroadcastMissing();
}
