// Tests for: Grid Construction System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "pcg/habitat_generator.h"
#include "systems/grid_construction_system.h"

using namespace atlas;

// ==================== Grid Construction System Tests ====================

static void testGridInit() {
    std::cout << "\n=== Grid Construction: Init ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    assertTrue(sys.initializeGrid("test_grid", "player1", 10, 10), "Grid initialized");
    assertTrue(sys.getGridWidth("test_grid") == 10, "Width is 10");
    assertTrue(sys.getGridHeight("test_grid") == 10, "Height is 10");
    assertTrue(!sys.initializeGrid("test_grid", "player1", 8, 8), "Duplicate init fails");
}

static void testGridPlace() {
    std::cout << "\n=== Grid Construction: Place ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    assertTrue(sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Foundation), "Place foundation");
    assertTrue(sys.getModuleAt("test_grid", 0, 0) == "foundation", "Module is foundation");
    assertTrue(sys.getModuleCount("test_grid") == 1, "Module count is 1");
    assertTrue(!sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Wall), "Cannot place on occupied");
}

static void testGridRemove() {
    std::cout << "\n=== Grid Construction: Remove ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 2, 2, components::GridConstruction::ModuleType::Wall);
    assertTrue(sys.removeModule("test_grid", 2, 2), "Remove module");
    assertTrue(sys.getModuleAt("test_grid", 2, 2) == "empty", "Cell empty after remove");
    assertTrue(!sys.removeModule("test_grid", 2, 2), "Cannot remove empty cell");
}

static void testGridAdjacency() {
    std::cout << "\n=== Grid Construction: Adjacency ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 1, 1, components::GridConstruction::ModuleType::Foundation);
    sys.placeModule("test_grid", 0, 1, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 2, 1, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 1, 0, components::GridConstruction::ModuleType::Floor);
    sys.placeModule("test_grid", 1, 2, components::GridConstruction::ModuleType::Floor);
    float integrity = sys.calculateIntegrity("test_grid");
    assertTrue(integrity > 0.0f, "Integrity > 0 with modules");
}

static void testGridPower() {
    std::cout << "\n=== Grid Construction: Power ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 1, 1, components::GridConstruction::ModuleType::PowerNode);
    sys.placeModule("test_grid", 1, 0, components::GridConstruction::ModuleType::HabitatModule);
    float balance = sys.calculatePower("test_grid");
    assertTrue(approxEqual(balance, 8.0f), "Power balance 10 - 2 = 8");
    sys.update(1.0f);
    assertTrue(sys.getPoweredCount("test_grid") == 2, "2 cells powered");
}

static void testGridIntegrity() {
    std::cout << "\n=== Grid Construction: Integrity ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    // Place a center module with 3+ neighbors for bonus
    sys.placeModule("test_grid", 3, 3, components::GridConstruction::ModuleType::Foundation);
    sys.placeModule("test_grid", 2, 3, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 4, 3, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 3, 2, components::GridConstruction::ModuleType::Floor);
    float integrity = sys.calculateIntegrity("test_grid");
    assertTrue(integrity > 0.0f, "Integrity calculated");
    assertTrue(integrity <= 1.0f, "Integrity within bounds");
}

static void testGridDamage() {
    std::cout << "\n=== Grid Construction: Damage ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Wall);
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 1.0f), "Initial health 1.0");
    assertTrue(sys.damageModule("test_grid", 0, 0, 0.3f), "Damage applied");
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 0.7f), "Health reduced to 0.7");
}

static void testGridRepair() {
    std::cout << "\n=== Grid Construction: Repair ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Wall);
    sys.damageModule("test_grid", 0, 0, 0.5f);
    assertTrue(sys.repairModule("test_grid", 0, 0, 0.3f), "Repair applied");
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 0.8f), "Health restored to 0.8");
    sys.repairModule("test_grid", 0, 0, 1.0f);
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 1.0f), "Health capped at 1.0");
}

static void testGridBounds() {
    std::cout << "\n=== Grid Construction: Bounds ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 4, 4);
    assertTrue(!sys.placeModule("test_grid", -1, 0, components::GridConstruction::ModuleType::Wall), "Negative x rejected");
    assertTrue(!sys.placeModule("test_grid", 0, -1, components::GridConstruction::ModuleType::Wall), "Negative y rejected");
    assertTrue(!sys.placeModule("test_grid", 4, 0, components::GridConstruction::ModuleType::Wall), "X out of bounds rejected");
    assertTrue(!sys.placeModule("test_grid", 0, 4, components::GridConstruction::ModuleType::Wall), "Y out of bounds rejected");
    assertTrue(sys.getModuleAt("test_grid", 10, 10) == "unknown", "Out of bounds query returns unknown");
}

static void testGridMissing() {
    std::cout << "\n=== Grid Construction: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    assertTrue(!sys.initializeGrid("nonexistent", "player1", 8, 8), "Init fails on missing");
    assertTrue(!sys.placeModule("nonexistent", 0, 0, components::GridConstruction::ModuleType::Wall), "Place fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getGridWidth("nonexistent") == 0, "Width 0 on missing");
}


void run_grid_construction_system_tests() {
    testGridInit();
    testGridPlace();
    testGridRemove();
    testGridAdjacency();
    testGridPower();
    testGridIntegrity();
    testGridDamage();
    testGridRepair();
    testGridBounds();
    testGridMissing();
}
