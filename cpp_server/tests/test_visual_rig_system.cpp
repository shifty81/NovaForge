// Tests for: VisualRigSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/rig_system.h"
#include "systems/visual_rig_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== VisualRigSystem Tests ====================

static void testVisualRigInit() {
    std::cout << "\n=== Visual Rig: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    assertTrue(sys.initializeVisualState("rig1", 12345), "Visual state initialized");
    assertTrue(sys.getThrusterConfig("rig1") == "none", "No thrusters initially");
    assertTrue(sys.getCargoSize("rig1") == "none", "No cargo initially");
    assertTrue(!sys.initializeVisualState("rig1", 67890), "Duplicate init rejected");
}

static void testVisualRigUpdateFromLoadout() {
    std::cout << "\n=== Visual Rig: Update From Loadout ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod_jet");
    auto* mod2 = world.createEntity("mod_cargo");

    // Add rig loadout
    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids.push_back("mod_jet");
    loadout->installed_module_ids.push_back("mod_cargo");
    rig->addComponent(std::move(loadout));

    // Add modules
    auto jet = std::make_unique<components::RigModule>();
    jet->type = components::RigModule::ModuleType::JetpackTank;
    mod1->addComponent(std::move(jet));

    auto cargo = std::make_unique<components::RigModule>();
    cargo->type = components::RigModule::ModuleType::CargoPod;
    mod2->addComponent(std::move(cargo));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    assertTrue(sys.updateFromLoadout("rig1"), "Updated from loadout");
    assertTrue(sys.getThrusterConfig("rig1") == "single", "Single thruster");
    assertTrue(sys.getCargoSize("rig1") == "small", "Small cargo");
}

static void testVisualRigThrusterConfig() {
    std::cout << "\n=== Visual Rig: Thruster Config ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");
    auto* mod3 = world.createEntity("mod3");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2", "mod3"};
    rig->addComponent(std::move(loadout));

    // Add 3 jetpack tanks
    for (const auto& id : {"mod1", "mod2", "mod3"}) {
        auto* e = world.getEntity(id);
        auto jet = std::make_unique<components::RigModule>();
        jet->type = components::RigModule::ModuleType::JetpackTank;
        e->addComponent(std::move(jet));
    }

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.getThrusterConfig("rig1") == "quad", "Quad thrusters from 3 tanks");
}

static void testVisualRigCargoSize() {
    std::cout << "\n=== Visual Rig: Cargo Size ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2"};
    rig->addComponent(std::move(loadout));

    for (const auto& id : {"mod1", "mod2"}) {
        auto* e = world.getEntity(id);
        auto cargo = std::make_unique<components::RigModule>();
        cargo->type = components::RigModule::ModuleType::CargoPod;
        e->addComponent(std::move(cargo));
    }

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.getCargoSize("rig1") == "medium", "Medium cargo from 2 pods");
}

static void testVisualRigFeatures() {
    std::cout << "\n=== Visual Rig: Features ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");
    auto* mod3 = world.createEntity("mod3");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2", "mod3"};
    rig->addComponent(std::move(loadout));

    auto shield = std::make_unique<components::RigModule>();
    shield->type = components::RigModule::ModuleType::Shield;
    world.getEntity("mod1")->addComponent(std::move(shield));

    auto sensor = std::make_unique<components::RigModule>();
    sensor->type = components::RigModule::ModuleType::Sensor;
    world.getEntity("mod2")->addComponent(std::move(sensor));

    auto solar = std::make_unique<components::RigModule>();
    solar->type = components::RigModule::ModuleType::SolarPanel;
    world.getEntity("mod3")->addComponent(std::move(solar));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.hasShieldEmitter("rig1"), "Has shield emitter");
    assertTrue(sys.hasAntenna("rig1"), "Has antenna from sensor");
    assertTrue(sys.hasSolarPanels("rig1"), "Has solar panels");
}

