// Tests for: FleetWarpCoordinatorSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/fleet_warp_coordinator_system.h"

using namespace atlas;

// ==================== FleetWarpCoordinatorSystem Tests ====================

static void testFleetWarpCreate() {
    std::cout << "\n=== FleetWarpCoordinator: Create ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initialize("fleet1", "alpha_fleet", "cmd_ship"), "Init fleet succeeds");
    assertTrue(sys.getMemberCount("fleet1") == 0, "Zero members initially");
    assertTrue(sys.getTotalFleetWarps("fleet1") == 0, "Zero warps initially");
    assertTrue(sys.getTotalMembersWarped("fleet1") == 0, "Zero members warped");
    assertTrue(!sys.isWarpInitiated("fleet1"), "No warp initiated");
    assertTrue(!sys.isWarpActive("fleet1"), "No warp active");
    assertTrue(sys.getDestination("fleet1").empty(), "No destination");
}

static void testFleetWarpInvalidInit() {
    std::cout << "\n=== FleetWarpCoordinator: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    assertTrue(!sys.initialize("missing", "f", "c"), "Missing entity fails");
    world.createEntity("fleet1");
    assertTrue(!sys.initialize("fleet1", "", "c"), "Empty fleet_id fails");
    assertTrue(!sys.initialize("fleet1", "f", ""), "Empty commander fails");
}

static void testFleetWarpAddMembers() {
    std::cout << "\n=== FleetWarpCoordinator: AddMembers ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");

    assertTrue(sys.addMember("fleet1", "ship1", 3.0f), "Add ship1");
    assertTrue(sys.addMember("fleet1", "ship2", 5.0f), "Add ship2");
    assertTrue(sys.addMember("fleet1", "ship3", 4.0f), "Add ship3");
    assertTrue(sys.getMemberCount("fleet1") == 3, "3 members");

    // Duplicate rejected
    assertTrue(!sys.addMember("fleet1", "ship1", 3.0f), "Duplicate ship rejected");

    // Invalid args
    assertTrue(!sys.addMember("fleet1", "", 3.0f), "Empty ship_id rejected");
    assertTrue(!sys.addMember("fleet1", "ship4", 0.0f), "Zero align time rejected");
    assertTrue(!sys.addMember("fleet1", "ship4", -1.0f), "Negative align time rejected");
    assertTrue(!sys.addMember("nonexistent", "ship4", 3.0f), "Missing entity rejected");
}

static void testFleetWarpRemoveMember() {
    std::cout << "\n=== FleetWarpCoordinator: RemoveMember ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");
    sys.addMember("fleet1", "ship1", 3.0f);
    sys.addMember("fleet1", "ship2", 5.0f);

    assertTrue(sys.removeMember("fleet1", "ship1"), "Remove ship1 succeeds");
    assertTrue(sys.getMemberCount("fleet1") == 1, "1 member remaining");
    assertTrue(!sys.removeMember("fleet1", "ship1"), "Double remove fails");
    assertTrue(!sys.removeMember("fleet1", "nonexistent"), "Remove nonexistent fails");
}

static void testFleetWarpInitiate() {
    std::cout << "\n=== FleetWarpCoordinator: Initiate ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");
    sys.addMember("fleet1", "ship1", 3.0f);
    sys.addMember("fleet1", "ship2", 5.0f);

    assertTrue(sys.initiateWarp("fleet1", "planet_alpha"), "Initiate warp succeeds");
    assertTrue(sys.isWarpInitiated("fleet1"), "Warp is initiated");
    assertTrue(sys.getDestination("fleet1") == "planet_alpha", "Destination set");
    assertTrue(sys.getWarpCountdown("fleet1") > 0.0f, "Countdown started");

    // Can't initiate twice
    assertTrue(!sys.initiateWarp("fleet1", "planet_beta"), "Double initiate rejected");
}

static void testFleetWarpInvalidInitiate() {
    std::cout << "\n=== FleetWarpCoordinator: InvalidInitiate ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");

    // No members
    assertTrue(!sys.initiateWarp("fleet1", "planet_alpha"), "No members fails");

    sys.addMember("fleet1", "ship1", 3.0f);
    assertTrue(!sys.initiateWarp("fleet1", ""), "Empty destination rejected");
    assertTrue(!sys.initiateWarp("nonexistent", "planet_alpha"), "Missing entity rejected");
}

