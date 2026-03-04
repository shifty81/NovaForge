// Tests for: Station Hangar System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "components/social_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/station_hangar_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Station Hangar System Tests ====================

static void testHangarCreate() {
    std::cout << "\n=== Hangar Create ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    // Create a station first.
    auto* station = world.createEntity("station1");
    auto stationComp = std::make_unique<components::Station>();
    stationComp->station_name = "Test Station";
    station->addComponent(std::move(stationComp));

    assertTrue(sys.createHangar("hangar1", "station1", "player1",
                                components::StationHangar::HangarType::Personal),
               "Hangar created");
    assertTrue(!sys.createHangar("hangar1", "station1", "player1"),
               "Duplicate hangar fails");
}

static void testHangarStoreShip() {
    std::cout << "\n=== Hangar Store Ship ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1");

    assertTrue(sys.storeShip("hangar1", "frigate_1"), "Store ship succeeds");
    assertTrue(sys.getStoredShipCount("hangar1") == 1, "One ship stored");
    assertTrue(!sys.storeShip("hangar1", "frigate_2"), "Hangar full at basic level");
}

static void testHangarRetrieveShip() {
    std::cout << "\n=== Hangar Retrieve Ship ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1");
    sys.storeShip("hangar1", "frigate_1");

    assertTrue(sys.retrieveShip("hangar1", "frigate_1"), "Retrieve succeeds");
    assertTrue(sys.getStoredShipCount("hangar1") == 0, "Hangar empty after retrieve");
    assertTrue(!sys.retrieveShip("hangar1", "frigate_1"), "Double retrieve fails");
}

static void testHangarUpgrade() {
    std::cout << "\n=== Hangar Upgrade ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1",
                     components::StationHangar::HangarType::Corporation);

    double cost = sys.upgradeHangar("hangar1");
    assertTrue(approxEqual(static_cast<float>(cost), 10000.0f), "Upgrade to Standard costs 10000");

    // After upgrade we should have 2 slots.
    assertTrue(sys.storeShip("hangar1", "ship_a"), "Slot 1");
    assertTrue(sys.storeShip("hangar1", "ship_b"), "Slot 2");
    assertTrue(!sys.storeShip("hangar1", "ship_c"), "Full at standard (2 slots)");
}

static void testHangarMaxUpgrade() {
    std::cout << "\n=== Hangar Max Upgrade ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1",
                     components::StationHangar::HangarType::Corporation);

    sys.upgradeHangar("hangar1"); // Basic -> Standard
    sys.upgradeHangar("hangar1"); // Standard -> Advanced
    sys.upgradeHangar("hangar1"); // Advanced -> Premium
    double cost = sys.upgradeHangar("hangar1"); // Already max
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No upgrade past Premium");
}

static void testHangarRentalAccrual() {
    std::cout << "\n=== Hangar Rental Accrual ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1",
                     components::StationHangar::HangarType::Leased);

    // Simulate 1 day (86400 seconds).
    sys.update(86400.0f);

    double balance = sys.getRentalBalance("hangar1");
    assertTrue(balance > 4999.0 && balance < 5001.0, "One day rental accrued ~5000 credits");
}

static void testHangarShouldUseHangarFrigate() {
    std::cout << "\n=== Should Use Hangar (Frigate) ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    auto* ship = world.createEntity("frig1");
    auto shipComp = std::make_unique<components::Ship>();
    shipComp->ship_class = "Frigate";
    ship->addComponent(std::move(shipComp));

    assertTrue(sys.shouldUseHangar("frig1"), "Frigate uses hangar");
}

static void testHangarShouldUseTetherCapital() {
    std::cout << "\n=== Should Use Tether (Capital) ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    auto* ship = world.createEntity("cap1");
    auto shipComp = std::make_unique<components::Ship>();
    shipComp->ship_class = "Capital";
    ship->addComponent(std::move(shipComp));

    assertTrue(!sys.shouldUseHangar("cap1"), "Capital uses tether (not hangar)");
}

static void testHangarShouldUseTetherTitan() {
    std::cout << "\n=== Should Use Tether (Titan) ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    auto* ship = world.createEntity("titan1");
    auto shipComp = std::make_unique<components::Ship>();
    shipComp->ship_class = "Titan";
    ship->addComponent(std::move(shipComp));

    assertTrue(!sys.shouldUseHangar("titan1"), "Titan uses tether (not hangar)");
}

static void testHangarSpawnPosition() {
    std::cout << "\n=== Hangar Spawn Position ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1");

    auto [x, y, z] = sys.getSpawnPosition("hangar1");
    assertTrue(approxEqual(x, 5.0f), "Spawn X is 5");
    assertTrue(approxEqual(y, 0.0f), "Spawn Y is 0");
    assertTrue(approxEqual(z, 2.0f), "Spawn Z is 2");
}

static void testHangarDuplicateShip() {
    std::cout << "\n=== Hangar Duplicate Ship ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    // Upgrade to 2 slots.
    sys.createHangar("hangar1", "station1", "player1");
    sys.upgradeHangar("hangar1");

    assertTrue(sys.storeShip("hangar1", "ship1"), "First store succeeds");
    assertTrue(!sys.storeShip("hangar1", "ship1"), "Duplicate ship blocked");
}

static void testHangarComponentDefaults() {
    std::cout << "\n=== Hangar Component Defaults ===" << std::endl;
    components::StationHangar h;
    assertTrue(h.type == components::StationHangar::HangarType::Leased, "Default type Leased");
    assertTrue(h.upgrade_level == components::StationHangar::UpgradeLevel::Basic, "Default Basic level");
    assertTrue(h.max_ship_slots == 1, "Default 1 slot");
    assertTrue(h.hasRoom(), "Has room at 0/1");
    assertTrue(h.isLeased(), "Is leased");
}


void run_station_hangar_system_tests() {
    testHangarCreate();
    testHangarStoreShip();
    testHangarRetrieveShip();
    testHangarUpgrade();
    testHangarMaxUpgrade();
    testHangarRentalAccrual();
    testHangarShouldUseHangarFrigate();
    testHangarShouldUseTetherCapital();
    testHangarShouldUseTetherTitan();
    testHangarSpawnPosition();
    testHangarDuplicateShip();
    testHangarComponentDefaults();
}
