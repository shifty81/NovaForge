// Tests for: KeyboardNavigationSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/keyboard_navigation_system.h"

using namespace atlas;

// ==================== KeyboardNavigationSystem Tests ====================

static void testKeyNavInitialize() {
    std::cout << "\n=== KeyboardNav: Initialize ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");

    assertTrue(sys.initializeNavigation("nav1", "player1"), "Initialize navigation");
    assertTrue(sys.getActivePanel("nav1").empty(), "No active panel initially");
    assertTrue(sys.getFocusIndex("nav1") == 0, "Focus index 0 initially");
    assertTrue(!sys.isModal("nav1"), "Not modal initially");
}

static void testKeyNavDuplicateInitRejected() {
    std::cout << "\n=== KeyboardNav: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");

    assertTrue(sys.initializeNavigation("nav1", "player1"), "First init ok");
    assertTrue(!sys.initializeNavigation("nav1", "player2"), "Duplicate init rejected");
}

static void testKeyNavSetFocusPanel() {
    std::cout << "\n=== KeyboardNav: SetFocusPanel ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    assertTrue(sys.setFocusPanel("nav1", "overview"), "Set focus panel");
    assertTrue(sys.getActivePanel("nav1") == "overview", "Active panel is overview");
    assertTrue(sys.getFocusIndex("nav1") == 0, "Focus index reset to 0");
}

static void testKeyNavMoveFocusDown() {
    std::cout << "\n=== KeyboardNav: MoveFocusDown ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    // Set up tab order
    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"item_a", "item_b", "item_c"};

    assertTrue(sys.moveFocus("nav1", "Down"), "Move focus down");
    assertTrue(sys.getFocusIndex("nav1") == 1, "Focus index is 1");

    assertTrue(sys.moveFocus("nav1", "Down"), "Move focus down again");
    assertTrue(sys.getFocusIndex("nav1") == 2, "Focus index is 2");
}

static void testKeyNavMoveFocusUp() {
    std::cout << "\n=== KeyboardNav: MoveFocusUp ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"item_a", "item_b", "item_c"};
    nav->focus_index = 2;

    assertTrue(sys.moveFocus("nav1", "Up"), "Move focus up");
    assertTrue(sys.getFocusIndex("nav1") == 1, "Focus index is 1");
}

static void testKeyNavMoveFocusClampedAtBounds() {
    std::cout << "\n=== KeyboardNav: MoveFocusClampedAtBounds ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"item_a", "item_b"};

    // Already at 0, moving up should stay at 0
    assertTrue(sys.moveFocus("nav1", "Up"), "Move up from 0");
    assertTrue(sys.getFocusIndex("nav1") == 0, "Clamped at 0");

    // Go to end
    nav->focus_index = 1;
    assertTrue(sys.moveFocus("nav1", "Down"), "Move down from end");
    assertTrue(sys.getFocusIndex("nav1") == 1, "Clamped at max");
}

static void testKeyNavMoveFocusEmptyTabOrder() {
    std::cout << "\n=== KeyboardNav: MoveFocusEmptyTabOrder ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    assertTrue(!sys.moveFocus("nav1", "Down"), "Move focus fails with empty tab order");
}

static void testKeyNavMoveFocusInvalidDirection() {
    std::cout << "\n=== KeyboardNav: MoveFocusInvalidDirection ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"item_a", "item_b"};

    assertTrue(!sys.moveFocus("nav1", "Diagonal"), "Invalid direction fails");
}

static void testKeyNavLeftRightDirections() {
    std::cout << "\n=== KeyboardNav: LeftRightDirections ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"col_a", "col_b", "col_c"};

    assertTrue(sys.moveFocus("nav1", "Right"), "Move right");
    assertTrue(sys.getFocusIndex("nav1") == 1, "Right moves index up");

    assertTrue(sys.moveFocus("nav1", "Left"), "Move left");
    assertTrue(sys.getFocusIndex("nav1") == 0, "Left moves index down");
}

static void testKeyNavActivateFocus() {
    std::cout << "\n=== KeyboardNav: ActivateFocus ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"btn_ok", "btn_cancel"};

    assertTrue(sys.activateFocus("nav1"), "Activate focus succeeds");
}

static void testKeyNavActivateFocusEmptyFails() {
    std::cout << "\n=== KeyboardNav: ActivateFocusEmptyFails ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    assertTrue(!sys.activateFocus("nav1"), "Activate fails with empty tab order");
}

static void testKeyNavFocusStack() {
    std::cout << "\n=== KeyboardNav: FocusStack ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");
    sys.setFocusPanel("nav1", "main_menu");

    assertTrue(sys.pushFocusStack("nav1", "settings"), "Push settings panel");
    assertTrue(sys.getActivePanel("nav1") == "settings", "Active panel is settings");

    assertTrue(sys.pushFocusStack("nav1", "audio_settings"), "Push audio panel");
    assertTrue(sys.getActivePanel("nav1") == "audio_settings", "Active panel is audio");

    assertTrue(sys.popFocusStack("nav1"), "Pop back to settings");
    assertTrue(sys.getActivePanel("nav1") == "settings", "Active panel back to settings");

    assertTrue(sys.popFocusStack("nav1"), "Pop back to main_menu");
    assertTrue(sys.getActivePanel("nav1") == "main_menu", "Active panel back to main_menu");

    assertTrue(!sys.popFocusStack("nav1"), "Pop empty stack fails");
}

