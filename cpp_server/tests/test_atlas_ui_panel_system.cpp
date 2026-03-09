// Tests for: AtlasUIPanelSystem Tests
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/atlas_ui_panel_system.h"

using namespace atlas;

// ==================== AtlasUIPanelSystem Tests ====================

static void testUIPanelCreateAndDefaults() {
    std::cout << "\n=== UIPanelSystem: Create and Defaults ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    assertTrue(sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Inventory),
               "Panel created successfully");
    assertTrue(sys.getPanelType("panel_1") == "Inventory", "Panel type is Inventory");
    assertTrue(!sys.isOpen("panel_1"), "Panel closed by default");
    assertTrue(sys.getItemCount("panel_1") == 0, "Item count is 0 by default");

    world.createEntity("panel_2");
    assertTrue(sys.initializePanel("panel_2", "owner_1", components::AtlasUIPanel::PanelType::Fitting),
               "Fitting panel created");
    assertTrue(sys.getPanelType("panel_2") == "Fitting", "Panel type is Fitting");

    assertTrue(!sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Market),
               "Duplicate init on existing entity fails");
}

static void testUIPanelOpenClose() {
    std::cout << "\n=== UIPanelSystem: Open/Close/Toggle ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Market);

    assertTrue(sys.openPanel("panel_1"), "Open succeeds");
    assertTrue(sys.isOpen("panel_1"), "Panel is open");
    assertTrue(sys.closePanel("panel_1"), "Close succeeds");
    assertTrue(!sys.isOpen("panel_1"), "Panel is closed");

    assertTrue(sys.togglePanel("panel_1"), "Toggle succeeds");
    assertTrue(sys.isOpen("panel_1"), "Panel open after toggle");
    assertTrue(sys.togglePanel("panel_1"), "Second toggle succeeds");
    assertTrue(!sys.isOpen("panel_1"), "Panel closed after second toggle");
}

static void testUIPanelItemManagement() {
    std::cout << "\n=== UIPanelSystem: Item Management ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Inventory);

    assertTrue(sys.addItem("panel_1", "item_1", "Tritanium", 100, 5.0f), "First item added");
    assertTrue(sys.getItemCount("panel_1") == 1, "Item count is 1");
    assertTrue(sys.addItem("panel_1", "item_2", "Pyerite", 50, 10.0f), "Second item added");
    assertTrue(sys.getItemCount("panel_1") == 2, "Item count is 2");

    assertTrue(sys.removeItem("panel_1", "item_1"), "Item removed");
    assertTrue(sys.getItemCount("panel_1") == 1, "Item count is 1 after removal");
    assertTrue(!sys.removeItem("panel_1", "item_1"), "Double remove fails");
    assertTrue(!sys.removeItem("panel_1", "item_999"), "Remove nonexistent item fails");
}

static void testUIPanelItemCapacity() {
    std::cout << "\n=== UIPanelSystem: Item Capacity ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Inventory);

    for (int i = 0; i < 100; ++i) {
        std::string id = "item_" + std::to_string(i);
        std::string name = "Item " + std::to_string(i);
        assertTrue(sys.addItem("panel_1", id, name, 1, 1.0f),
                   ("Add item " + std::to_string(i)).c_str());
    }
    assertTrue(sys.getItemCount("panel_1") == 100, "Panel has 100 items");
    assertTrue(!sys.addItem("panel_1", "item_overflow", "Overflow", 1, 1.0f),
               "101st item rejected at max capacity");
}

static void testUIPanelDuplicateItem() {
    std::cout << "\n=== UIPanelSystem: Duplicate Item ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Inventory);

    assertTrue(sys.addItem("panel_1", "item_1", "Tritanium", 100, 5.0f), "First add succeeds");
    assertTrue(!sys.addItem("panel_1", "item_1", "Tritanium Duplicate", 200, 10.0f),
               "Duplicate item_id rejected");
    assertTrue(sys.getItemCount("panel_1") == 1, "Still 1 item after duplicate attempt");
}

static void testUIPanelFilterAndSort() {
    std::cout << "\n=== UIPanelSystem: Filter and Sort ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Market);

    assertTrue(sys.setFilter("panel_1", "ore"), "Filter set to 'ore'");
    assertTrue(sys.setFilter("panel_1", ""), "Filter cleared");
    assertTrue(sys.setSort("panel_1", "name", true), "Sort set ascending by name");
    assertTrue(sys.setSort("panel_1", "value", false), "Sort set descending by value");
}

static void testUIPanelSelection() {
    std::cout << "\n=== UIPanelSystem: Selection ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "owner_1", components::AtlasUIPanel::PanelType::Inventory);

    sys.addItem("panel_1", "item_1", "Ore", 10, 1.0f);
    sys.addItem("panel_1", "item_2", "Mineral", 20, 2.0f);
    sys.addItem("panel_1", "item_3", "Crystal", 5, 50.0f);

    assertTrue(sys.selectItem("panel_1", 0), "Select index 0");
    assertTrue(sys.selectItem("panel_1", 2), "Select index 2 (last item)");
    assertTrue(sys.selectItem("panel_1", -1), "Select index -1 (deselect)");
    assertTrue(!sys.selectItem("panel_1", 3), "Index 3 out of range fails");
    assertTrue(!sys.selectItem("panel_1", -2), "Index -2 out of range fails");
    assertTrue(!sys.selectItem("panel_1", 100), "Index 100 out of range fails");
}

static void testUIPanelMissingEntity() {
    std::cout << "\n=== UIPanelSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);

    assertTrue(!sys.initializePanel("nonexistent", "o", components::AtlasUIPanel::PanelType::Inventory),
               "Init fails on missing entity");
    assertTrue(!sys.openPanel("nonexistent"), "Open fails on missing");
    assertTrue(!sys.closePanel("nonexistent"), "Close fails on missing");
    assertTrue(!sys.togglePanel("nonexistent"), "Toggle fails on missing");
    assertTrue(!sys.isOpen("nonexistent"), "Not open on missing");
    assertTrue(sys.getItemCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getPanelType("nonexistent").empty(), "Empty type on missing");
    assertTrue(!sys.addItem("nonexistent", "i", "n", 1, 1.0f), "AddItem fails on missing");
    assertTrue(!sys.removeItem("nonexistent", "i"), "RemoveItem fails on missing");
    assertTrue(!sys.selectItem("nonexistent", 0), "SelectItem fails on missing");
}


void run_atlas_ui_panel_system_tests() {
    testUIPanelCreateAndDefaults();
    testUIPanelOpenClose();
    testUIPanelItemManagement();
    testUIPanelItemCapacity();
    testUIPanelDuplicateItem();
    testUIPanelFilterAndSort();
    testUIPanelSelection();
    testUIPanelMissingEntity();
}
