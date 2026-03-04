// Tests for: DockNodeLayout Tests
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/dock_node_layout_system.h"

using namespace atlas;

// ==================== DockNodeLayout Tests ====================

static void testDockNodeInit() {
    std::cout << "\n=== DockNodeLayout: Init ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    assertTrue(sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f), "Layout initialized");
    assertTrue(sys.getWindowCount("layout_1") == 0, "No windows initially");
    assertTrue(sys.getNodeType("layout_1", "root") == "root" || sys.getNodeType("layout_1", "root") == "leaf", "Root node exists");
    assertTrue(!sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f), "Duplicate init fails");
}

static void testDockNodeAddWindow() {
    std::cout << "\n=== DockNodeLayout: Add Window ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    assertTrue(sys.addWindow("layout_1", "win_1"), "Window added");
    assertTrue(sys.getWindowCount("layout_1") == 1, "Window count is 1");
    assertTrue(sys.addWindow("layout_1", "win_2"), "Second window added");
    assertTrue(sys.getWindowCount("layout_1") == 2, "Window count is 2");
}

static void testDockNodeRemoveWindow() {
    std::cout << "\n=== DockNodeLayout: Remove Window ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    assertTrue(sys.getWindowCount("layout_1") == 1, "1 window before remove");
    assertTrue(sys.removeWindow("layout_1", "win_1"), "Window removed");
    assertTrue(sys.getWindowCount("layout_1") == 0, "0 windows after remove");
    assertTrue(!sys.removeWindow("layout_1", "win_1"), "Double remove fails");
}

static void testDockNodeSplit() {
    std::cout << "\n=== DockNodeLayout: Split ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    // Find the node with win_1 - it should be root which became a leaf
    assertTrue(sys.splitNode("layout_1", "root",
        components::DockNodeLayout::SplitDirection::Horizontal, 0.5f), "Node split");
    assertTrue(sys.getNodeType("layout_1", "root") == "split", "Root is now split");
}

static void testDockNodeDock() {
    std::cout << "\n=== DockNodeLayout: Dock ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    // Find leaf node with win_1
    // Dock win_2 next to root
    assertTrue(sys.dockWindow("layout_1", "win_2", "root",
        components::DockNodeLayout::SplitDirection::Horizontal), "Window docked");
    assertTrue(sys.getWindowCount("layout_1") == 2, "2 windows after dock");
}

static void testDockNodeUndock() {
    std::cout << "\n=== DockNodeLayout: Undock ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    assertTrue(sys.undockWindow("layout_1", "win_1"), "Window undocked");
    assertTrue(sys.getWindowCount("layout_1") == 0, "0 windows after undock");
    assertTrue(!sys.undockWindow("layout_1", "win_1"), "Double undock fails");
}

static void testDockNodeMaxWindows() {
    std::cout << "\n=== DockNodeLayout: Max Windows ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    for (int i = 0; i < 20; i++) {
        sys.addWindow("layout_1", "win_" + std::to_string(i));
    }
    assertTrue(sys.getWindowCount("layout_1") == 20, "20 windows added");
    assertTrue(!sys.addWindow("layout_1", "win_overflow"), "Cannot exceed max windows");
}

static void testDockNodeLayout() {
    std::cout << "\n=== DockNodeLayout: Layout Calculation ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1000.0f, 500.0f);
    sys.addWindow("layout_1", "win_1");
    sys.dockWindow("layout_1", "win_2", "root",
        components::DockNodeLayout::SplitDirection::Horizontal);
    sys.update(0.016f);
    auto bounds1 = sys.getWindowBounds("layout_1", "win_1");
    auto bounds2 = sys.getWindowBounds("layout_1", "win_2");
    assertTrue(std::get<2>(bounds1) > 0.0f, "Win1 has width");
    assertTrue(std::get<2>(bounds2) > 0.0f, "Win2 has width");
    assertTrue(approxEqual(std::get<2>(bounds1) + std::get<2>(bounds2), 1000.0f), "Widths sum to total");
}

static void testDockNodeNestedSplit() {
    std::cout << "\n=== DockNodeLayout: Nested Split ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    sys.splitNode("layout_1", "root",
        components::DockNodeLayout::SplitDirection::Horizontal, 0.5f);
    // The left child should have win_1, right is empty leaf
    assertTrue(sys.getNodeType("layout_1", "root") == "split", "Root is split");
    // Add window to the right child
    sys.addWindow("layout_1", "win_2");
    assertTrue(sys.getWindowCount("layout_1") == 2, "2 windows after nested ops");
}

static void testDockNodeMissing() {
    std::cout << "\n=== DockNodeLayout: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    assertTrue(!sys.initializeLayout("nonexistent", "o", 100.0f, 100.0f), "Init fails on missing");
    assertTrue(!sys.addWindow("nonexistent", "win"), "Add fails on missing");
    assertTrue(sys.getWindowCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getNodeType("nonexistent", "root") == "unknown", "Type unknown on missing");
}


void run_dock_node_layout_tests() {
    testDockNodeInit();
    testDockNodeAddWindow();
    testDockNodeRemoveWindow();
    testDockNodeSplit();
    testDockNodeDock();
    testDockNodeUndock();
    testDockNodeMaxWindows();
    testDockNodeLayout();
    testDockNodeNestedSplit();
    testDockNodeMissing();
}
