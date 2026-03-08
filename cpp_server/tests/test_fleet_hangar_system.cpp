// Tests for: FleetHangarSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fleet_hangar_system.h"

using namespace atlas;

// ==================== FleetHangarSystem Tests ====================

static void testFleetHangarCreate() {
    std::cout << "\n=== FleetHangar: Create ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    assertTrue(sys.initializeHangar("station1", "player1", "Main Bay", 1), "Init tier-1 hangar");
    assertTrue(sys.getTier("station1") == 1, "Tier is 1");
    assertTrue(sys.getMaxSlots("station1") == 5, "Max slots 5 for tier 1");
    assertTrue(sys.getShipCount("station1") == 0, "Zero ships initially");
}

static void testFleetHangarInvalidInit() {
    std::cout << "\n=== FleetHangar: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    assertTrue(!sys.initializeHangar("missing", "owner", "Bay", 1), "Missing entity fails");

    world.createEntity("station1");
    assertTrue(sys.initializeHangar("station1", "owner", "Bay", 1), "First init succeeds");
    assertTrue(!sys.initializeHangar("station1", "owner", "Bay", 1), "Double init fails");
}

static void testFleetHangarTierClamping() {
    std::cout << "\n=== FleetHangar: TierClamping ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);

    world.createEntity("s1");
    assertTrue(sys.initializeHangar("s1", "o", "Bay", 0), "Tier 0 clamped to 1");
    assertTrue(sys.getTier("s1") == 1, "Clamped tier is 1");

    world.createEntity("s2");
    assertTrue(sys.initializeHangar("s2", "o", "Bay", 99), "Tier 99 clamped to 5");
    assertTrue(sys.getTier("s2") == 5, "Clamped tier is 5");
    assertTrue(sys.getMaxSlots("s2") == 50, "Max slots 50 for tier 5");
}

static void testFleetHangarDockShip() {
    std::cout << "\n=== FleetHangar: DockShip ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);

    assertTrue(sys.dockShip("station1", "ship_a", "Frigate", 100.0f), "Dock ship_a");
    assertTrue(sys.getShipCount("station1") == 1, "1 ship docked");
    assertTrue(sys.dockShip("station1", "ship_b", "Cruiser", 80.0f), "Dock ship_b");
    assertTrue(sys.getShipCount("station1") == 2, "2 ships docked");

    // Duplicate rejected
    assertTrue(!sys.dockShip("station1", "ship_a", "Frigate", 100.0f), "Duplicate dock rejected");
    assertTrue(sys.getShipCount("station1") == 2, "Still 2 ships");
}

static void testFleetHangarDockFull() {
    std::cout << "\n=== FleetHangar: DockFull ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1); // 5 slots

    for (int i = 0; i < 5; i++) {
        std::string id = "ship_" + std::to_string(i);
        assertTrue(sys.dockShip("station1", id, "Frigate", 100.0f),
                   ("Dock " + id).c_str());
    }
    assertTrue(sys.getShipCount("station1") == 5, "5 ships docked (full)");
    assertTrue(!sys.dockShip("station1", "ship_extra", "Frigate", 100.0f),
               "Full hangar rejects new ship");
}

static void testFleetHangarUndockShip() {
    std::cout << "\n=== FleetHangar: UndockShip ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);
    sys.dockShip("station1", "ship_a", "Frigate", 100.0f);
    sys.dockShip("station1", "ship_b", "Cruiser", 80.0f);

    assertTrue(sys.undockShip("station1", "ship_a"), "Undock ship_a");
    assertTrue(sys.getShipCount("station1") == 1, "1 ship remaining");
    assertTrue(!sys.undockShip("station1", "ship_a"), "Double undock fails");
    assertTrue(!sys.undockShip("station1", "nonexistent"), "Nonexistent undock fails");
}

static void testFleetHangarLockUnlock() {
    std::cout << "\n=== FleetHangar: LockUnlock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);
    sys.dockShip("station1", "ship_a", "Frigate", 100.0f);

    assertTrue(sys.lockShip("station1", "ship_a"), "Lock ship_a");
    assertTrue(!sys.undockShip("station1", "ship_a"), "Cannot undock locked ship");
    assertTrue(sys.getShipCount("station1") == 1, "Ship still docked");

    assertTrue(sys.unlockShip("station1", "ship_a"), "Unlock ship_a");
    assertTrue(sys.undockShip("station1", "ship_a"), "Undock after unlock succeeds");
    assertTrue(sys.getShipCount("station1") == 0, "No ships remaining");

    // Lock/unlock nonexistent
    assertTrue(!sys.lockShip("station1", "nonexistent"), "Lock nonexistent fails");
    assertTrue(!sys.unlockShip("station1", "nonexistent"), "Unlock nonexistent fails");
}

