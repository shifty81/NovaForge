// Tests for: ShipDesignerSystem Tests
#include "test_log.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "pcg/ship_designer.h"
#include "systems/ship_designer_system.h"

using namespace atlas;

// ==================== ShipDesignerSystem Tests ====================

static void testShipDesignerCreate() {
    std::cout << "\n=== ShipDesigner: Create ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    assertTrue(sys.createDesigner("ship1"), "Create designer succeeds");
    auto* sd = e->getComponent<components::ShipDesigner>();
    assertTrue(sd != nullptr, "Component exists");
    assertTrue(sd->high_slots == 4, "Default high_slots is 4");
    assertTrue(approxEqual(sd->total_cpu, 300.0f), "Default total_cpu is 300");
    assertTrue(!sd->valid, "Not valid by default");
}

static void testShipDesignerBlueprint() {
    std::cout << "\n=== ShipDesigner: Blueprint ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.createDesigner("ship1");
    assertTrue(sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar"), "Set blueprint succeeds");
    auto* sd = e->getComponent<components::ShipDesigner>();
    assertTrue(sd->blueprint_name == "Rifter", "Blueprint name set");
    assertTrue(sd->hull_type == "Frigate", "Hull type set");
}

static void testShipDesignerFitModule() {
    std::cout << "\n=== ShipDesigner: FitModule ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    assertTrue(sys.fitModule("ship1", "Autocannon", 0, 30.0f, 10.0f), "Fit module succeeds");
    assertTrue(sys.getFittedCount("ship1") == 1, "Fitted count is 1");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 30.0f / 300.0f), "CPU usage correct");
}

static void testShipDesignerRemoveModule() {
    std::cout << "\n=== ShipDesigner: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    sys.fitModule("ship1", "Autocannon", 0, 30.0f, 10.0f);
    assertTrue(sys.removeModule("ship1", "Autocannon"), "Remove module succeeds");
    assertTrue(sys.getFittedCount("ship1") == 0, "Fitted count is 0");
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 0.0f), "CPU usage is 0");
}

static void testShipDesignerValidate() {
    std::cout << "\n=== ShipDesigner: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar");
    sys.fitModule("ship1", "Gun", 0, 50.0f, 100.0f);
    assertTrue(sys.validateDesign("ship1"), "Validate succeeds within budget");
    assertTrue(sys.isValid("ship1"), "Design is valid");
    assertTrue(sys.getFittedCount("ship1") == 1, "1 module fitted");
}

static void testShipDesignerOverbudget() {
    std::cout << "\n=== ShipDesigner: Overbudget ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.createDesigner("ship1");
    auto* sd = e->getComponent<components::ShipDesigner>();
    sd->total_cpu = 50.0f;
    sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar");
    sys.fitModule("ship1", "BigGun", 0, 100.0f, 10.0f);
    sys.update(0.0f);
    assertTrue(!sys.validateDesign("ship1"), "Validate fails when over CPU");
    assertTrue(!sys.isValid("ship1"), "Design is not valid");
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 100.0f / 50.0f), "CPU usage ratio > 1");
}

static void testShipDesignerSlotsFull() {
    std::cout << "\n=== ShipDesigner: SlotsFull ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.createDesigner("ship1");
    auto* sd = e->getComponent<components::ShipDesigner>();
    sd->high_slots = 2;
    assertTrue(sys.fitModule("ship1", "G1", 0, 10.0f, 10.0f), "Fit slot 1 succeeds");
    assertTrue(sys.fitModule("ship1", "G2", 0, 10.0f, 10.0f), "Fit slot 2 succeeds");
    assertTrue(!sys.fitModule("ship1", "G3", 0, 10.0f, 10.0f), "Fit slot 3 fails (full)");
    assertTrue(sys.getSlotsFree("ship1", 0) == 0, "0 high slots free");
}

static void testShipDesignerClear() {
    std::cout << "\n=== ShipDesigner: Clear ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar");
    sys.fitModule("ship1", "Gun", 0, 50.0f, 100.0f);
    assertTrue(sys.clearDesign("ship1"), "Clear succeeds");
    assertTrue(sys.getFittedCount("ship1") == 0, "0 modules after clear");
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 0.0f), "CPU usage is 0 after clear");
}

static void testShipDesignerMultipleModules() {
    std::cout << "\n=== ShipDesigner: MultipleModules ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    assertTrue(sys.fitModule("ship1", "Gun", 0, 30.0f, 50.0f), "Fit high slot");
    assertTrue(sys.fitModule("ship1", "Shield", 1, 20.0f, 80.0f), "Fit mid slot");
    assertTrue(sys.fitModule("ship1", "Armor", 2, 15.0f, 40.0f), "Fit low slot");
    assertTrue(sys.getFittedCount("ship1") == 3, "3 modules fitted");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 65.0f / 300.0f), "CPU usage from all modules");
    assertTrue(sys.getSlotsFree("ship1", 0) == 3, "3 high slots free");
}

static void testShipDesignerMissing() {
    std::cout << "\n=== ShipDesigner: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    assertTrue(!sys.createDesigner("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setBlueprint("nonexistent", "R", "F", "M"), "SetBlueprint fails on missing");
    assertTrue(!sys.fitModule("nonexistent", "G", 0, 10.0f, 10.0f), "FitModule fails on missing");
    assertTrue(!sys.removeModule("nonexistent", "G"), "RemoveModule fails on missing");
    assertTrue(!sys.validateDesign("nonexistent"), "Validate fails on missing");
    assertTrue(sys.getFittedCount("nonexistent") == 0, "0 fitted on missing");
    assertTrue(approxEqual(sys.getCpuUsage("nonexistent"), 0.0f), "0 CPU on missing");
    assertTrue(sys.getSlotsFree("nonexistent", 0) == 0, "0 slots free on missing");
    assertTrue(!sys.isValid("nonexistent"), "Not valid on missing");
}


void run_ship_designer_system_tests() {
    testShipDesignerCreate();
    testShipDesignerBlueprint();
    testShipDesignerFitModule();
    testShipDesignerRemoveModule();
    testShipDesignerValidate();
    testShipDesignerOverbudget();
    testShipDesignerSlotsFull();
    testShipDesignerClear();
    testShipDesignerMultipleModules();
    testShipDesignerMissing();
}
