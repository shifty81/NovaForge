// Tests for: Solar Panel System Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/solar_panel_system.h"

using namespace atlas;

// ==================== Solar Panel System Tests ====================

static void testSolarPanelInit() {
    std::cout << "\n=== Solar Panel: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    assertTrue(sys.initializePanels("base1", "owner_001", 4), "Panels initialized");
    assertTrue(sys.getPanelCount("base1") == 4, "4 panels");
    assertTrue(!sys.isDeployed("base1"), "Not deployed initially");
    assertTrue(!sys.initializePanels("base1", "owner_002", 2), "Duplicate init rejected");
}

static void testSolarPanelDeploy() {
    std::cout << "\n=== Solar Panel: Deploy ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);

    assertTrue(sys.deployPanels("base1"), "Deployed");
    assertTrue(sys.isDeployed("base1"), "Is deployed");
}

static void testSolarPanelRetract() {
    std::cout << "\n=== Solar Panel: Retract ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");

    assertTrue(sys.retractPanels("base1"), "Retracted");
    assertTrue(!sys.isDeployed("base1"), "Not deployed after retract");
}

static void testSolarPanelAddRemove() {
    std::cout << "\n=== Solar Panel: Add/Remove ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);

    assertTrue(sys.addPanel("base1"), "Panel added");
    assertTrue(sys.getPanelCount("base1") == 5, "5 panels");
    assertTrue(sys.removePanel("base1"), "Panel removed");
    assertTrue(sys.getPanelCount("base1") == 4, "Back to 4 panels");

    // Fill to max
    for (int i = 0; i < 6; i++) sys.addPanel("base1");
    assertTrue(sys.getPanelCount("base1") == 10, "At max panels");
    assertTrue(!sys.addPanel("base1"), "Cannot exceed max");
}

static void testSolarPanelEnergyOutput() {
    std::cout << "\n=== Solar Panel: Energy Output ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.5f);  // noon

    sys.update(0.1f);
    assertTrue(sys.getEnergyOutput("base1") > 0.0f, "Energy output at noon > 0");
    assertTrue(sys.isDaytime("base1"), "Is daytime at noon");
}

static void testSolarPanelNighttime() {
    std::cout << "\n=== Solar Panel: Nighttime ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.0f);  // midnight

    sys.update(0.001f);  // tiny dt to avoid advancing cycle much
    assertTrue(approxEqual(sys.getEnergyOutput("base1"), 0.0f), "No energy at midnight");
}

static void testSolarPanelDegradation() {
    std::cout << "\n=== Solar Panel: Degradation ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.5f);

    float initial_eff = sys.getEfficiency("base1");
    for (int i = 0; i < 100; i++) {
        sys.setDayCyclePosition("base1", 0.5f);  // keep at noon
        sys.update(1.0f);
    }
    assertTrue(sys.getEfficiency("base1") < initial_eff, "Efficiency decreased over time");
}

static void testSolarPanelMaintenance() {
    std::cout << "\n=== Solar Panel: Maintenance ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");

    // Degrade
    for (int i = 0; i < 50; i++) {
        sys.setDayCyclePosition("base1", 0.5f);
        sys.update(1.0f);
    }
    float degraded = sys.getEfficiency("base1");
    assertTrue(degraded < 1.0f, "Efficiency degraded");

    assertTrue(sys.performMaintenance("base1", 0.5f), "Maintenance performed");
    assertTrue(sys.getEfficiency("base1") > degraded, "Efficiency restored after maintenance");
    assertTrue(sys.getEfficiency("base1") <= 1.0f, "Efficiency clamped to 1.0");
}

static void testSolarPanelEnergyStorage() {
    std::cout << "\n=== Solar Panel: Energy Storage ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.5f);

    assertTrue(approxEqual(sys.getEnergyStored("base1"), 0.0f), "No stored energy initially");
    sys.update(1.0f);
    assertTrue(sys.getEnergyStored("base1") > 0.0f, "Energy stored after update");
}

static void testSolarPanelMissing() {
    std::cout << "\n=== Solar Panel: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SolarPanelSystem sys(&world);
    assertTrue(!sys.initializePanels("nonexistent", "owner", 4), "Init fails on missing");
    assertTrue(!sys.deployPanels("nonexistent"), "Deploy fails on missing");
    assertTrue(sys.getPanelCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(approxEqual(sys.getEnergyOutput("nonexistent"), 0.0f), "Output 0 on missing");
    assertTrue(!sys.isDeployed("nonexistent"), "Not deployed on missing");
}


void run_solar_panel_system_tests() {
    testSolarPanelInit();
    testSolarPanelDeploy();
    testSolarPanelRetract();
    testSolarPanelAddRemove();
    testSolarPanelEnergyOutput();
    testSolarPanelNighttime();
    testSolarPanelDegradation();
    testSolarPanelMaintenance();
    testSolarPanelEnergyStorage();
    testSolarPanelMissing();
}
