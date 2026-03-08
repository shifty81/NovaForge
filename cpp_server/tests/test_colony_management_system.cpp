// Tests for: Colony Management System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/colony_management_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Colony Management System Tests ====================

static void testColonyManagementCreate() {
    std::cout << "\n=== ColonyManagement: Create ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    assertTrue(sys.initialize("col1", "Alpha Colony", "planet_3"), "Init succeeds");
    assertTrue(sys.getColonyName("col1") == "Alpha Colony", "Colony name matches");
    assertTrue(sys.getBuildingCount("col1") == 0, "0 buildings initially");
    assertTrue(approxEqual(sys.getTotalExports("col1"), 0.0f), "0 exports initially");
    assertTrue(approxEqual(sys.getTotalExportValue("col1"), 0.0f), "0 export value initially");
    assertTrue(sys.getTotalProductionCycles("col1") == 0, "0 production cycles initially");
}

static void testColonyManagementInitValidation() {
    std::cout << "\n=== ColonyManagement: InitValidation ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    assertTrue(!sys.initialize("col1", "", "planet_3"), "Empty colony name rejected");
    world.createEntity("col2");
    assertTrue(!sys.initialize("col2", "Colony", ""), "Empty planet_id rejected");
    assertTrue(!sys.initialize("nonexistent", "Colony", "planet_1"), "Missing entity rejected");
}

static void testColonyManagementAddBuilding() {
    std::cout << "\n=== ColonyManagement: AddBuilding ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    assertTrue(sys.addBuilding("col1", "b1", "extractor", 1.0f, 10.0f), "Add extractor");
    assertTrue(sys.addBuilding("col1", "b2", "processor", 0.5f, 15.0f), "Add processor");
    assertTrue(sys.getBuildingCount("col1") == 2, "2 buildings");
    assertTrue(sys.getOnlineBuildingCount("col1") == 2, "2 online");
    assertTrue(approxEqual(sys.getPowerUsed("col1"), 25.0f), "25W used");
}

static void testColonyManagementBuildingValidation() {
    std::cout << "\n=== ColonyManagement: BuildingValidation ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    assertTrue(!sys.addBuilding("col1", "", "extractor", 1.0f, 10.0f), "Empty building_id rejected");
    assertTrue(!sys.addBuilding("col1", "b1", "", 1.0f, 10.0f), "Empty type rejected");
}

static void testColonyManagementDuplicateBuilding() {
    std::cout << "\n=== ColonyManagement: DuplicateBuilding ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    assertTrue(sys.addBuilding("col1", "b1", "extractor", 1.0f, 10.0f), "First building");
    assertTrue(!sys.addBuilding("col1", "b1", "processor", 0.5f, 5.0f), "Duplicate rejected");
}

static void testColonyManagementRemoveBuilding() {
    std::cout << "\n=== ColonyManagement: RemoveBuilding ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    sys.addBuilding("col1", "b1", "extractor", 1.0f, 10.0f);
    sys.addBuilding("col1", "b2", "processor", 0.5f, 15.0f);
    assertTrue(sys.removeBuilding("col1", "b1"), "Remove b1");
    assertTrue(sys.getBuildingCount("col1") == 1, "1 building left");
    assertTrue(!sys.removeBuilding("col1", "nonexistent"), "Cannot remove nonexistent");
}

static void testColonyManagementToggleBuilding() {
    std::cout << "\n=== ColonyManagement: ToggleBuilding ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    sys.addBuilding("col1", "b1", "extractor", 1.0f, 10.0f);
    assertTrue(sys.getOnlineBuildingCount("col1") == 1, "1 online");
    assertTrue(sys.toggleBuilding("col1", "b1"), "Toggle off");
    assertTrue(sys.getOnlineBuildingCount("col1") == 0, "0 online");
    assertTrue(sys.toggleBuilding("col1", "b1"), "Toggle on");
    assertTrue(sys.getOnlineBuildingCount("col1") == 1, "1 online again");
    assertTrue(!sys.toggleBuilding("col1", "nonexistent"), "Toggle nonexistent fails");
}

static void testColonyManagementPowerBudget() {
    std::cout << "\n=== ColonyManagement: PowerBudget ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    sys.setPowerCapacity("col1", 50.0f);
    assertTrue(sys.addBuilding("col1", "b1", "extractor", 1.0f, 30.0f), "Add 30W building");
    assertTrue(sys.addBuilding("col1", "b2", "processor", 1.0f, 15.0f), "Add 15W building");
    assertTrue(!sys.addBuilding("col1", "b3", "storage", 1.0f, 10.0f), "10W exceeds 50W capacity");
    assertTrue(approxEqual(sys.getRemainingPower("col1"), 5.0f), "5W remaining");
}