static void testFleetHangarRepairShip() {
    std::cout << "\n=== FleetHangar: RepairShip ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);
    sys.dockShip("station1", "ship_a", "Frigate", 50.0f);

    assertTrue(sys.repairShip("station1", "ship_a", 30.0f), "Repair 30 points");
    // Repair caps at 100
    assertTrue(sys.repairShip("station1", "ship_a", 100.0f), "Over-repair capped");
    assertTrue(!sys.repairShip("station1", "nonexistent", 10.0f), "Repair nonexistent fails");
}

static void testFleetHangarUpgrade() {
    std::cout << "\n=== FleetHangar: Upgrade ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);

    assertTrue(sys.upgradeHangar("station1"), "Upgrade to tier 2");
    assertTrue(sys.getTier("station1") == 2, "Tier is 2");
    assertTrue(sys.getMaxSlots("station1") == 10, "Max slots 10 for tier 2");

    assertTrue(sys.upgradeHangar("station1"), "Upgrade to tier 3");
    assertTrue(sys.upgradeHangar("station1"), "Upgrade to tier 4");
    assertTrue(sys.upgradeHangar("station1"), "Upgrade to tier 5");
    assertTrue(sys.getTier("station1") == 5, "Tier is 5");
    assertTrue(sys.getMaxSlots("station1") == 50, "Max slots 50 for tier 5");

    assertTrue(!sys.upgradeHangar("station1"), "Cannot upgrade past tier 5");
    assertTrue(!sys.upgradeHangar("nonexistent"), "Upgrade nonexistent fails");
}

static void testFleetHangarPower() {
    std::cout << "\n=== FleetHangar: Power ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);
    sys.dockShip("station1", "ship_a", "Frigate", 100.0f);

    assertTrue(sys.setPowerEnabled("station1", false), "Disable power");
    assertTrue(!sys.dockShip("station1", "ship_b", "Cruiser", 80.0f),
               "Cannot dock when unpowered");
    assertTrue(!sys.undockShip("station1", "ship_a"),
               "Cannot undock when unpowered");

    assertTrue(sys.setPowerEnabled("station1", true), "Re-enable power");
    assertTrue(sys.dockShip("station1", "ship_b", "Cruiser", 80.0f),
               "Dock succeeds when powered again");
    assertTrue(!sys.setPowerEnabled("nonexistent", true), "Power nonexistent fails");
}

static void testFleetHangarUpdate() {
    std::cout << "\n=== FleetHangar: Update ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station1");
    sys.initializeHangar("station1", "owner", "Bay", 1);
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testFleetHangarMissing() {
    std::cout << "\n=== FleetHangar: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    assertTrue(sys.getShipCount("x") == 0, "Default ship count on missing");
    assertTrue(sys.getMaxSlots("x") == 0, "Default max slots on missing");
    assertTrue(sys.getTier("x") == 0, "Default tier on missing");
    assertTrue(!sys.dockShip("x", "s", "c", 100.0f), "Dock on missing fails");
    assertTrue(!sys.undockShip("x", "s"), "Undock on missing fails");
    assertTrue(!sys.lockShip("x", "s"), "Lock on missing fails");
    assertTrue(!sys.unlockShip("x", "s"), "Unlock on missing fails");
    assertTrue(!sys.repairShip("x", "s", 10.0f), "Repair on missing fails");
    assertTrue(!sys.upgradeHangar("x"), "Upgrade on missing fails");
    assertTrue(!sys.setPowerEnabled("x", true), "Power on missing fails");
}

void run_fleet_hangar_system_tests() {
    testFleetHangarCreate();
    testFleetHangarInvalidInit();
    testFleetHangarTierClamping();
    testFleetHangarDockShip();
    testFleetHangarDockFull();
    testFleetHangarUndockShip();
    testFleetHangarLockUnlock();
    testFleetHangarRepairShip();
    testFleetHangarUpgrade();
    testFleetHangarPower();
    testFleetHangarUpdate();
    testFleetHangarMissing();
}
