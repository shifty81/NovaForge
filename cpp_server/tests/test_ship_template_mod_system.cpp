// Tests for: ShipTemplateModSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/ship_template_mod_system.h"

using namespace atlas;

// ==================== ShipTemplateModSystem Tests ====================

static void testShipTemplateRegister() {
    std::cout << "\n=== ShipTemplateMod: Register ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* e = world.createEntity("tmpl1");
    assertTrue(sys.registerTemplate("tmpl1", "rifter_v2", "Rifter Mk II", "frigate", "minmatar"), "Register succeeds");
    auto* mod = e->getComponent<components::ShipTemplateMod>();
    assertTrue(mod != nullptr, "Component exists");
    assertTrue(mod->template_id == "rifter_v2", "Template ID set");
    assertTrue(mod->ship_class == "frigate", "Ship class set");
}

static void testShipTemplateValidate() {
    std::cout << "\n=== ShipTemplateMod: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "Ship", "cruiser", "amarr");
    assertTrue(sys.validateTemplate("tmpl1"), "Validation passes");
    assertTrue(sys.isValid("tmpl1"), "Is valid after validation");
}

static void testShipTemplateValidateFail() {
    std::cout << "\n=== ShipTemplateMod: Validate Fail ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* e = world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "", "cruiser", "amarr");
    assertTrue(!sys.validateTemplate("tmpl1"), "Validation fails with empty name");
    assertTrue(!sys.isValid("tmpl1"), "Not valid");
}

static void testShipTemplateModSource() {
    std::cout << "\n=== ShipTemplateMod: Mod Source ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* e = world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "Ship", "frigate", "gallente");
    assertTrue(sys.setModSource("tmpl1", "my_mod", 10), "Set mod source");
    auto* mod = e->getComponent<components::ShipTemplateMod>();
    assertTrue(mod->mod_source == "my_mod", "Mod source set");
    assertTrue(mod->priority == 10, "Priority set");
}

static void testShipTemplateBaseInherit() {
    std::cout << "\n=== ShipTemplateMod: Base Template Inheritance ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* base = world.createEntity("base1");
    sys.registerTemplate("base1", "base_frigate", "Base Frigate", "frigate", "minmatar");
    auto* baseMod = base->getComponent<components::ShipTemplateMod>();
    baseMod->hull_hp = 500.0f;
    baseMod->shield_hp = 300.0f;
    baseMod->high_slots = 4;

    auto* child = world.createEntity("child1");
    sys.registerTemplate("child1", "child_frigate", "Child Frigate", "frigate", "minmatar");
    sys.setBaseTemplate("child1", "base_frigate");
    auto* childMod = child->getComponent<components::ShipTemplateMod>();
    sys.update(0.0f);
    assertTrue(approxEqual(childMod->hull_hp, 500.0f), "Hull inherited from base");
    assertTrue(childMod->high_slots == 4, "High slots inherited");
}

static void testShipTemplateAutoValidate() {
    std::cout << "\n=== ShipTemplateMod: Auto-Validate on Update ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "Ship", "cruiser", "caldari");
    sys.update(0.0f);
    assertTrue(sys.isValid("tmpl1"), "Auto-validated after update");
}

static void testShipTemplateHighestPriority() {
    std::cout << "\n=== ShipTemplateMod: Highest Priority ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("t1");
    world.createEntity("t2");
    sys.registerTemplate("t1", "id1", "S1", "frigate", "f1");
    sys.registerTemplate("t2", "id2", "S2", "cruiser", "f2");
    sys.setModSource("t1", "base", 5);
    sys.setModSource("t2", "addon", 15);
    assertTrue(sys.getHighestPriority() == 15, "Highest priority is 15");
}

static void testShipTemplateCount() {
    std::cout << "\n=== ShipTemplateMod: Template Count ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    assertTrue(sys.getTemplateCount() == 0, "0 templates initially");
    world.createEntity("t1");
    sys.registerTemplate("t1", "id1", "S1", "frigate", "f1");
    assertTrue(sys.getTemplateCount() == 1, "1 template after register");
    world.createEntity("t2");
    sys.registerTemplate("t2", "id2", "S2", "cruiser", "f2");
    assertTrue(sys.getTemplateCount() == 2, "2 templates after second register");
}

static void testShipTemplateGetters() {
    std::cout << "\n=== ShipTemplateMod: Getters ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("t1");
    sys.registerTemplate("t1", "my_template", "Battleship", "battleship", "amarr");
    assertTrue(sys.getTemplateId("t1") == "my_template", "Template ID getter");
    assertTrue(sys.getShipClass("t1") == "battleship", "Ship class getter");
}

static void testShipTemplateMissing() {
    std::cout << "\n=== ShipTemplateMod: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    assertTrue(!sys.registerTemplate("nonexistent", "t", "n", "c", "f"), "Register fails on missing");
    assertTrue(!sys.setBaseTemplate("nonexistent", "b"), "SetBase fails on missing");
    assertTrue(!sys.setModSource("nonexistent", "m", 0), "SetModSource fails on missing");
    assertTrue(!sys.validateTemplate("nonexistent"), "Validate fails on missing");
    assertTrue(!sys.isValid("nonexistent"), "Not valid on missing");
    assertTrue(sys.getTemplateId("nonexistent") == "", "Empty template ID on missing");
    assertTrue(sys.getShipClass("nonexistent") == "", "Empty ship class on missing");
}


void run_ship_template_mod_system_tests() {
    testShipTemplateRegister();
    testShipTemplateValidate();
    testShipTemplateValidateFail();
    testShipTemplateModSource();
    testShipTemplateBaseInherit();
    testShipTemplateAutoValidate();
    testShipTemplateHighestPriority();
    testShipTemplateCount();
    testShipTemplateGetters();
    testShipTemplateMissing();
}
