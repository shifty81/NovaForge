// Tests for: Station Service Broker System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/station_service_broker_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Station Service Broker System Tests ====================

static void testStationServiceBrokerCreate() {
    std::cout << "\n=== StationServiceBroker: Create ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    assertTrue(sys.initialize("ssb1"), "Init succeeds");
    assertTrue(sys.getServiceCount("ssb1") == 0, "No services initially");
    assertTrue(approxEqual(sys.getTaxRate("ssb1"), 0.05f), "Default tax rate 5%");
    assertTrue(approxEqual(sys.getStandingDiscount("ssb1"), 0.0f), "No standing discount");
    assertTrue(approxEqual(sys.getTotalRevenue("ssb1"), 0.0f), "0 revenue initially");
    assertTrue(sys.getTotalTransactions("ssb1") == 0, "0 transactions initially");
}

static void testStationServiceBrokerAddService() {
    std::cout << "\n=== StationServiceBroker: AddService ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    assertTrue(sys.addService("ssb1", "repair", 500.0f), "Add repair service");
    assertTrue(sys.getServiceCount("ssb1") == 1, "1 service");
    assertTrue(sys.isServiceAvailable("ssb1", "repair"), "Repair available");
    assertTrue(!sys.addService("ssb1", "repair", 500.0f), "Duplicate rejected");
    assertTrue(!sys.addService("ssb1", "", 100.0f), "Empty ID rejected");
    assertTrue(!sys.addService("ssb1", "bad", -10.0f), "Negative cost rejected");
}

static void testStationServiceBrokerRemoveService() {
    std::cout << "\n=== StationServiceBroker: RemoveService ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.addService("ssb1", "repair", 500.0f);
    assertTrue(sys.removeService("ssb1", "repair"), "Remove repair");
    assertTrue(sys.getServiceCount("ssb1") == 0, "0 services after remove");
    assertTrue(!sys.removeService("ssb1", "repair"), "Double remove fails");
}

static void testStationServiceBrokerServiceCost() {
    std::cout << "\n=== StationServiceBroker: ServiceCost ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.addService("ssb1", "repair", 1000.0f);
    // Cost = base * demand(1.0) * (1 + tax(0.05)) * (1 - discount(0.0)) = 1050.0
    float cost = sys.getServiceCost("ssb1", "repair");
    assertTrue(approxEqual(cost, 1050.0f), "Cost is 1050 ISC with 5% tax");
}

static void testStationServiceBrokerTaxAndDiscount() {
    std::cout << "\n=== StationServiceBroker: TaxAndDiscount ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.addService("ssb1", "repair", 1000.0f);
    sys.setTaxRate("ssb1", 0.10f);
    sys.setStandingDiscount("ssb1", 0.20f);
    // Cost = 1000 * 1.0 * 1.10 * 0.80 = 880.0
    float cost = sys.getServiceCost("ssb1", "repair");
    assertTrue(approxEqual(cost, 880.0f), "Cost is 880 ISC with 10% tax and 20% discount");
}

static void testStationServiceBrokerDemand() {
    std::cout << "\n=== StationServiceBroker: Demand ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.addService("ssb1", "refuel", 200.0f);
    assertTrue(sys.setDemandModifier("ssb1", "refuel", 2.0f), "Set demand 2x");
    // Cost = 200 * 2.0 * 1.05 * 1.0 = 420
    float cost = sys.getServiceCost("ssb1", "refuel");
    assertTrue(approxEqual(cost, 420.0f), "Cost 420 with 2x demand");
    // Clamp
    sys.setDemandModifier("ssb1", "refuel", 10.0f);
    float cost2 = sys.getServiceCost("ssb1", "refuel");
    // 200 * 5.0 * 1.05 = 1050
    assertTrue(approxEqual(cost2, 1050.0f), "Demand clamped to 5.0");
}

static void testStationServiceBrokerAvailability() {
    std::cout << "\n=== StationServiceBroker: Availability ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.addService("ssb1", "repair", 500.0f);
    assertTrue(sys.isServiceAvailable("ssb1", "repair"), "Initially available");
    assertTrue(sys.setServiceAvailable("ssb1", "repair", false), "Disable service");
    assertTrue(!sys.isServiceAvailable("ssb1", "repair"), "Now unavailable");
    assertTrue(!sys.processTransaction("ssb1", "repair"), "Cannot use unavailable service");
}

