// Tests for: Menu System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/menu_system.h"

using namespace atlas;

// ==================== Menu System Tests ====================

static void testMenuStateDefaults() {
    std::cout << "\n=== Menu State Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu1");
    auto* menu = addComp<components::MenuState>(e);
    assertTrue(menu->current_screen == components::MenuState::Screen::TitleScreen, "Starts at TitleScreen");
    assertTrue(menu->previous_screen == components::MenuState::Screen::TitleScreen, "Previous is TitleScreen");
    assertTrue(!menu->transition_active, "No transition active");
}

static void testMenuNavigateTo() {
    std::cout << "\n=== Menu Navigate To ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu2");
    addComp<components::MenuState>(e);

    systems::MenuSystem sys(&world);
    bool result = sys.navigateTo("menu2", components::MenuState::Screen::NewGame);
    assertTrue(result, "Navigate succeeded");
    assertTrue(sys.getCurrentScreen("menu2") == components::MenuState::Screen::NewGame, "Screen is NewGame");
}

static void testMenuGoBack() {
    std::cout << "\n=== Menu Go Back ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu3");
    addComp<components::MenuState>(e);

    systems::MenuSystem sys(&world);
    sys.navigateTo("menu3", components::MenuState::Screen::NewGame);
    bool result = sys.goBack("menu3");
    assertTrue(result, "GoBack succeeded");
    assertTrue(sys.getCurrentScreen("menu3") == components::MenuState::Screen::TitleScreen, "Back to TitleScreen");
}

static void testMenuIsInGame() {
    std::cout << "\n=== Menu IsInGame ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu4");
    addComp<components::MenuState>(e);

    systems::MenuSystem sys(&world);
    assertTrue(!sys.isInGame("menu4"), "Not in game at title");
    sys.navigateTo("menu4", components::MenuState::Screen::InGame);
    assertTrue(sys.isInGame("menu4"), "In game after navigate");
}


void run_menu_system_tests() {
    testMenuStateDefaults();
    testMenuNavigateTo();
    testMenuGoBack();
    testMenuIsInGame();
}
