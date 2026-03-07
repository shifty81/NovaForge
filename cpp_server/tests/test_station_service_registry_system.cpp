// Tests for: Station Service Registry System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/station_service_registry_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Station Service Registry System Tests ====================

static void testStationServiceRegistryCreate() {
    std::cout << "\n=== StationServiceRegistry: Create ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    assertTrue(sys.initialize("station1"), "Init succeeds");
    assertTrue(sys.getServiceCount("station1") == 0, "No services initially");
    assertTrue(sys.getAvailableCount("station1") == 0, "No available services");
    assertTrue(sys.getTotalUses("station1") == 0, "No uses initially");
    assertTrue(approxEqual(sys.getTotalRevenue("station1"), 0.0f), "No revenue initially");
}

static void testStationServiceRegistryRegister() {
    std::cout << "\n=== StationServiceRegistry: Register ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    assertTrue(sys.registerService("station1", "repair", "Ship Repair", Cat::Repair, 5000.0f, 0.0f), "Register repair");
    assertTrue(sys.registerService("station1", "refine", "Ore Refinery", Cat::Refine, 2000.0f, 60.0f), "Register refine");
    assertTrue(sys.registerService("station1", "market", "Market Access", Cat::Market, 0.0f, 0.0f), "Register market");
    assertTrue(sys.getServiceCount("station1") == 3, "3 services registered");
    assertTrue(sys.getAvailableCount("station1") == 3, "All 3 available");
}

static void testStationServiceRegistryDuplicate() {
    std::cout << "\n=== StationServiceRegistry: Duplicate ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "repair", "Ship Repair", Cat::Repair, 5000.0f, 0.0f);
    assertTrue(!sys.registerService("station1", "repair", "Another Repair", Cat::Repair, 3000.0f, 0.0f), "Duplicate rejected");
    assertTrue(sys.getServiceCount("station1") == 1, "Still 1 service");
}

static void testStationServiceRegistryUseService() {
    std::cout << "\n=== StationServiceRegistry: UseService ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "repair", "Ship Repair", Cat::Repair, 5000.0f, 0.0f);
    assertTrue(sys.useService("station1", "repair"), "Use repair succeeds");
    assertTrue(sys.getTimesUsed("station1", "repair") == 1, "1 time used");
    assertTrue(sys.getTotalUses("station1") == 1, "1 total use");
    assertTrue(approxEqual(sys.getTotalRevenue("station1"), 5000.0f), "Revenue = 5000");
    assertTrue(sys.useService("station1", "repair"), "Use repair again (no cooldown)");
    assertTrue(sys.getTotalUses("station1") == 2, "2 total uses");
    assertTrue(approxEqual(sys.getTotalRevenue("station1"), 10000.0f), "Revenue = 10000");
}

static void testStationServiceRegistryCooldown() {
    std::cout << "\n=== StationServiceRegistry: Cooldown ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "refine", "Ore Refinery", Cat::Refine, 2000.0f, 30.0f);
    assertTrue(sys.useService("station1", "refine"), "First use succeeds");
    assertTrue(sys.isOnCooldown("station1", "refine"), "On cooldown after use");
    assertTrue(!sys.useService("station1", "refine"), "Second use blocked by cooldown");

    sys.update(15.0f);
    assertTrue(sys.isOnCooldown("station1", "refine"), "Still on cooldown at 15s");
    sys.update(16.0f);
    assertTrue(!sys.isOnCooldown("station1", "refine"), "Cooldown expired at 31s");
    assertTrue(sys.useService("station1", "refine"), "Use succeeds after cooldown");
}

static void testStationServiceRegistryAvailability() {
    std::cout << "\n=== StationServiceRegistry: Availability ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "repair", "Ship Repair", Cat::Repair, 5000.0f, 0.0f);
    assertTrue(sys.isServiceAvailable("station1", "repair"), "Available by default");
    assertTrue(sys.setAvailability("station1", "repair", false), "Disable succeeds");
    assertTrue(!sys.isServiceAvailable("station1", "repair"), "Not available after disable");
    assertTrue(!sys.useService("station1", "repair"), "Can't use disabled service");
    assertTrue(sys.getAvailableCount("station1") == 0, "0 available");
    assertTrue(sys.setAvailability("station1", "repair", true), "Re-enable succeeds");
    assertTrue(sys.isServiceAvailable("station1", "repair"), "Available again");
}

