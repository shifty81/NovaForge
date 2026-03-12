// Tests for: PlayerModeSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/player_mode_system.h"

using namespace atlas;

// ==================== PlayerModeSystem Tests ====================

static void testPlayerModeInit() {
    std::cout << "\n=== PlayerMode: Init ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_current_mode("e1") == 0, "Default mode is FPS (0)");
    assertTrue(sys.get_previous_mode("e1") == 0, "Previous mode is FPS (0)");
    assertTrue(sys.get_bound_entity("e1") == "", "No bound entity");
    assertTrue(!sys.is_in_transition("e1"), "Not in transition");
    assertTrue(sys.get_total_switches("e1") == 0, "Zero switches");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testPlayerModeSwitchMode() {
    std::cout << "\n=== PlayerMode: SwitchMode ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using PM = components::PlayerModeState::PlayerMode;
    assertTrue(sys.switch_mode("e1", PM::Cockpit), "Switch to Cockpit");
    assertTrue(sys.get_current_mode("e1") == static_cast<int>(PM::Cockpit), "Current is Cockpit");
    assertTrue(sys.get_previous_mode("e1") == static_cast<int>(PM::FPS), "Previous is FPS");
    assertTrue(sys.is_in_transition("e1"), "In transition after switch");
    assertTrue(sys.get_total_switches("e1") == 1, "One switch");

    // Can't switch during transition
    assertTrue(!sys.switch_mode("e1", PM::Turret), "Blocked during transition");
    assertTrue(sys.get_total_switches("e1") == 1, "Still one switch");
}

static void testPlayerModeSameMode() {
    std::cout << "\n=== PlayerMode: SameMode ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using PM = components::PlayerModeState::PlayerMode;
    assertTrue(!sys.switch_mode("e1", PM::FPS), "Switch to same mode rejected");
    assertTrue(sys.get_total_switches("e1") == 0, "No switches");
}

static void testPlayerModeCompleteTransition() {
    std::cout << "\n=== PlayerMode: CompleteTransition ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using PM = components::PlayerModeState::PlayerMode;
    assertTrue(!sys.complete_transition("e1"), "Can't complete when not transitioning");
    sys.switch_mode("e1", PM::Cockpit);
    assertTrue(sys.complete_transition("e1"), "Complete transition succeeds");
    assertTrue(!sys.is_in_transition("e1"), "No longer transitioning");

    // Now can switch again
    assertTrue(sys.switch_mode("e1", PM::Turret), "Switch after completing");
    assertTrue(sys.get_current_mode("e1") == static_cast<int>(PM::Turret), "Now Turret");
    assertTrue(sys.get_previous_mode("e1") == static_cast<int>(PM::Cockpit), "Previous is Cockpit");
    assertTrue(sys.get_total_switches("e1") == 2, "Two switches");
}

static void testPlayerModeUpdateTransition() {
    std::cout << "\n=== PlayerMode: UpdateTransition ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using PM = components::PlayerModeState::PlayerMode;
    sys.switch_mode("e1", PM::Drone);
    assertTrue(sys.is_in_transition("e1"), "In transition");

    // Transition time is 0.5s by default, tick 0.3s
    sys.update(0.3f);
    assertTrue(sys.is_in_transition("e1"), "Still transitioning at 0.3s");

    // Tick another 0.3s → should complete
    sys.update(0.3f);
    assertTrue(!sys.is_in_transition("e1"), "Transition complete after 0.6s");
}

static void testPlayerModeBindEntity() {
    std::cout << "\n=== PlayerMode: BindEntity ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.bind_entity("e1", "ship_42"), "Bind succeeds");
    assertTrue(sys.get_bound_entity("e1") == "ship_42", "Bound to ship_42");
    assertTrue(sys.bind_entity("e1", "station_1"), "Re-bind succeeds");
    assertTrue(sys.get_bound_entity("e1") == "station_1", "Now bound to station_1");
    assertTrue(sys.bind_entity("e1", ""), "Bind empty to clear");
    assertTrue(sys.get_bound_entity("e1") == "", "Cleared");
}

static void testPlayerModeAllModes() {
    std::cout << "\n=== PlayerMode: AllModes ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using PM = components::PlayerModeState::PlayerMode;
    PM modes[] = {PM::Cockpit, PM::Turret, PM::Drone, PM::FleetCommand,
                  PM::StrategicMap, PM::Editor, PM::Spectator};
    for (auto m : modes) {
        sys.switch_mode("e1", m);
        sys.complete_transition("e1");
    }
    assertTrue(sys.get_total_switches("e1") == 7, "Seven switches through all modes");
    assertTrue(sys.get_current_mode("e1") == static_cast<int>(PM::Spectator), "Final mode Spectator");
}

static void testPlayerModeMissing() {
    std::cout << "\n=== PlayerMode: Missing ===" << std::endl;
    ecs::World world;
    systems::PlayerModeSystem sys(&world);

    using PM = components::PlayerModeState::PlayerMode;
    assertTrue(!sys.switch_mode("no", PM::Cockpit), "switch_mode fails on missing");
    assertTrue(!sys.complete_transition("no"), "complete_transition fails");
    assertTrue(!sys.bind_entity("no", "x"), "bind_entity fails");
    assertTrue(sys.get_current_mode("no") == 0, "get_current_mode default");
    assertTrue(sys.get_previous_mode("no") == 0, "get_previous_mode default");
    assertTrue(sys.get_bound_entity("no") == "", "get_bound_entity default");
    assertTrue(!sys.is_in_transition("no"), "is_in_transition default");
    assertTrue(sys.get_total_switches("no") == 0, "get_total_switches default");
}

void run_player_mode_system_tests() {
    testPlayerModeInit();
    testPlayerModeSwitchMode();
    testPlayerModeSameMode();
    testPlayerModeCompleteTransition();
    testPlayerModeUpdateTransition();
    testPlayerModeBindEntity();
    testPlayerModeAllModes();
    testPlayerModeMissing();
}
