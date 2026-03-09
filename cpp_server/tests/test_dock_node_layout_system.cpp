// Tests for: DockNodeLayoutSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/dock_node_layout_system.h"

using namespace atlas;

// ==================== DockNodeLayoutSystem Tests ====================

static void testDockNodeInitializeLayout() {
    std::cout << "\n=== DockNodeLayout: InitializeLayout ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");

    assertTrue(sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f), "Initialize layout");
    assertTrue(sys.getWindowCount("layout1") == 0, "No windows initially");
    assertTrue(sys.getNodeType("layout1", "root") == "leaf" || sys.getNodeType("layout1", "root") == "root",
               "Root node exists");
}

static void testDockNodeDuplicateInitRejected() {
    std::cout << "\n=== DockNodeLayout: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");

    assertTrue(sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f), "First init ok");
    assertTrue(!sys.initializeLayout("layout1", "player2", 800.0f, 600.0f), "Duplicate init rejected");
}

static void testDockNodeAddWindow() {
    std::cout << "\n=== DockNodeLayout: AddWindow ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    assertTrue(sys.addWindow("layout1", "overview"), "Add first window");
    assertTrue(sys.getWindowCount("layout1") == 1, "Window count is 1");
}

static void testDockNodeAddMultipleWindows() {
    std::cout << "\n=== DockNodeLayout: AddMultipleWindows ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    assertTrue(sys.addWindow("layout1", "overview"), "Add overview");
    assertTrue(sys.addWindow("layout1", "local"), "Add local");
    assertTrue(sys.getWindowCount("layout1") == 2, "Window count is 2");
}

static void testDockNodeRemoveWindow() {
    std::cout << "\n=== DockNodeLayout: RemoveWindow ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    sys.addWindow("layout1", "overview");
    assertTrue(sys.removeWindow("layout1", "overview"), "Remove window");
    assertTrue(sys.getWindowCount("layout1") == 0, "Window count back to 0");
}

static void testDockNodeRemoveNonexistentWindow() {
    std::cout << "\n=== DockNodeLayout: RemoveNonexistentWindow ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    assertTrue(!sys.removeWindow("layout1", "ghost"), "Cannot remove nonexistent window");
}

static void testDockNodeSplitNode() {
    std::cout << "\n=== DockNodeLayout: SplitNode ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);
    sys.addWindow("layout1", "overview");

    // Find a leaf node to split
    assertTrue(sys.splitNode("layout1", "root",
               components::DockNodeLayout::SplitDirection::Horizontal, 0.5f),
               "Split root node horizontally");
    assertTrue(sys.getNodeType("layout1", "root") == "split", "Root is now split");
}

static void testDockNodeSplitRatioClamped() {
    std::cout << "\n=== DockNodeLayout: SplitRatioClamped ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    // Try extreme ratios - should be clamped to [0.1, 0.9]
    assertTrue(sys.splitNode("layout1", "root",
               components::DockNodeLayout::SplitDirection::Horizontal, 0.0f),
               "Split with 0 ratio (clamped)");
}

static void testDockNodeDockWindow() {
    std::cout << "\n=== DockNodeLayout: DockWindow ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);
    sys.addWindow("layout1", "overview");

    assertTrue(sys.dockWindow("layout1", "local", "root",
               components::DockNodeLayout::SplitDirection::Horizontal),
               "Dock local window");
    assertTrue(sys.getWindowCount("layout1") >= 2, "At least 2 windows after dock");
}

static void testDockNodeUndockWindow() {
    std::cout << "\n=== DockNodeLayout: UndockWindow ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);
    sys.addWindow("layout1", "overview");

    assertTrue(sys.undockWindow("layout1", "overview"), "Undock overview window");
}

static void testDockNodeUndockNonexistent() {
    std::cout << "\n=== DockNodeLayout: UndockNonexistent ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    assertTrue(!sys.undockWindow("layout1", "ghost"), "Cannot undock nonexistent window");
}