static void testStationServiceRegistryRemove() {
    std::cout << "\n=== StationServiceRegistry: Remove ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "repair", "Ship Repair", Cat::Repair, 5000.0f, 0.0f);
    assertTrue(sys.removeService("station1", "repair"), "Remove succeeds");
    assertTrue(sys.getServiceCount("station1") == 0, "0 services after remove");
    assertTrue(!sys.removeService("station1", "repair"), "Double remove fails");
}

static void testStationServiceRegistryCost() {
    std::cout << "\n=== StationServiceRegistry: Cost ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "clone", "Clone Bay", Cat::CloneBay, 15000.0f, 0.0f);
    assertTrue(approxEqual(sys.getServiceCost("station1", "clone"), 15000.0f), "Cost is 15000");
    assertTrue(approxEqual(sys.getServiceCost("station1", "nonexistent"), 0.0f), "0 cost for missing");
}

static void testStationServiceRegistryMaxLimit() {
    std::cout << "\n=== StationServiceRegistry: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    auto* entity = world.getEntity("station1");
    auto* comp = entity->getComponent<components::StationServiceRegistry>();
    comp->max_services = 2;

    using Cat = components::StationServiceRegistry::ServiceCategory;
    sys.registerService("station1", "repair", "Ship Repair", Cat::Repair, 5000.0f, 0.0f);
    sys.registerService("station1", "market", "Market", Cat::Market, 0.0f, 0.0f);
    assertTrue(!sys.registerService("station1", "clone", "Clone Bay", Cat::CloneBay, 15000.0f, 0.0f), "Max limit enforced");
    assertTrue(sys.getServiceCount("station1") == 2, "Still 2 services");
}

static void testStationServiceRegistryMissing() {
    std::cout << "\n=== StationServiceRegistry: Missing ===" << std::endl;
    ecs::World world;
    systems::StationServiceRegistrySystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");

    using Cat = components::StationServiceRegistry::ServiceCategory;
    assertTrue(!sys.registerService("nonexistent", "r", "R", Cat::Repair, 1.0f, 0.0f), "Register fails on missing");
    assertTrue(!sys.removeService("nonexistent", "r"), "Remove fails on missing");
    assertTrue(!sys.useService("nonexistent", "r"), "Use fails on missing");
    assertTrue(!sys.setAvailability("nonexistent", "r", true), "SetAvail fails on missing");
    assertTrue(sys.getServiceCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getAvailableCount("nonexistent") == 0, "0 available on missing");
    assertTrue(approxEqual(sys.getServiceCost("nonexistent", "r"), 0.0f), "0 cost on missing");
    assertTrue(sys.getTimesUsed("nonexistent", "r") == 0, "0 used on missing");
    assertTrue(!sys.isServiceAvailable("nonexistent", "r"), "Not available on missing");
    assertTrue(!sys.isOnCooldown("nonexistent", "r"), "Not on cooldown on missing");
    assertTrue(sys.getTotalUses("nonexistent") == 0, "0 total uses on missing");
    assertTrue(approxEqual(sys.getTotalRevenue("nonexistent"), 0.0f), "0 revenue on missing");
}


void run_station_service_registry_system_tests() {
    testStationServiceRegistryCreate();
    testStationServiceRegistryRegister();
    testStationServiceRegistryDuplicate();
    testStationServiceRegistryUseService();
    testStationServiceRegistryCooldown();
    testStationServiceRegistryAvailability();
    testStationServiceRegistryRemove();
    testStationServiceRegistryCost();
    testStationServiceRegistryMaxLimit();
    testStationServiceRegistryMissing();
}
