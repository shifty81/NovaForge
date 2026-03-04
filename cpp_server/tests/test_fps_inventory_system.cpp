// Tests for: FPS Inventory System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/crew_components.h"
#include "components/fps_components.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fps_character_controller_system.h"
#include "systems/fps_combat_system.h"
#include "systems/fps_inventory_system.h"
#include "systems/combat_system.h"
#include "systems/inventory_system.h"

using namespace atlas;

// ==================== FPS Inventory System Tests ====================

static void testFPSInventoryCreate() {
    std::cout << "\n=== FPS Inventory Create ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    assertTrue(sys.createInventory("p1", 8), "Create inventory succeeds");
    assertTrue(!sys.createInventory("p1", 8), "Duplicate create fails");
    assertTrue(sys.getMaxSlots("p1") == 8, "Max slots correct");
    assertTrue(sys.getItemCount("p1") == 0, "Starts empty");
}

static void testFPSInventoryAddRemove() {
    std::cout << "\n=== FPS Inventory Add/Remove ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 4);

    assertTrue(sys.addItem("p1", "item_pistol", "Pistol"), "Add item succeeds");
    assertTrue(sys.hasItem("p1", "item_pistol"), "Has item");
    assertTrue(sys.getItemCount("p1") == 1, "1 item");

    assertTrue(sys.removeItem("p1", "item_pistol"), "Remove succeeds");
    assertTrue(!sys.hasItem("p1", "item_pistol"), "Item removed");
    assertTrue(sys.getItemCount("p1") == 0, "0 items");
}

static void testFPSInventoryStacking() {
    std::cout << "\n=== FPS Inventory Stacking ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 4);

    sys.addItem("p1", "ammo_9mm", "9mm Ammo", 10);
    sys.addItem("p1", "ammo_9mm", "9mm Ammo", 5);

    assertTrue(sys.getItemCount("p1") == 1, "Stacked into one slot");
    assertTrue(sys.hasItem("p1", "ammo_9mm"), "Has stacked item");
}

static void testFPSInventoryFull() {
    std::cout << "\n=== FPS Inventory Full ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 2);

    sys.addItem("p1", "item_a", "Item A");
    sys.addItem("p1", "item_b", "Item B");
    assertTrue(sys.isInventoryFull("p1"), "Inventory full");
    assertTrue(!sys.addItem("p1", "item_c", "Item C"), "Can't add when full");
}

static void testFPSInventoryEquipWeapon() {
    std::cout << "\n=== FPS Inventory Equip Weapon ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 8);
    sys.addItem("p1", "wpn_pistol", "Pistol");

    assertTrue(sys.equipWeapon("p1", "wpn_pistol"), "Equip weapon succeeds");
    assertTrue(sys.getEquippedWeapon("p1") == "wpn_pistol", "Weapon equipped");

    assertTrue(sys.unequipWeapon("p1"), "Unequip succeeds");
    assertTrue(sys.getEquippedWeapon("p1").empty(), "No weapon equipped");
}

static void testFPSInventoryEquipWeaponNotInInventory() {
    std::cout << "\n=== FPS Inventory Equip Not In Inventory ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 8);

    assertTrue(!sys.equipWeapon("p1", "wpn_ghost"), "Can't equip missing item");
}

static void testFPSInventoryEquipTool() {
    std::cout << "\n=== FPS Inventory Equip Tool ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 8);
    sys.addItem("p1", "tool_repair", "Repair Tool");

    assertTrue(sys.equipTool("p1", "tool_repair"), "Equip tool succeeds");
    assertTrue(sys.getEquippedTool("p1") == "tool_repair", "Tool equipped");

    assertTrue(sys.unequipTool("p1"), "Unequip tool succeeds");
    assertTrue(sys.getEquippedTool("p1").empty(), "No tool equipped");
}

static void testFPSInventoryUseConsumableOxygen() {
    std::cout << "\n=== FPS Inventory Use Consumable Oxygen ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->oxygen = 30.0f;
    charEnt->addComponent(std::move(needs));

    sys.createInventory("p1", 8);
    sys.addItem("p1", "oxygen_canister", "Oxygen Canister");

    assertTrue(sys.useConsumable("p1", "oxygen_canister"), "Oxygen consumed");
    auto* n = charEnt->getComponent<components::SurvivalNeeds>();
    assertTrue(n->oxygen > 30.0f, "Oxygen increased");
    assertTrue(!sys.hasItem("p1", "oxygen_canister"), "Item consumed");
}

static void testFPSInventoryUseConsumableFood() {
    std::cout << "\n=== FPS Inventory Use Consumable Food ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->hunger = 60.0f;
    charEnt->addComponent(std::move(needs));

    sys.createInventory("p1", 8);
    sys.addItem("p1", "food_ration", "Ration");

    assertTrue(sys.useConsumable("p1", "food_ration"), "Food consumed");
    auto* n = charEnt->getComponent<components::SurvivalNeeds>();
    assertTrue(n->hunger < 60.0f, "Hunger reduced");
}

static void testFPSInventoryUseConsumableMedkit() {
    std::cout << "\n=== FPS Inventory Use Consumable Medkit ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);
    systems::FPSCombatSystem combatSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    combatSys.createHealth("fpschar_p1", 100.0f, 0.0f, 0.0f);
    combatSys.applyDamage("fpschar_p1", 60.0f);

    sys.createInventory("p1", 8);
    sys.addItem("p1", "medkit_basic", "Medkit");

    assertTrue(sys.useConsumable("p1", "medkit_basic"), "Medkit consumed");
    assertTrue(combatSys.getHealth("fpschar_p1") > 40.0f, "Health restored");
}

static void testFPSInventoryComponentDefaults() {
    std::cout << "\n=== FPS Inventory Component Defaults ===" << std::endl;
    components::FPSInventoryComponent inv;
    assertTrue(inv.max_slots == 8, "Default max slots 8");
    assertTrue(inv.slots.empty(), "Default empty slots");
    assertTrue(inv.equipped_weapon_id.empty(), "Default no weapon equipped");
    assertTrue(inv.equipped_tool_id.empty(), "Default no tool equipped");
    assertTrue(!inv.isFull(), "Default not full");
    assertTrue(inv.itemCount() == 0, "Default 0 items");
}


void run_fps_inventory_system_tests() {
    testFPSInventoryCreate();
    testFPSInventoryAddRemove();
    testFPSInventoryStacking();
    testFPSInventoryFull();
    testFPSInventoryEquipWeapon();
    testFPSInventoryEquipWeaponNotInInventory();
    testFPSInventoryEquipTool();
    testFPSInventoryUseConsumableOxygen();
    testFPSInventoryUseConsumableFood();
    testFPSInventoryUseConsumableMedkit();
    testFPSInventoryComponentDefaults();
}