static void testStationServiceBrokerTransaction() {
    std::cout << "\n=== StationServiceBroker: Transaction ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.addService("ssb1", "repair", 1000.0f);
    assertTrue(sys.processTransaction("ssb1", "repair"), "Transaction succeeds");
    assertTrue(sys.getTotalTransactions("ssb1") == 1, "1 transaction");
    assertTrue(sys.getTotalRevenue("ssb1") > 0.0f, "Revenue > 0");
}

static void testStationServiceBrokerMaxServices() {
    std::cout << "\n=== StationServiceBroker: MaxServices ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    for (int i = 0; i < 12; ++i) {
        assertTrue(sys.addService("ssb1", "svc" + std::to_string(i), 100.0f), "Add service " + std::to_string(i));
    }
    assertTrue(!sys.addService("ssb1", "overflow", 100.0f), "13th service rejected (max 12)");
    assertTrue(sys.getServiceCount("ssb1") == 12, "12 services at max");
}

static void testStationServiceBrokerTaxClamp() {
    std::cout << "\n=== StationServiceBroker: TaxClamp ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    world.createEntity("ssb1");
    sys.initialize("ssb1");
    sys.setTaxRate("ssb1", 1.0f);  // should clamp to 0.5
    assertTrue(approxEqual(sys.getTaxRate("ssb1"), 0.5f), "Tax clamped to 0.5");
    sys.setTaxRate("ssb1", -0.1f);  // should clamp to 0
    assertTrue(approxEqual(sys.getTaxRate("ssb1"), 0.0f), "Tax clamped to 0");
    sys.setStandingDiscount("ssb1", 0.8f);  // should clamp to 0.5
    assertTrue(approxEqual(sys.getStandingDiscount("ssb1"), 0.5f), "Discount clamped to 0.5");
}

static void testStationServiceBrokerMissing() {
    std::cout << "\n=== StationServiceBroker: Missing ===" << std::endl;
    ecs::World world;
    systems::StationServiceBrokerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addService("nonexistent", "repair", 100.0f), "Add fails on missing");
    assertTrue(!sys.removeService("nonexistent", "repair"), "Remove fails on missing");
    assertTrue(!sys.setServiceAvailable("nonexistent", "repair", true), "Avail fails on missing");
    assertTrue(!sys.setDemandModifier("nonexistent", "repair", 1.0f), "Demand fails on missing");
    assertTrue(!sys.setTaxRate("nonexistent", 0.1f), "Tax fails on missing");
    assertTrue(!sys.setStandingDiscount("nonexistent", 0.1f), "Discount fails on missing");
    assertTrue(!sys.processTransaction("nonexistent", "repair"), "Transaction fails on missing");
    assertTrue(approxEqual(sys.getServiceCost("nonexistent", "repair"), 0.0f), "0 cost on missing");
    assertTrue(!sys.isServiceAvailable("nonexistent", "repair"), "Not available on missing");
    assertTrue(approxEqual(sys.getTaxRate("nonexistent"), 0.0f), "0 tax on missing");
    assertTrue(approxEqual(sys.getStandingDiscount("nonexistent"), 0.0f), "0 discount on missing");
    assertTrue(approxEqual(sys.getTotalRevenue("nonexistent"), 0.0f), "0 revenue on missing");
    assertTrue(sys.getTotalTransactions("nonexistent") == 0, "0 transactions on missing");
    assertTrue(sys.getServiceCount("nonexistent") == 0, "0 services on missing");
}

void run_station_service_broker_system_tests() {
    testStationServiceBrokerCreate();
    testStationServiceBrokerAddService();
    testStationServiceBrokerRemoveService();
    testStationServiceBrokerServiceCost();
    testStationServiceBrokerTaxAndDiscount();
    testStationServiceBrokerDemand();
    testStationServiceBrokerAvailability();
    testStationServiceBrokerTransaction();
    testStationServiceBrokerMaxServices();
    testStationServiceBrokerTaxClamp();
    testStationServiceBrokerMissing();
}
