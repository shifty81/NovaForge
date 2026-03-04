// Tests for: KeyboardNavigation Tests
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/keyboard_navigation_system.h"

using namespace atlas;

// ==================== KeyboardNavigation Tests ====================

static void testKbNavInit() {
    std::cout << "\n=== KeyboardNavigation: Init ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    assertTrue(sys.initializeNavigation("nav_1", "player_1"), "Navigation initialized");
    assertTrue(sys.getActivePanel("nav_1").empty(), "No active panel initially");
    assertTrue(sys.getFocusIndex("nav_1") == 0, "Focus index is 0");
    assertTrue(!sys.initializeNavigation("nav_1", "player_1"), "Duplicate init fails");
}

static void testKbNavFocus() {
    std::cout << "\n=== KeyboardNavigation: Focus ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    assertTrue(sys.setFocusPanel("nav_1", "inventory"), "Focus set to inventory");
    assertTrue(sys.getActivePanel("nav_1") == "inventory", "Active panel is inventory");
}

static void testKbNavMoveFocus() {
    std::cout << "\n=== KeyboardNavigation: Move Focus ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"inv", "fit", "market"};
    assertTrue(sys.moveFocus("nav_1", "Down"), "Move down");
    assertTrue(sys.getFocusIndex("nav_1") == 1, "Focus is 1 after down");
    assertTrue(sys.moveFocus("nav_1", "Up"), "Move up");
    assertTrue(sys.getFocusIndex("nav_1") == 0, "Focus is 0 after up");
}

static void testKbNavFocusStack() {
    std::cout << "\n=== KeyboardNavigation: Focus Stack ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    sys.setFocusPanel("nav_1", "main_menu");
    assertTrue(sys.pushFocusStack("nav_1", "submenu"), "Push focus stack");
    assertTrue(sys.getActivePanel("nav_1") == "submenu", "Active is submenu");
    assertTrue(sys.popFocusStack("nav_1"), "Pop focus stack");
    assertTrue(sys.getActivePanel("nav_1") == "main_menu", "Active is main_menu again");
    assertTrue(!sys.popFocusStack("nav_1"), "Pop empty stack fails");
}

static void testKbNavBindKey() {
    std::cout << "\n=== KeyboardNavigation: Bind Key ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    assertTrue(sys.bindKey("nav_1", "Tab", "next_panel"), "Key bound");
    assertTrue(sys.bindKey("nav_1", "Escape", "close"), "Escape bound");
}

static void testKbNavModal() {
    std::cout << "\n=== KeyboardNavigation: Modal ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    assertTrue(!sys.isModal("nav_1"), "Not modal initially");
    assertTrue(sys.setModal("nav_1", true, "dialog"), "Set modal");
    assertTrue(sys.isModal("nav_1"), "Is modal after set");
    assertTrue(sys.setModal("nav_1", false, ""), "Unset modal");
    assertTrue(!sys.isModal("nav_1"), "Not modal after unset");
}

static void testKbNavKeyInput() {
    std::cout << "\n=== KeyboardNavigation: Key Input ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"a", "b", "c"};
    sys.bindKey("nav_1", "j", "move_down");
    assertTrue(sys.handleKeyInput("nav_1", "j"), "Bound key handled");
    assertTrue(sys.getFocusIndex("nav_1") == 1, "Focus moved by bound key");
}

static void testKbNavCursorBlink() {
    std::cout << "\n=== KeyboardNavigation: Cursor Blink ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    bool initial = nav->cursor_visible;
    sys.update(1.0f);
    assertTrue(nav->cursor_visible != initial, "Cursor toggled after 1s");
}

static void testKbNavInputBuffer() {
    std::cout << "\n=== KeyboardNavigation: Input Buffer ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    sys.handleKeyInput("nav_1", "a");
    sys.handleKeyInput("nav_1", "b");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    assertTrue(nav->input_buffer == "ab", "Input buffer has 'ab'");
}

static void testKbNavMissing() {
    std::cout << "\n=== KeyboardNavigation: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    assertTrue(!sys.initializeNavigation("nonexistent", "o"), "Init fails on missing");
    assertTrue(sys.getActivePanel("nonexistent").empty(), "Empty panel on missing");
    assertTrue(sys.getFocusIndex("nonexistent") == 0, "Focus 0 on missing");
    assertTrue(!sys.isModal("nonexistent"), "Not modal on missing");
}


void run_keyboard_navigation_tests() {
    testKbNavInit();
    testKbNavFocus();
    testKbNavMoveFocus();
    testKbNavFocusStack();
    testKbNavBindKey();
    testKbNavModal();
    testKbNavKeyInput();
    testKbNavCursorBlink();
    testKbNavInputBuffer();
    testKbNavMissing();
}
