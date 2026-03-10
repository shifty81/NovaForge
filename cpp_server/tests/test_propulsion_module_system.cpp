// Tests for: PropulsionModuleSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/propulsion_module_system.h"

using namespace atlas;

// ==================== PropulsionModuleSystem Tests ====================

static void testPropulsionInit() {
    std::cout << "\n=== Propulsion: Init AB ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    assertTrue(sys.initialize("s1", components::PropulsionModuleState::ModuleType::Afterburner),
               "Init AB succeeds");
    assertTrue(!sys.isActive("s1"), "Not active initially");
    assertTrue(approxEqual(sys.getSpeedMultiplier("s1"), 1.5f), "AB speed multiplier = 1.5");
    assertTrue(approxEqual(sys.getSignatureBloom("s1"), 1.0f), "AB no signature bloom");
    assertTrue(sys.getTotalCycles("s1") == 0, "Zero cycles initially");
    assertTrue(approxEqual(sys.getActiveDuration("s1"), 0.0f), "Zero active duration");
    assertTrue(approxEqual(sys.getEffectiveSpeedMultiplier("s1"), 1.0f),
               "Effective speed 1.0 when inactive");
    assertTrue(approxEqual(sys.getEffectiveSignatureBloom("s1"), 1.0f),
               "Effective bloom 1.0 when inactive");
}

static void testPropulsionInitMWD() {
    std::cout << "\n=== Propulsion: Init MWD ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    assertTrue(sys.initialize("s1", components::PropulsionModuleState::ModuleType::MicrowarpDrive),
               "Init MWD succeeds");
    assertTrue(approxEqual(sys.getSpeedMultiplier("s1"), 5.0f), "MWD speed multiplier = 5.0");
    assertTrue(approxEqual(sys.getSignatureBloom("s1"), 5.0f), "MWD signature bloom = 5.0");
}

static void testPropulsionInitFails() {
    std::cout << "\n=== Propulsion: InitFails ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", components::PropulsionModuleState::ModuleType::Afterburner),
               "Init fails on missing entity");
}

static void testPropulsionActivateDeactivate() {
    std::cout << "\n=== Propulsion: ActivateDeactivate ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", components::PropulsionModuleState::ModuleType::Afterburner);

    assertTrue(sys.activateModule("s1"), "Activate succeeds");
    assertTrue(sys.isActive("s1"), "Module is active");
    assertTrue(approxEqual(sys.getEffectiveSpeedMultiplier("s1"), 1.5f),
               "Effective speed 1.5 when active");
    assertTrue(!sys.activateModule("s1"), "Cannot double-activate");

    assertTrue(sys.deactivateModule("s1"), "Deactivate succeeds");
    assertTrue(!sys.isActive("s1"), "Module is inactive");
    assertTrue(!sys.deactivateModule("s1"), "Cannot double-deactivate");
    assertTrue(approxEqual(sys.getEffectiveSpeedMultiplier("s1"), 1.0f),
               "Effective speed 1.0 when inactive");
}

static void testPropulsionMWDBloom() {
    std::cout << "\n=== Propulsion: MWDBloom ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", components::PropulsionModuleState::ModuleType::MicrowarpDrive);

    assertTrue(approxEqual(sys.getEffectiveSignatureBloom("s1"), 1.0f),
               "No bloom when inactive");
    sys.activateModule("s1");
    assertTrue(approxEqual(sys.getEffectiveSignatureBloom("s1"), 5.0f),
               "Full bloom when MWD active");
    sys.deactivateModule("s1");
    assertTrue(approxEqual(sys.getEffectiveSignatureBloom("s1"), 1.0f),
               "Bloom returns to 1.0 when deactivated");
}

static void testPropulsionCycling() {
    std::cout << "\n=== Propulsion: Cycling ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", components::PropulsionModuleState::ModuleType::Afterburner);
    // Default: cycle_time=5s, cap_drain=10, capacitor=100
    sys.activateModule("s1");

    // Tick 5 seconds — should complete 1 cycle
    sys.update(5.0f);
    assertTrue(sys.getTotalCycles("s1") == 1, "One cycle completed after 5s");
    assertTrue(approxEqual(sys.getCapacitorRemaining("s1"), 90.0f), "Capacitor drained by 10");
    assertTrue(sys.isActive("s1"), "Still active after 1 cycle");
    assertTrue(approxEqual(sys.getActiveDuration("s1"), 5.0f), "Active duration = 5s");

    // Tick another 10 seconds — 2 more cycles
    sys.update(10.0f);
    assertTrue(sys.getTotalCycles("s1") == 3, "Three cycles total");
    assertTrue(approxEqual(sys.getCapacitorRemaining("s1"), 70.0f), "Capacitor at 70");
}

