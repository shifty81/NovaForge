// Tests for: ThermalManagement System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/thermal_management_system.h"

using namespace atlas;

// ==================== ThermalManagement System Tests ====================

static void testThermalCreate() {
    std::cout << "\n=== ThermalManagement: Create ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeThermal("ship1", 100.0f, 5.0f), "Init thermal succeeds");
    assertTrue(approxEqual(sys.getCurrentHeat("ship1"), 0.0f), "0 heat initially");
    assertTrue(approxEqual(sys.getHeatFraction("ship1"), 0.0f), "0 fraction initially");
    assertTrue(!sys.isOverheated("ship1"), "Not overheated initially");
    assertTrue(!sys.isWarning("ship1"), "No warning initially");
}

static void testThermalAddHeat() {
    std::cout << "\n=== ThermalManagement: AddHeat ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 5.0f);

    assertTrue(sys.addHeat("ship1", 30.0f), "Add 30 heat");
    assertTrue(approxEqual(sys.getCurrentHeat("ship1"), 30.0f), "Heat is 30");
    assertTrue(approxEqual(sys.getHeatFraction("ship1"), 0.3f), "Fraction is 0.3");
    assertTrue(!sys.isWarning("ship1"), "Not warning at 30%");
}

static void testThermalWarning() {
    std::cout << "\n=== ThermalManagement: Warning ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 5.0f);

    sys.addHeat("ship1", 76.0f);
    assertTrue(sys.isWarning("ship1"), "Warning at 76%");
    assertTrue(!sys.isOverheated("ship1"), "Not overheated at 76%");
}

static void testThermalOverheat() {
    std::cout << "\n=== ThermalManagement: Overheat ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 5.0f);

    sys.addHeat("ship1", 100.0f);
    assertTrue(sys.isOverheated("ship1"), "Overheated at 100%");
    assertTrue(sys.getTotalOverheatEvents("ship1") == 1, "1 overheat event");
    assertTrue(approxEqual(sys.getHeatFraction("ship1"), 1.0f), "Fraction is 1.0");
}

static void testThermalCap() {
    std::cout << "\n=== ThermalManagement: HeatCap ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 5.0f);

    sys.addHeat("ship1", 150.0f);
    assertTrue(approxEqual(sys.getCurrentHeat("ship1"), 100.0f), "Capped at max_heat");
    assertTrue(approxEqual(sys.getTotalHeatGenerated("ship1"), 150.0f), "Total generated 150");
}

static void testThermalDissipation() {
    std::cout << "\n=== ThermalManagement: Dissipation ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 10.0f);  // 10 heat/sec dissipation

    sys.addHeat("ship1", 50.0f);
    // 1 second of dissipation at 10/sec = -10 heat
    for (int i = 0; i < 10; ++i) sys.update(0.1f);

    assertTrue(approxEqual(sys.getCurrentHeat("ship1"), 40.0f), "50 - 10 = 40 after 1s");
    assertTrue(sys.getTotalHeatDissipated("ship1") > 9.0f, "Dissipated ~10 heat");
}

static void testThermalFullDissipation() {
    std::cout << "\n=== ThermalManagement: FullDissipation ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 50.0f);  // fast dissipation

    sys.addHeat("ship1", 20.0f);
    for (int i = 0; i < 10; ++i) sys.update(0.1f);  // 1 second at 50/sec

    assertTrue(approxEqual(sys.getCurrentHeat("ship1"), 0.0f), "Fully dissipated");
    assertTrue(!sys.isOverheated("ship1"), "Not overheated after cooling");
}

static void testThermalSetDissipation() {
    std::cout << "\n=== ThermalManagement: SetDissipation ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 5.0f);

    assertTrue(sys.setDissipationRate("ship1", 20.0f), "Set dissipation to 20");
    sys.addHeat("ship1", 50.0f);
    for (int i = 0; i < 10; ++i) sys.update(0.1f);  // 1 second at 20/sec

    assertTrue(approxEqual(sys.getCurrentHeat("ship1"), 30.0f), "50 - 20 = 30");
}

static void testThermalDuplicateInit() {
    std::cout << "\n=== ThermalManagement: DuplicateInit ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeThermal("ship1"), "First init succeeds");
    assertTrue(!sys.initializeThermal("ship1"), "Duplicate init rejected");
}

static void testThermalMissing() {
    std::cout << "\n=== ThermalManagement: Missing ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    assertTrue(!sys.initializeThermal("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.addHeat("nonexistent", 10.0f), "Add heat fails on missing");
    assertTrue(approxEqual(sys.getCurrentHeat("nonexistent"), 0.0f), "0 heat on missing");
    assertTrue(approxEqual(sys.getHeatFraction("nonexistent"), 0.0f), "0 fraction on missing");
    assertTrue(!sys.isOverheated("nonexistent"), "Not overheated on missing");
    assertTrue(!sys.isWarning("nonexistent"), "No warning on missing");
    assertTrue(sys.getTotalOverheatEvents("nonexistent") == 0, "0 events on missing");
}

static void testThermalInvalidHeat() {
    std::cout << "\n=== ThermalManagement: InvalidHeat ===" << std::endl;
    ecs::World world;
    systems::ThermalManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeThermal("ship1", 100.0f, 5.0f);

    assertTrue(!sys.addHeat("ship1", 0.0f), "Zero heat rejected");
    assertTrue(!sys.addHeat("ship1", -10.0f), "Negative heat rejected");
    assertTrue(!sys.setDissipationRate("ship1", -1.0f), "Negative dissipation rejected");
}

void run_thermal_management_system_tests() {
    testThermalCreate();
    testThermalAddHeat();
    testThermalWarning();
    testThermalOverheat();
    testThermalCap();
    testThermalDissipation();
    testThermalFullDissipation();
    testThermalSetDissipation();
    testThermalDuplicateInit();
    testThermalMissing();
    testThermalInvalidHeat();
}