static void testVisualRigMounts() {
    std::cout << "\n=== Visual Rig: Weapon/Tool Mounts ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2"};
    rig->addComponent(std::move(loadout));

    auto weapon = std::make_unique<components::RigModule>();
    weapon->type = components::RigModule::ModuleType::WeaponMount;
    world.getEntity("mod1")->addComponent(std::move(weapon));

    auto tool = std::make_unique<components::RigModule>();
    tool->type = components::RigModule::ModuleType::ToolMount;
    world.getEntity("mod2")->addComponent(std::move(tool));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.getWeaponMountCount("rig1") == 1, "1 weapon mount");
    assertTrue(sys.getToolMountCount("rig1") == 1, "1 tool mount");
}

static void testVisualRigColors() {
    std::cout << "\n=== Visual Rig: Colors ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);

    assertTrue(sys.setColors("rig1", "red", "white"), "Colors set");
    assertTrue(sys.getPrimaryColor("rig1") == "red", "Primary is red");
    assertTrue(sys.getSecondaryColor("rig1") == "white", "Secondary is white");
}

static void testVisualRigTrinkets() {
    std::cout << "\n=== Visual Rig: Trinkets ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);

    assertTrue(sys.canAddTrinket("rig1"), "Can add trinket");
    assertTrue(sys.addTrinket("rig1", "bobblehead1"), "Trinket added");
    assertTrue(sys.getTrinketCount("rig1") == 1, "1 trinket");
    assertTrue(!sys.addTrinket("rig1", "bobblehead1"), "Duplicate rejected");

    sys.addTrinket("rig1", "sticker1");
    sys.addTrinket("rig1", "mug1");
    sys.addTrinket("rig1", "figure1");
    assertTrue(sys.getTrinketCount("rig1") == 4, "4 trinkets");
    assertTrue(!sys.canAddTrinket("rig1"), "Max trinkets reached");

    assertTrue(sys.removeTrinket("rig1", "mug1"), "Trinket removed");
    assertTrue(sys.getTrinketCount("rig1") == 3, "3 trinkets");
}

static void testVisualRigBulkGlow() {
    std::cout << "\n=== Visual Rig: Bulk and Glow ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->total_cargo = 500.0f;
    loadout->total_shield = 50.0f;
    loadout->total_power = 100.0f;
    rig->addComponent(std::move(loadout));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.update(0.1f);

    assertTrue(sys.getTotalBulk("rig1") > 1.0f, "Bulk > 1 with cargo/shield");
    assertTrue(approxEqual(sys.getGlowIntensity("rig1"), 1.0f), "Glow at max from power");

    assertTrue(sys.setGlowIntensity("rig1", 0.5f), "Glow set");
    assertTrue(approxEqual(sys.getGlowIntensity("rig1"), 0.5f), "Glow at 0.5");
}

static void testVisualRigScale() {
    std::cout << "\n=== Visual Rig: Scale ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);

    assertTrue(sys.setThrusterScale("rig1", 2.0f), "Thruster scale set");
    assertTrue(approxEqual(sys.getThrusterScale("rig1"), 2.0f), "Thruster scale is 2.0");
    assertTrue(sys.setCargoScale("rig1", 1.5f), "Cargo scale set");
    assertTrue(approxEqual(sys.getCargoScale("rig1"), 1.5f), "Cargo scale is 1.5");
}

static void testVisualRigMissing() {
    std::cout << "\n=== Visual Rig: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::VisualRigSystem sys(&world);
    assertTrue(sys.getThrusterConfig("nonexistent") == "unknown", "Unknown config on missing");
    assertTrue(sys.getCargoSize("nonexistent") == "unknown", "Unknown cargo on missing");
    assertTrue(!sys.hasShieldEmitter("nonexistent"), "No shield on missing");
    assertTrue(sys.getPrimaryColor("nonexistent").empty(), "Empty color on missing");
    assertTrue(sys.getTrinketCount("nonexistent") == 0, "No trinkets on missing");
}


void run_visual_rig_system_tests() {
    testVisualRigInit();
    testVisualRigUpdateFromLoadout();
    testVisualRigThrusterConfig();
    testVisualRigCargoSize();
    testVisualRigFeatures();
    testVisualRigMounts();
    testVisualRigColors();
    testVisualRigTrinkets();
    testVisualRigBulkGlow();
    testVisualRigScale();
    testVisualRigMissing();
}