static void testPropulsionCapDepletion() {
    std::cout << "\n=== Propulsion: CapDepletion ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", components::PropulsionModuleState::ModuleType::Afterburner);

    // Set low capacitor to deplete quickly
    sys.setCapacitor("s1", 25.0f);
    sys.activateModule("s1");

    // Tick 10 seconds — 2 cycles (drain 20), cap = 5 after
    sys.update(10.0f);
    assertTrue(sys.isActive("s1"), "Still active at cap=5");
    assertTrue(sys.getTotalCycles("s1") == 2, "Two cycles completed");

    // Tick 5 more seconds — 3rd cycle drains 10, cap goes to -5 → clamped to 0, deactivated
    sys.update(5.0f);
    assertTrue(!sys.isActive("s1"), "Deactivated when cap depleted");
    assertTrue(approxEqual(sys.getCapacitorRemaining("s1"), 0.0f), "Capacitor at zero");
    assertTrue(sys.getTotalCycles("s1") == 3, "Three cycles completed before deactivation");
}

static void testPropulsionActivateEmptyCap() {
    std::cout << "\n=== Propulsion: ActivateEmptyCap ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", components::PropulsionModuleState::ModuleType::Afterburner);
    sys.setCapacitor("s1", 0.0f);

    assertTrue(!sys.activateModule("s1"), "Cannot activate with zero capacitor");
    assertTrue(!sys.isActive("s1"), "Still inactive");
}

static void testPropulsionConfiguration() {
    std::cout << "\n=== Propulsion: Configuration ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", components::PropulsionModuleState::ModuleType::Afterburner);

    assertTrue(sys.setSpeedMultiplier("s1", 2.0f), "Set speed multiplier");
    assertTrue(approxEqual(sys.getSpeedMultiplier("s1"), 2.0f), "Speed multiplier updated");
    assertTrue(!sys.setSpeedMultiplier("s1", 0.0f), "Zero multiplier rejected");
    assertTrue(!sys.setSpeedMultiplier("s1", -1.0f), "Negative multiplier rejected");

    assertTrue(sys.setSignatureBloom("s1", 3.0f), "Set bloom");
    assertTrue(approxEqual(sys.getSignatureBloom("s1"), 3.0f), "Bloom updated");
    assertTrue(!sys.setSignatureBloom("s1", 0.5f), "Bloom below 1.0 rejected");

    assertTrue(sys.setCapDrainPerCycle("s1", 20.0f), "Set cap drain");
    assertTrue(!sys.setCapDrainPerCycle("s1", 0.0f), "Zero drain rejected");

    assertTrue(sys.setCycleTime("s1", 3.0f), "Set cycle time");
    assertTrue(!sys.setCycleTime("s1", 0.0f), "Zero cycle time rejected");

    assertTrue(sys.setCapacitor("s1", 200.0f), "Set capacitor");
    assertTrue(!sys.setCapacitor("s1", -1.0f), "Negative capacitor rejected");
}

static void testPropulsionMissing() {
    std::cout << "\n=== Propulsion: Missing ===" << std::endl;
    ecs::World world;
    systems::PropulsionModuleSystem sys(&world);

    assertTrue(!sys.activateModule("nonexistent"), "Activate fails on missing");
    assertTrue(!sys.deactivateModule("nonexistent"), "Deactivate fails on missing");
    assertTrue(!sys.setSpeedMultiplier("nonexistent", 2.0f), "SetSpeed fails on missing");
    assertTrue(!sys.setSignatureBloom("nonexistent", 2.0f), "SetBloom fails on missing");
    assertTrue(!sys.setCapDrainPerCycle("nonexistent", 10.0f), "SetDrain fails on missing");
    assertTrue(!sys.setCycleTime("nonexistent", 3.0f), "SetCycle fails on missing");
    assertTrue(!sys.setCapacitor("nonexistent", 100.0f), "SetCap fails on missing");
    assertTrue(approxEqual(sys.getSpeedMultiplier("nonexistent"), 1.0f), "Default speed on missing");
    assertTrue(approxEqual(sys.getSignatureBloom("nonexistent"), 1.0f), "Default bloom on missing");
    assertTrue(approxEqual(sys.getEffectiveSpeedMultiplier("nonexistent"), 1.0f), "Default eff speed");
    assertTrue(approxEqual(sys.getEffectiveSignatureBloom("nonexistent"), 1.0f), "Default eff bloom");
    assertTrue(!sys.isActive("nonexistent"), "Not active on missing");
    assertTrue(sys.getTotalCycles("nonexistent") == 0, "Zero cycles on missing");
    assertTrue(approxEqual(sys.getActiveDuration("nonexistent"), 0.0f), "Zero duration on missing");
    assertTrue(approxEqual(sys.getCapacitorRemaining("nonexistent"), 0.0f), "Zero cap on missing");
}

void run_propulsion_module_system_tests() {
    testPropulsionInit();
    testPropulsionInitMWD();
    testPropulsionInitFails();
    testPropulsionActivateDeactivate();
    testPropulsionMWDBloom();
    testPropulsionCycling();
    testPropulsionCapDepletion();
    testPropulsionActivateEmptyCap();
    testPropulsionConfiguration();
    testPropulsionMissing();
}