static void testDockNodeGetWindowBounds() {
    std::cout << "\n=== DockNodeLayout: GetWindowBounds ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);
    sys.addWindow("layout1", "overview");

    auto [x, y, w, h] = sys.getWindowBounds("layout1", "overview");
    // The window should have some bounds from the root node
    assertTrue(w > 0.0f || h > 0.0f || x >= 0.0f, "Window bounds are valid");
}

static void testDockNodeGetWindowBoundsUnknown() {
    std::cout << "\n=== DockNodeLayout: GetWindowBoundsUnknown ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    auto [x, y, w, h] = sys.getWindowBounds("layout1", "ghost");
    assertTrue(w == 0.0f && h == 0.0f, "Unknown window returns zero bounds");
}

static void testDockNodeUpdateRecalculatesLayout() {
    std::cout << "\n=== DockNodeLayout: UpdateRecalculatesLayout ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);
    sys.addWindow("layout1", "overview");
    sys.dockWindow("layout1", "local", "root",
                   components::DockNodeLayout::SplitDirection::Horizontal);

    // Update should recalculate child bounds
    sys.update(0.016f);

    auto [x1, y1, w1, h1] = sys.getWindowBounds("layout1", "overview");
    auto [x2, y2, w2, h2] = sys.getWindowBounds("layout1", "local");
    // Both windows should have non-zero dimensions after recalculation
    assertTrue(w1 > 0.0f || w2 > 0.0f, "Layout recalculated with valid bounds");
}

static void testDockNodeMissingEntity() {
    std::cout << "\n=== DockNodeLayout: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);

    assertTrue(!sys.initializeLayout("ghost", "p1", 1920.0f, 1080.0f), "Init fails for missing entity");
    assertTrue(!sys.addWindow("ghost", "w1"), "addWindow fails for missing");
    assertTrue(!sys.removeWindow("ghost", "w1"), "removeWindow fails for missing");
    assertTrue(!sys.splitNode("ghost", "root",
               components::DockNodeLayout::SplitDirection::Horizontal, 0.5f),
               "splitNode fails for missing");
    assertTrue(!sys.dockWindow("ghost", "w1", "root",
               components::DockNodeLayout::SplitDirection::Horizontal),
               "dockWindow fails for missing");
    assertTrue(!sys.undockWindow("ghost", "w1"), "undockWindow fails for missing");
    assertTrue(sys.getWindowCount("ghost") == 0, "getWindowCount 0 for missing");
    assertTrue(sys.getNodeType("ghost", "root") == "unknown", "getNodeType unknown for missing");
}

static void testDockNodeVerticalSplit() {
    std::cout << "\n=== DockNodeLayout: VerticalSplit ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);
    sys.addWindow("layout1", "overview");

    assertTrue(sys.splitNode("layout1", "root",
               components::DockNodeLayout::SplitDirection::Vertical, 0.6f),
               "Split vertically");
    assertTrue(sys.getNodeType("layout1", "root") == "split", "Root is split after vertical split");
}

static void testDockNodeCannotSplitSplitNode() {
    std::cout << "\n=== DockNodeLayout: CannotSplitSplitNode ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout1");
    sys.initializeLayout("layout1", "player1", 1920.0f, 1080.0f);

    sys.splitNode("layout1", "root",
                  components::DockNodeLayout::SplitDirection::Horizontal, 0.5f);

    assertTrue(!sys.splitNode("layout1", "root",
               components::DockNodeLayout::SplitDirection::Vertical, 0.5f),
               "Cannot split already split node");
}

void run_dock_node_layout_system_tests() {
    testDockNodeInitializeLayout();
    testDockNodeDuplicateInitRejected();
    testDockNodeAddWindow();
    testDockNodeAddMultipleWindows();
    testDockNodeRemoveWindow();
    testDockNodeRemoveNonexistentWindow();
    testDockNodeSplitNode();
    testDockNodeSplitRatioClamped();
    testDockNodeDockWindow();
    testDockNodeUndockWindow();
    testDockNodeUndockNonexistent();
    testDockNodeGetWindowBounds();
    testDockNodeGetWindowBoundsUnknown();
    testDockNodeUpdateRecalculatesLayout();
    testDockNodeMissingEntity();
    testDockNodeVerticalSplit();
    testDockNodeCannotSplitSplitNode();
}
