// Tests for: SlotGridSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/slot_grid_system.h"

using namespace atlas;

// ==================== SlotGridSystem Tests ====================

static void testSlotGridInit() {
    std::cout << "\n=== SlotGrid: Init ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_slot_count("e1") == 0, "No slots initially");
    assertTrue(sys.get_occupied_count("e1") == 0, "Zero occupied");
    assertTrue(sys.get_ship_class("e1") == "", "No ship class");
    assertTrue(sys.get_tier("e1") == 1, "Default tier 1");
    assertTrue(sys.get_total_modules_placed("e1") == 0, "Zero modules placed");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSlotGridAddSlots() {
    std::cout << "\n=== SlotGrid: AddSlots ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.add_slot("e1", "s1", 0, 0, 0, 2), "Add slot s1");
    assertTrue(sys.add_slot("e1", "s2", 1, 0, 0, 1), "Add slot s2");
    assertTrue(sys.add_slot("e1", "s3", 0, 1, 0, 3), "Add slot s3");
    assertTrue(sys.get_slot_count("e1") == 3, "Three slots");
    assertTrue(!sys.add_slot("e1", "s1", 0, 0, 0, 2), "Duplicate rejected");
    assertTrue(!sys.add_slot("e1", "", 0, 0, 0, 0), "Empty id rejected");
    assertTrue(sys.get_slot_count("e1") == 3, "Still three slots");
}

static void testSlotGridRemoveSlot() {
    std::cout << "\n=== SlotGrid: RemoveSlot ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_slot("e1", "s1", 0, 0, 0, 2);
    sys.add_slot("e1", "s2", 1, 0, 0, 1);

    assertTrue(sys.remove_slot("e1", "s1"), "Remove s1");
    assertTrue(sys.get_slot_count("e1") == 1, "One slot left");
    assertTrue(!sys.remove_slot("e1", "s1"), "Remove nonexistent fails");
}

static void testSlotGridPlaceModule() {
    std::cout << "\n=== SlotGrid: PlaceModule ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_slot("e1", "s1", 0, 0, 0, 2);
    sys.add_slot("e1", "s2", 1, 0, 0, 1);

    assertTrue(!sys.is_slot_occupied("e1", "s1"), "s1 not occupied");
    assertTrue(sys.place_module("e1", "s1", "mod_a", "weapon"), "Place module");
    assertTrue(sys.is_slot_occupied("e1", "s1"), "s1 now occupied");
    assertTrue(sys.get_module_at("e1", "s1") == "mod_a", "Module is mod_a");
    assertTrue(sys.get_occupied_count("e1") == 1, "One occupied");
    assertTrue(sys.get_total_modules_placed("e1") == 1, "One placed total");

    // Can't place in occupied slot
    assertTrue(!sys.place_module("e1", "s1", "mod_b", "shield"), "Double-place rejected");
    // Place in nonexistent slot
    assertTrue(!sys.place_module("e1", "s_none", "mod_c", "armor"), "Nonexistent slot");
}

static void testSlotGridRemoveModule() {
    std::cout << "\n=== SlotGrid: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_slot("e1", "s1", 0, 0, 0, 2);
    sys.place_module("e1", "s1", "mod_a", "weapon");

    assertTrue(sys.remove_module("e1", "s1"), "Remove module");
    assertTrue(!sys.is_slot_occupied("e1", "s1"), "Slot now empty");
    assertTrue(sys.get_module_at("e1", "s1") == "", "Module cleared");
    assertTrue(!sys.remove_module("e1", "s1"), "Remove from empty fails");
    assertTrue(!sys.remove_module("e1", "s_none"), "Remove from nonexistent slot fails");
}

static void testSlotGridClear() {
    std::cout << "\n=== SlotGrid: Clear ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_slot("e1", "s1", 0, 0, 0, 2);
    sys.add_slot("e1", "s2", 1, 0, 0, 1);
    assertTrue(sys.clear_grid("e1"), "Clear grid");
    assertTrue(sys.get_slot_count("e1") == 0, "Zero slots after clear");
}

static void testSlotGridShipMeta() {
    std::cout << "\n=== SlotGrid: ShipMeta ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_ship_class("e1", "Frigate"), "Set ship class");
    assertTrue(sys.get_ship_class("e1") == "Frigate", "Ship class is Frigate");
    assertTrue(sys.set_tier("e1", 3), "Set tier 3");
    assertTrue(sys.get_tier("e1") == 3, "Tier is 3");
    assertTrue(!sys.set_tier("e1", 0), "Tier 0 rejected");
    assertTrue(!sys.set_tier("e1", -1), "Negative tier rejected");
    assertTrue(sys.get_tier("e1") == 3, "Tier unchanged");
}

static void testSlotGridMaxCap() {
    std::cout << "\n=== SlotGrid: MaxCap ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    for (int i = 0; i < 50; i++) {
        std::string id = "slot_" + std::to_string(i);
        assertTrue(sys.add_slot("e1", id, i % 10, i / 10, 0, 1), "Add slot within limit");
    }
    assertTrue(!sys.add_slot("e1", "slot_50", 0, 5, 0, 1), "Blocked at max");
    assertTrue(sys.get_slot_count("e1") == 50, "At max capacity");
}

static void testSlotGridUpdate() {
    std::cout << "\n=== SlotGrid: Update ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_slot("e1", "s1", 0, 0, 0, 2);
    sys.update(0.016f);
    assertTrue(sys.get_slot_count("e1") == 1, "Slot persists after update");
}

static void testSlotGridMissing() {
    std::cout << "\n=== SlotGrid: Missing ===" << std::endl;
    ecs::World world;
    systems::SlotGridSystem sys(&world);

    assertTrue(!sys.add_slot("no", "s", 0, 0, 0, 0), "add_slot fails");
    assertTrue(!sys.remove_slot("no", "s"), "remove_slot fails");
    assertTrue(!sys.place_module("no", "s", "m", "t"), "place_module fails");
    assertTrue(!sys.remove_module("no", "s"), "remove_module fails");
    assertTrue(!sys.clear_grid("no"), "clear_grid fails");
    assertTrue(!sys.set_ship_class("no", "x"), "set_ship_class fails");
    assertTrue(!sys.set_tier("no", 1), "set_tier fails");
    assertTrue(!sys.is_slot_occupied("no", "s"), "is_slot_occupied default");
    assertTrue(sys.get_module_at("no", "s") == "", "get_module_at default");
    assertTrue(sys.get_slot_count("no") == 0, "get_slot_count default");
    assertTrue(sys.get_occupied_count("no") == 0, "get_occupied_count default");
    assertTrue(sys.get_ship_class("no") == "", "get_ship_class default");
    assertTrue(sys.get_tier("no") == 0, "get_tier default");
    assertTrue(sys.get_total_modules_placed("no") == 0, "get_total_modules_placed default");
}

void run_slot_grid_system_tests() {
    testSlotGridInit();
    testSlotGridAddSlots();
    testSlotGridRemoveSlot();
    testSlotGridPlaceModule();
    testSlotGridRemoveModule();
    testSlotGridClear();
    testSlotGridShipMeta();
    testSlotGridMaxCap();
    testSlotGridUpdate();
    testSlotGridMissing();
}
