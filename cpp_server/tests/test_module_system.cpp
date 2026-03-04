// Tests for: Module System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/module_system.h"

using namespace atlas;

// ==================== Module System Tests ====================

static void testModuleActivation() {
    std::cout << "\n=== Module Activation ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    // Add a module to high slot
    components::ModuleRack::FittedModule gun;
    gun.module_id = "gun_001";
    gun.name = "125mm Autocannon";
    gun.slot_type = "high";
    gun.slot_index = 0;
    gun.cycle_time = 5.0f;
    gun.capacitor_cost = 10.0f;
    rack->high_slots.push_back(gun);

    // Activate
    bool activated = modSys.activateModule("ship1", "high", 0);
    assertTrue(activated, "Module activated");
    assertTrue(rack->high_slots[0].active, "Module is active");

    // Can't activate again
    bool double_activate = modSys.activateModule("ship1", "high", 0);
    assertTrue(!double_activate, "Can't activate already active module");
}

static void testModuleCycling() {
    std::cout << "\n=== Module Cycling ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    components::ModuleRack::FittedModule repper;
    repper.module_id = "rep_001";
    repper.name = "Small Armor Repairer";
    repper.slot_type = "low";
    repper.slot_index = 0;
    repper.cycle_time = 4.0f;
    repper.capacitor_cost = 20.0f;
    rack->low_slots.push_back(repper);

    modSys.activateModule("ship1", "low", 0);

    // Partially cycle
    modSys.update(2.0f);
    assertTrue(approxEqual(rack->low_slots[0].cycle_progress, 0.5f),
               "Half cycle after 2s (4s cycle time)");

    // Complete cycle — should consume cap
    modSys.update(3.0f);
    assertTrue(approxEqual(cap->capacitor, 80.0f, 1.0f),
               "Capacitor consumed after cycle completion");
}

static void testModuleCapDrain() {
    std::cout << "\n=== Module Capacitor Drain ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 15.0f;  // Just enough for one cycle
    cap->capacitor_max = 100.0f;

    components::ModuleRack::FittedModule mod;
    mod.cycle_time = 1.0f;
    mod.capacitor_cost = 10.0f;
    rack->high_slots.push_back(mod);

    modSys.activateModule("ship1", "high", 0);

    // First cycle completes
    modSys.update(1.5f);
    assertTrue(rack->high_slots[0].active, "Module still active after first cycle");

    // Second cycle — not enough cap
    modSys.update(1.5f);
    assertTrue(!rack->high_slots[0].active,
               "Module deactivated when capacitor exhausted");
}

static void testModuleFittingValidation() {
    std::cout << "\n=== Module Fitting Validation ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->cpu_max = 100.0f;
    shipComp->powergrid_max = 50.0f;
    auto* rack = addComp<components::ModuleRack>(ship);

    // Fit a module within limits
    components::ModuleRack::FittedModule mod1;
    mod1.cpu_usage = 30.0f;
    mod1.powergrid_usage = 20.0f;
    rack->high_slots.push_back(mod1);

    assertTrue(modSys.validateFitting("ship1"), "Fitting within limits");

    // Exceed CPU
    components::ModuleRack::FittedModule mod2;
    mod2.cpu_usage = 80.0f;
    mod2.powergrid_usage = 10.0f;
    rack->mid_slots.push_back(mod2);

    assertTrue(!modSys.validateFitting("ship1"), "Fitting exceeds CPU");
}

static void testModuleToggle() {
    std::cout << "\n=== Module Toggle ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;

    components::ModuleRack::FittedModule mod;
    mod.capacitor_cost = 5.0f;
    rack->mid_slots.push_back(mod);

    // Toggle on
    modSys.toggleModule("ship1", "mid", 0);
    assertTrue(rack->mid_slots[0].active, "Module toggled on");

    // Toggle off
    modSys.toggleModule("ship1", "mid", 0);
    assertTrue(!rack->mid_slots[0].active, "Module toggled off");
}


void run_module_system_tests() {
    testModuleActivation();
    testModuleCycling();
    testModuleCapDrain();
    testModuleFittingValidation();
    testModuleToggle();
}
