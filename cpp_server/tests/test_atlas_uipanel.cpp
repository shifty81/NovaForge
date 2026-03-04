// Tests for: AtlasUIPanel Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/atlas_ui_panel_system.h"

using namespace atlas;

// ==================== AtlasUIPanel Tests ====================

static void testUIPanelInit() {
    std::cout << "\n=== AtlasUIPanel: Init ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    assertTrue(sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory), "Panel initialized");
    assertTrue(sys.getPanelType("panel_1") == "Inventory", "Panel type is Inventory");
    assertTrue(!sys.isOpen("panel_1"), "Panel closed by default");
    assertTrue(!sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory), "Duplicate init fails");
}

static void testUIPanelOpenClose() {
    std::cout << "\n=== AtlasUIPanel: Open/Close ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Market);
    assertTrue(sys.openPanel("panel_1"), "Panel opened");
    assertTrue(sys.isOpen("panel_1"), "Panel is open");
    assertTrue(sys.closePanel("panel_1"), "Panel closed");
    assertTrue(!sys.isOpen("panel_1"), "Panel is closed");
}

static void testUIPanelToggle() {
    std::cout << "\n=== AtlasUIPanel: Toggle ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Fitting);
    assertTrue(sys.togglePanel("panel_1"), "Toggle succeeds");
    assertTrue(sys.isOpen("panel_1"), "Panel is open after toggle");
    assertTrue(sys.togglePanel("panel_1"), "Toggle again succeeds");
    assertTrue(!sys.isOpen("panel_1"), "Panel is closed after toggle");
}

static void testUIPanelAddItem() {
    std::cout << "\n=== AtlasUIPanel: Add Item ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    assertTrue(sys.addItem("panel_1", "item_1", "Tritanium", 100, 5.0f), "Item added");
    assertTrue(sys.getItemCount("panel_1") == 1, "Item count is 1");
    assertTrue(!sys.addItem("panel_1", "item_1", "Tritanium", 100, 5.0f), "Duplicate item fails");
}

static void testUIPanelRemoveItem() {
    std::cout << "\n=== AtlasUIPanel: Remove Item ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    sys.addItem("panel_1", "item_1", "Pyerite", 50, 10.0f);
    assertTrue(sys.removeItem("panel_1", "item_1"), "Item removed");
    assertTrue(sys.getItemCount("panel_1") == 0, "Item count is 0");
    assertTrue(!sys.removeItem("panel_1", "item_1"), "Double remove fails");
}

static void testUIPanelFilter() {
    std::cout << "\n=== AtlasUIPanel: Filter ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Market);
    assertTrue(sys.setFilter("panel_1", "ore"), "Filter set");
    assertTrue(sys.setFilter("panel_1", ""), "Filter cleared");
}

static void testUIPanelSort() {
    std::cout << "\n=== AtlasUIPanel: Sort ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    assertTrue(sys.setSort("panel_1", "name", true), "Sort set ascending");
    assertTrue(sys.setSort("panel_1", "value", false), "Sort set descending");
}

static void testUIPanelSelect() {
    std::cout << "\n=== AtlasUIPanel: Select ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    sys.addItem("panel_1", "item_1", "Ore", 10, 1.0f);
    sys.addItem("panel_1", "item_2", "Mineral", 20, 2.0f);
    assertTrue(sys.selectItem("panel_1", 0), "Selected index 0");
    assertTrue(sys.selectItem("panel_1", 1), "Selected index 1");
    assertTrue(!sys.selectItem("panel_1", 5), "Out of range fails");
}

static void testUIPanelTypes() {
    std::cout << "\n=== AtlasUIPanel: All Types ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    using PT = components::AtlasUIPanel::PanelType;
    world.createEntity("p1"); sys.initializePanel("p1", "o", PT::Inventory);
    world.createEntity("p2"); sys.initializePanel("p2", "o", PT::Fitting);
    world.createEntity("p3"); sys.initializePanel("p3", "o", PT::Market);
    world.createEntity("p4"); sys.initializePanel("p4", "o", PT::Overview);
    world.createEntity("p5"); sys.initializePanel("p5", "o", PT::Chat);
    world.createEntity("p6"); sys.initializePanel("p6", "o", PT::Drone);
    assertTrue(sys.getPanelType("p1") == "Inventory", "Inventory type");
    assertTrue(sys.getPanelType("p2") == "Fitting", "Fitting type");
    assertTrue(sys.getPanelType("p3") == "Market", "Market type");
    assertTrue(sys.getPanelType("p4") == "Overview", "Overview type");
    assertTrue(sys.getPanelType("p5") == "Chat", "Chat type");
    assertTrue(sys.getPanelType("p6") == "Drone", "Drone type");
}

static void testUIPanelMissing() {
    std::cout << "\n=== AtlasUIPanel: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    assertTrue(!sys.initializePanel("nonexistent", "o", components::AtlasUIPanel::PanelType::Inventory), "Init fails on missing");
    assertTrue(!sys.isOpen("nonexistent"), "Not open on missing");
    assertTrue(sys.getItemCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getPanelType("nonexistent").empty(), "Empty type on missing");
}


void run_atlas_uipanel_tests() {
    testUIPanelInit();
    testUIPanelOpenClose();
    testUIPanelToggle();
    testUIPanelAddItem();
    testUIPanelRemoveItem();
    testUIPanelFilter();
    testUIPanelSort();
    testUIPanelSelect();
    testUIPanelTypes();
    testUIPanelMissing();
}