static void testFleetWarpAlignment() {
    std::cout << "\n=== FleetWarpCoordinator: Alignment ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");
    sys.addMember("fleet1", "ship1", 2.0f);  // 2s align time
    sys.addMember("fleet1", "ship2", 4.0f);  // 4s align time

    sys.initiateWarp("fleet1", "planet_alpha");

    // After 2s: ship1 aligned, ship2 half-aligned
    sys.update(2.0f);
    float a1 = sys.getMemberAlignment("fleet1", "ship1");
    float a2 = sys.getMemberAlignment("fleet1", "ship2");
    assertTrue(a1 >= 0.99f, "Ship1 fully aligned after 2s");
    assertTrue(a2 > 0.49f && a2 < 0.51f, "Ship2 ~50% aligned after 2s");
    assertTrue(sys.getReadyCount("fleet1") == 1, "1 ready member");
    assertTrue(!sys.isAllReady("fleet1"), "Not all ready yet");
}

static void testFleetWarpFullSequence() {
    std::cout << "\n=== FleetWarpCoordinator: FullSequence ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");
    sys.addMember("fleet1", "ship1", 2.0f);
    sys.addMember("fleet1", "ship2", 3.0f);

    sys.initiateWarp("fleet1", "planet_alpha");

    // Align all (3s for slowest)
    sys.update(3.0f);
    assertTrue(sys.isAllReady("fleet1"), "All ready after 3s");

    // Countdown (default 3s)
    sys.update(3.0f);
    assertTrue(sys.isWarpActive("fleet1"), "Warp now active");
    assertTrue(sys.getTotalFleetWarps("fleet1") == 1, "1 fleet warp completed");
    assertTrue(sys.getTotalMembersWarped("fleet1") == 2, "2 members warped");
}

static void testFleetWarpCancel() {
    std::cout << "\n=== FleetWarpCoordinator: Cancel ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");
    sys.addMember("fleet1", "ship1", 2.0f);

    sys.initiateWarp("fleet1", "planet_alpha");
    sys.update(1.0f);

    assertTrue(sys.cancelWarp("fleet1"), "Cancel succeeds");
    assertTrue(!sys.isWarpInitiated("fleet1"), "Warp no longer initiated");
    assertTrue(!sys.isWarpActive("fleet1"), "Warp not active");
    assertTrue(sys.getDestination("fleet1").empty(), "Destination cleared");

    // Can't cancel when not initiated
    assertTrue(!sys.cancelWarp("fleet1"), "Cancel when not initiated fails");
}

static void testFleetWarpUpdate() {
    std::cout << "\n=== FleetWarpCoordinator: Update ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1", "alpha_fleet", "cmd_ship");
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testFleetWarpMissing() {
    std::cout << "\n=== FleetWarpCoordinator: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetWarpCoordinatorSystem sys(&world);
    assertTrue(sys.getMemberCount("x") == 0, "Default members on missing");
    assertTrue(sys.getReadyCount("x") == 0, "Default ready on missing");
    assertTrue(!sys.isAllReady("x"), "Default all ready on missing");
    assertTrue(!sys.isWarpActive("x"), "Default warp active on missing");
    assertTrue(!sys.isWarpInitiated("x"), "Default warp initiated on missing");
    assertTrue(approxEqual(sys.getMemberAlignment("x", "s"), 0.0f), "Default alignment on missing");
    assertTrue(sys.getDestination("x").empty(), "Default destination on missing");
    assertTrue(sys.getTotalFleetWarps("x") == 0, "Default fleet warps on missing");
    assertTrue(sys.getTotalMembersWarped("x") == 0, "Default members warped on missing");
    assertTrue(approxEqual(sys.getWarpCountdown("x"), 0.0f), "Default countdown on missing");
}

void run_fleet_warp_coordinator_system_tests() {
    testFleetWarpCreate();
    testFleetWarpInvalidInit();
    testFleetWarpAddMembers();
    testFleetWarpRemoveMember();
    testFleetWarpInitiate();
    testFleetWarpInvalidInitiate();
    testFleetWarpAlignment();
    testFleetWarpFullSequence();
    testFleetWarpCancel();
    testFleetWarpUpdate();
    testFleetWarpMissing();
}