static void testKeyNavBindKey() {
    std::cout << "\n=== KeyboardNav: BindKey ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    assertTrue(sys.bindKey("nav1", "Tab", "next_panel"), "Bind Tab key");
    assertTrue(sys.bindKey("nav1", "Escape", "close"), "Bind Escape key");
}

static void testKeyNavModal() {
    std::cout << "\n=== KeyboardNav: Modal ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    assertTrue(!sys.isModal("nav1"), "Not modal initially");

    assertTrue(sys.setModal("nav1", true, "confirm_dialog"), "Set modal");
    assertTrue(sys.isModal("nav1"), "Now modal");

    assertTrue(sys.setModal("nav1", false, ""), "Unset modal");
    assertTrue(!sys.isModal("nav1"), "No longer modal");
}

static void testKeyNavHandleKeyInputBound() {
    std::cout << "\n=== KeyboardNav: HandleKeyInputBound ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"item_a", "item_b", "item_c"};

    sys.bindKey("nav1", "j", "move_down");
    sys.bindKey("nav1", "k", "move_up");

    assertTrue(sys.handleKeyInput("nav1", "j"), "Handle j key");
    assertTrue(sys.getFocusIndex("nav1") == 1, "j moved focus down");

    assertTrue(sys.handleKeyInput("nav1", "k"), "Handle k key");
    assertTrue(sys.getFocusIndex("nav1") == 0, "k moved focus up");
}

static void testKeyNavHandleKeyInputUnbound() {
    std::cout << "\n=== KeyboardNav: HandleKeyInputUnbound ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    assertTrue(sys.handleKeyInput("nav1", "x"), "Unbound key returns true");
    auto* nav = e->getComponent<components::KeyboardNavigation>();
    assertTrue(nav->input_buffer == "x", "Unbound key buffered");

    sys.handleKeyInput("nav1", "y");
    assertTrue(nav->input_buffer == "xy", "Multiple unbound keys buffered");
}

static void testKeyNavHandleActivateBinding() {
    std::cout << "\n=== KeyboardNav: HandleActivateBinding ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"btn_ok"};

    sys.bindKey("nav1", "Enter", "activate");
    assertTrue(sys.handleKeyInput("nav1", "Enter"), "Handle Enter activates");
}

static void testKeyNavCursorBlinkUpdate() {
    std::cout << "\n=== KeyboardNav: CursorBlinkUpdate ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    auto* e = world.createEntity("nav1");
    sys.initializeNavigation("nav1", "player1");

    auto* nav = e->getComponent<components::KeyboardNavigation>();
    assertTrue(nav->cursor_visible, "Cursor initially visible");

    // Simulate enough time for one blink toggle (1.0s)
    sys.update(1.1f);
    assertTrue(!nav->cursor_visible, "Cursor toggled after 1s");
}

static void testKeyNavMissingEntity() {
    std::cout << "\n=== KeyboardNav: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);

    assertTrue(!sys.initializeNavigation("ghost", "p1"), "Init fails for missing entity");
    assertTrue(!sys.setFocusPanel("ghost", "panel"), "setFocusPanel fails for missing");
    assertTrue(!sys.moveFocus("ghost", "Down"), "moveFocus fails for missing");
    assertTrue(!sys.activateFocus("ghost"), "activateFocus fails for missing");
    assertTrue(!sys.pushFocusStack("ghost", "p"), "pushFocusStack fails for missing");
    assertTrue(!sys.popFocusStack("ghost"), "popFocusStack fails for missing");
    assertTrue(!sys.bindKey("ghost", "k", "a"), "bindKey fails for missing");
    assertTrue(sys.getActivePanel("ghost").empty(), "getActivePanel empty for missing");
    assertTrue(sys.getFocusIndex("ghost") == 0, "getFocusIndex 0 for missing");
    assertTrue(!sys.isModal("ghost"), "isModal false for missing");
    assertTrue(!sys.setModal("ghost", true, "d"), "setModal fails for missing");
    assertTrue(!sys.handleKeyInput("ghost", "x"), "handleKeyInput fails for missing");
}

void run_keyboard_navigation_system_tests() {
    testKeyNavInitialize();
    testKeyNavDuplicateInitRejected();
    testKeyNavSetFocusPanel();
    testKeyNavMoveFocusDown();
    testKeyNavMoveFocusUp();
    testKeyNavMoveFocusClampedAtBounds();
    testKeyNavMoveFocusEmptyTabOrder();
    testKeyNavMoveFocusInvalidDirection();
    testKeyNavLeftRightDirections();
    testKeyNavActivateFocus();
    testKeyNavActivateFocusEmptyFails();
    testKeyNavFocusStack();
    testKeyNavBindKey();
    testKeyNavModal();
    testKeyNavHandleKeyInputBound();
    testKeyNavHandleKeyInputUnbound();
    testKeyNavHandleActivateBinding();
    testKeyNavCursorBlinkUpdate();
    testKeyNavMissingEntity();
}