static void testColonyManagementGoods() {
    std::cout << "\n=== ColonyManagement: Goods ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    assertTrue(sys.addGoods("col1", "Tritanium", 500.0f, 1000.0f), "Add Tritanium");
    assertTrue(approxEqual(sys.getGoodsQuantity("col1", "Tritanium"), 500.0f), "500 Tritanium");
    // Add more of same type (stacks)
    assertTrue(sys.addGoods("col1", "Tritanium", 300.0f, 1000.0f), "Add more Tritanium");
    assertTrue(approxEqual(sys.getGoodsQuantity("col1", "Tritanium"), 800.0f), "800 Tritanium stacked");
    // Cannot exceed max
    sys.addGoods("col1", "Tritanium", 500.0f, 1000.0f);
    assertTrue(approxEqual(sys.getGoodsQuantity("col1", "Tritanium"), 1000.0f), "Capped at 1000");
}

static void testColonyManagementExport() {
    std::cout << "\n=== ColonyManagement: Export ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    sys.addGoods("col1", "Tritanium", 500.0f, 1000.0f);
    assertTrue(sys.exportGoods("col1", "Tritanium", 200.0f, 10.0f), "Export 200 @ 10 ISC");
    assertTrue(approxEqual(sys.getGoodsQuantity("col1", "Tritanium"), 300.0f), "300 left");
    assertTrue(approxEqual(sys.getTotalExports("col1"), 200.0f), "200 total exports");
    assertTrue(approxEqual(sys.getTotalExportValue("col1"), 2000.0f), "2000 ISC value");
    assertTrue(!sys.exportGoods("col1", "Tritanium", 500.0f, 10.0f), "Insufficient goods");
    assertTrue(!sys.exportGoods("col1", "Nonexistent", 10.0f, 10.0f), "Unknown good type");
    assertTrue(!sys.exportGoods("col1", "Tritanium", 0.0f, 10.0f), "Zero quantity rejected");
    assertTrue(!sys.exportGoods("col1", "Tritanium", 10.0f, 0.0f), "Zero price rejected");
}

static void testColonyManagementMaxBuildings() {
    std::cout << "\n=== ColonyManagement: MaxBuildings ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    sys.setPowerCapacity("col1", 10000.0f);
    for (int i = 0; i < 15; ++i) {
        assertTrue(sys.addBuilding("col1", "b_" + std::to_string(i), "extractor", 1.0f, 1.0f),
                   ("Add building " + std::to_string(i)).c_str());
    }
    assertTrue(!sys.addBuilding("col1", "b_overflow", "extractor", 1.0f, 1.0f),
               "16th building rejected");
    assertTrue(sys.getBuildingCount("col1") == 15, "15 buildings max");
}

static void testColonyManagementProductionCycles() {
    std::cout << "\n=== ColonyManagement: ProductionCycles ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    world.createEntity("col1");
    sys.initialize("col1", "Alpha", "planet_1");
    sys.addBuilding("col1", "b1", "extractor", 1.0f, 10.0f);
    sys.update(1.0f);
    assertTrue(sys.getTotalProductionCycles("col1") >= 1, "Production cycle counted");
}

static void testColonyManagementMissing() {
    std::cout << "\n=== ColonyManagement: Missing ===" << std::endl;
    ecs::World world;
    systems::ColonyManagementSystem sys(&world);
    assertTrue(!sys.addBuilding("nonexistent", "b1", "ext", 1.0f, 10.0f), "Add fails on missing");
    assertTrue(!sys.removeBuilding("nonexistent", "b1"), "Remove fails on missing");
    assertTrue(!sys.toggleBuilding("nonexistent", "b1"), "Toggle fails on missing");
    assertTrue(!sys.addGoods("nonexistent", "ore", 10.0f, 100.0f), "Add goods fails on missing");
    assertTrue(!sys.exportGoods("nonexistent", "ore", 5.0f, 10.0f), "Export fails on missing");
    assertTrue(!sys.setPowerCapacity("nonexistent", 100.0f), "Power fails on missing");
    assertTrue(sys.getBuildingCount("nonexistent") == 0, "0 buildings on missing");
    assertTrue(sys.getOnlineBuildingCount("nonexistent") == 0, "0 online on missing");
    assertTrue(approxEqual(sys.getPowerUsed("nonexistent"), 0.0f), "0 power on missing");
    assertTrue(approxEqual(sys.getRemainingPower("nonexistent"), 0.0f), "0 remaining on missing");
    assertTrue(approxEqual(sys.getGoodsQuantity("nonexistent", "ore"), 0.0f), "0 goods on missing");
    assertTrue(approxEqual(sys.getTotalExports("nonexistent"), 0.0f), "0 exports on missing");
    assertTrue(approxEqual(sys.getTotalExportValue("nonexistent"), 0.0f), "0 value on missing");
    assertTrue(sys.getTotalProductionCycles("nonexistent") == 0, "0 cycles on missing");
    assertTrue(sys.getColonyName("nonexistent").empty(), "Empty name on missing");
}

void run_colony_management_system_tests() {
    testColonyManagementCreate();
    testColonyManagementInitValidation();
    testColonyManagementAddBuilding();
    testColonyManagementBuildingValidation();
    testColonyManagementDuplicateBuilding();
    testColonyManagementRemoveBuilding();
    testColonyManagementToggleBuilding();
    testColonyManagementPowerBudget();
    testColonyManagementGoods();
    testColonyManagementExport();
    testColonyManagementMaxBuildings();
    testColonyManagementProductionCycles();
    testColonyManagementMissing();
}
