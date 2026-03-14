// Tests for: HangarSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/hangar_system.h"

using namespace atlas;

// ==================== HangarSystem Tests ====================

static void testHangarInit() {
    std::cout << "\n=== Hangar: Init ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getShipCount("e1") == 0, "Zero ships initially");
    assertTrue(sys.getTotalShipsStored("e1") == 0, "Zero total stored");
    assertTrue(sys.getTotalShipsRetrieved("e1") == 0, "Zero total retrieved");
    assertTrue(sys.getStationId("e1") == "", "Empty station initially");
    assertTrue(sys.getActiveShipId("e1") == "", "No active ship initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testHangarStoreShip() {
    std::cout << "\n=== Hangar: StoreShip ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.storeShip("e1", "s1", "Frigate", "My Rifter"), "Store ship");
    assertTrue(sys.getShipCount("e1") == 1, "1 ship");
    assertTrue(sys.hasShip("e1", "s1"), "Has s1");
    assertTrue(sys.getShipName("e1", "s1") == "My Rifter", "Name matches");
    assertTrue(sys.getShipType("e1", "s1") == "Frigate", "Type matches");
    assertTrue(sys.getTotalShipsStored("e1") == 1, "1 total stored");
    assertTrue(sys.storeShip("e1", "s2", "Cruiser", "My Omen"), "Store second ship");
    assertTrue(sys.getShipCount("e1") == 2, "2 ships");
    assertTrue(sys.getTotalShipsStored("e1") == 2, "2 total stored");
}

static void testHangarStoreValidation() {
    std::cout << "\n=== Hangar: StoreValidation ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.storeShip("e1", "", "Frigate", "Name"), "Empty id rejected");
    assertTrue(!sys.storeShip("e1", "s1", "", "Name"), "Empty type rejected");
    assertTrue(!sys.storeShip("e1", "s1", "Frigate", ""), "Empty name rejected");
    assertTrue(sys.storeShip("e1", "s1", "Frigate", "Ship1"), "Valid store");
    assertTrue(!sys.storeShip("e1", "s1", "Cruiser", "Dup"), "Duplicate id rejected");
    assertTrue(sys.getShipCount("e1") == 1, "Still 1 ship");
    assertTrue(!sys.storeShip("missing", "s2", "F", "N"), "Missing entity rejected");
}

static void testHangarCapacity() {
    std::cout << "\n=== Hangar: Capacity ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxShips("e1", 3);

    assertTrue(sys.storeShip("e1", "s1", "F", "Ship1"), "Store 1");
    assertTrue(sys.storeShip("e1", "s2", "F", "Ship2"), "Store 2");
    assertTrue(sys.storeShip("e1", "s3", "F", "Ship3"), "Store 3 at cap");
    assertTrue(!sys.storeShip("e1", "s4", "F", "Ship4"), "Store 4 rejected");
    assertTrue(sys.getShipCount("e1") == 3, "Still 3 ships");
    assertTrue(sys.hasShip("e1", "s1"), "s1 still present");
}

static void testHangarRetrieve() {
    std::cout << "\n=== Hangar: Retrieve ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.storeShip("e1", "s1", "Frigate", "Ship1");
    sys.storeShip("e1", "s2", "Cruiser", "Ship2");

    assertTrue(sys.retrieveShip("e1", "s1"), "Retrieve s1");
    assertTrue(sys.getShipCount("e1") == 1, "1 ship left");
    assertTrue(!sys.hasShip("e1", "s1"), "s1 gone");
    assertTrue(sys.hasShip("e1", "s2"), "s2 present");
    assertTrue(sys.getTotalShipsRetrieved("e1") == 1, "1 total retrieved");
    assertTrue(!sys.retrieveShip("e1", "s1"), "Retrieve already gone fails");
    assertTrue(!sys.retrieveShip("e1", "unknown"), "Retrieve unknown fails");
}

static void testHangarRename() {
    std::cout << "\n=== Hangar: Rename ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.storeShip("e1", "s1", "Frigate", "OldName");

    assertTrue(sys.renameShip("e1", "s1", "NewName"), "Rename succeeds");
    assertTrue(sys.getShipName("e1", "s1") == "NewName", "Name updated");
    assertTrue(!sys.renameShip("e1", "s1", ""), "Empty name rejected");
    assertTrue(sys.getShipName("e1", "s1") == "NewName", "Name unchanged");
    assertTrue(!sys.renameShip("e1", "s99", "X"), "Rename missing ship fails");
    assertTrue(!sys.renameShip("missing", "s1", "X"), "Rename missing entity fails");
}

static void testHangarActiveShip() {
    std::cout << "\n=== Hangar: ActiveShip ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.storeShip("e1", "s1", "Frigate", "Ship1");
    sys.storeShip("e1", "s2", "Cruiser", "Ship2");

    assertTrue(sys.getActiveShipId("e1") == "", "No active initially");
    assertTrue(sys.setActiveShip("e1", "s1"), "Set s1 active");
    assertTrue(sys.getActiveShipId("e1") == "s1", "s1 is active");
    assertTrue(sys.setActiveShip("e1", "s2"), "Set s2 active");
    assertTrue(sys.getActiveShipId("e1") == "s2", "s2 is active now");
    // s1 should no longer be active
    assertTrue(!sys.setActiveShip("e1", "s99"), "Set missing ship fails");
    assertTrue(sys.getActiveShipId("e1") == "s2", "Still s2 active after failed set");
}

static void testHangarInsurance() {
    std::cout << "\n=== Hangar: Insurance ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.storeShip("e1", "s1", "Frigate", "Ship1");

    assertTrue(approxEqual(sys.getInsurance("e1", "s1"), 0.0f), "0 insurance initially");
    assertTrue(sys.setInsurance("e1", "s1", 50000.0f), "Set insurance");
    assertTrue(approxEqual(sys.getInsurance("e1", "s1"), 50000.0f), "Insurance updated");
    assertTrue(!sys.setInsurance("e1", "s1", -100.0f), "Negative insurance rejected");
    assertTrue(approxEqual(sys.getInsurance("e1", "s1"), 50000.0f), "Insurance unchanged");
    assertTrue(sys.setInsurance("e1", "s1", 0.0f), "Zero insurance allowed");
    assertTrue(!sys.setInsurance("e1", "s99", 100.0f), "Insurance missing ship fails");
}

static void testHangarConfig() {
    std::cout << "\n=== Hangar: Config ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setStationId("e1", "STATION_001"), "Set station_id");
    assertTrue(sys.getStationId("e1") == "STATION_001", "Station matches");
    assertTrue(!sys.setStationId("e1", ""), "Empty station rejected");
    assertTrue(sys.setMaxShips("e1", 25), "Set max ships 25");
    assertTrue(!sys.setMaxShips("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxShips("e1", -5), "Negative max rejected");
}

static void testHangarClear() {
    std::cout << "\n=== Hangar: Clear ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.storeShip("e1", "s1", "F", "Ship1");
    sys.storeShip("e1", "s2", "C", "Ship2");

    assertTrue(sys.clearHangar("e1"), "Clear succeeds");
    assertTrue(sys.getShipCount("e1") == 0, "0 ships after clear");
    assertTrue(!sys.hasShip("e1", "s1"), "s1 gone");
    assertTrue(sys.getTotalShipsStored("e1") == 2, "Total stored preserved");
}

static void testHangarShipCountByType() {
    std::cout << "\n=== Hangar: ShipCountByType ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.storeShip("e1", "s1", "Frigate", "Ship1");
    sys.storeShip("e1", "s2", "Frigate", "Ship2");
    sys.storeShip("e1", "s3", "Cruiser", "Ship3");
    sys.storeShip("e1", "s4", "Battleship", "Ship4");

    assertTrue(sys.getShipCountByType("e1", "Frigate") == 2, "2 frigates");
    assertTrue(sys.getShipCountByType("e1", "Cruiser") == 1, "1 cruiser");
    assertTrue(sys.getShipCountByType("e1", "Battleship") == 1, "1 battleship");
    assertTrue(sys.getShipCountByType("e1", "Destroyer") == 0, "0 destroyers");
}

static void testHangarMissing() {
    std::cout << "\n=== Hangar: Missing ===" << std::endl;
    ecs::World world;
    systems::HangarSystem sys(&world);

    assertTrue(!sys.storeShip("none", "s1", "F", "N"), "Store fails on missing");
    assertTrue(!sys.retrieveShip("none", "s1"), "Retrieve fails on missing");
    assertTrue(!sys.renameShip("none", "s1", "X"), "Rename fails on missing");
    assertTrue(!sys.setActiveShip("none", "s1"), "SetActive fails on missing");
    assertTrue(!sys.setInsurance("none", "s1", 100), "SetInsurance fails on missing");
    assertTrue(!sys.clearHangar("none"), "Clear fails on missing");
    assertTrue(!sys.setStationId("none", "S"), "SetStation fails on missing");
    assertTrue(!sys.setMaxShips("none", 10), "SetMax fails on missing");
    assertTrue(sys.getShipCount("none") == 0, "0 count on missing");
    assertTrue(!sys.hasShip("none", "s1"), "No ship on missing");
    assertTrue(sys.getShipName("none", "s1") == "", "Empty name on missing");
    assertTrue(sys.getShipType("none", "s1") == "", "Empty type on missing");
    assertTrue(sys.getActiveShipId("none") == "", "Empty active on missing");
    assertTrue(sys.getStationId("none") == "", "Empty station on missing");
    assertTrue(approxEqual(sys.getInsurance("none", "s1"), 0.0f), "0 insurance on missing");
    assertTrue(sys.getTotalShipsStored("none") == 0, "0 total stored on missing");
    assertTrue(sys.getTotalShipsRetrieved("none") == 0, "0 total retrieved on missing");
    assertTrue(sys.getShipCountByType("none", "F") == 0, "0 by type on missing");
}

void run_hangar_system_tests() {
    testHangarInit();
    testHangarStoreShip();
    testHangarStoreValidation();
    testHangarCapacity();
    testHangarRetrieve();
    testHangarRename();
    testHangarActiveShip();
    testHangarInsurance();
    testHangarConfig();
    testHangarClear();
    testHangarShipCountByType();
    testHangarMissing();
}
