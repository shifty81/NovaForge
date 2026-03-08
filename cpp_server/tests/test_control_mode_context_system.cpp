// Tests for: ControlModeContextSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/control_mode_context_system.h"

using namespace atlas;
using CM = components::ControlModeContextState::ControlMode;

// ==================== ControlModeContextSystem Tests ====================

static void testControlDefaultState() {
    std::cout << "\n=== ControlModeContext: DefaultState ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::ControlModeContextState>(e);

    assertTrue(state->current_mode == CM::SpaceUI, "Default mode SpaceUI");
    assertTrue(state->previous_mode == CM::SpaceUI, "Default previous SpaceUI");
    assertTrue(!state->mouse_captured, "Default mouse not captured");
    assertTrue(state->sidebar_visible, "Default sidebar visible");
    assertTrue(!state->crosshair_visible, "Default no crosshair");
    assertTrue(!state->orbit_camera_active, "Default no orbit camera");
    assertTrue(state->mode_switches == 0, "Default 0 switches");
    assertTrue(approxEqual(state->time_in_current_mode, 0.0f), "Default time 0");
    assertTrue(state->active, "Default active");
}

static void testControlSwitchToFPS() {
    std::cout << "\n=== ControlModeContext: SwitchToFPS ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ControlModeContextState>(e);

    assertTrue(sys.switchMode("p1", CM::FPS), "Switch to FPS OK");
    assertTrue(state->current_mode == CM::FPS, "Mode is FPS");
    assertTrue(state->previous_mode == CM::SpaceUI, "Previous is SpaceUI");
    assertTrue(state->mouse_captured, "FPS mouse captured");
    assertTrue(!state->sidebar_visible, "FPS no sidebar");
    assertTrue(state->crosshair_visible, "FPS crosshair");
    assertTrue(!state->orbit_camera_active, "FPS no orbit cam");
}

static void testControlSwitchToCockpit() {
    std::cout << "\n=== ControlModeContext: SwitchToCockpit ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ControlModeContextState>(e);

    assertTrue(sys.switchMode("p1", CM::Cockpit), "Switch to Cockpit OK");
    assertTrue(!sys.isMouseCaptured("p1"), "Cockpit mouse free");
    assertTrue(sys.isSidebarVisible("p1"), "Cockpit sidebar");
    assertTrue(sys.isCrosshairVisible("p1"), "Cockpit crosshair");
}

static void testControlSwitchToFleetCommand() {
    std::cout << "\n=== ControlModeContext: SwitchToFleetCommand ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ControlModeContextState>(e);

    assertTrue(sys.switchMode("p1", CM::FleetCommand), "Switch to FleetCommand OK");
    assertTrue(!state->mouse_captured, "FleetCmd mouse free");
    assertTrue(state->sidebar_visible, "FleetCmd sidebar");
    assertTrue(!state->crosshair_visible, "FleetCmd no crosshair");
    assertTrue(state->orbit_camera_active, "FleetCmd orbit active");
}

static void testControlSwitchToStationMenu() {
    std::cout << "\n=== ControlModeContext: SwitchToStationMenu ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ControlModeContextState>(e);

    assertTrue(sys.switchMode("p1", CM::StationMenu), "Switch to StationMenu OK");
    assertTrue(!state->mouse_captured, "Station mouse free");
    assertTrue(state->sidebar_visible, "Station sidebar");
    assertTrue(!state->crosshair_visible, "Station no crosshair");
    assertTrue(!state->orbit_camera_active, "Station no orbit cam");
}

static void testControlSwitchToBuildMode() {
    std::cout << "\n=== ControlModeContext: SwitchToBuildMode ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ControlModeContextState>(e);

    assertTrue(sys.switchMode("p1", CM::BuildMode), "Switch to BuildMode OK");
    assertTrue(!state->mouse_captured, "Build mouse free");
    assertTrue(!state->sidebar_visible, "Build no sidebar");
    assertTrue(state->crosshair_visible, "Build crosshair");
    assertTrue(state->orbit_camera_active, "Build orbit active");
}

static void testControlPreviousModeTracked() {
    std::cout << "\n=== ControlModeContext: PreviousModeTracked ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ControlModeContextState>(e);

    sys.switchMode("p1", CM::FPS);
    assertTrue(state->previous_mode == CM::SpaceUI, "Previous SpaceUI");
    sys.switchMode("p1", CM::Cockpit);
    assertTrue(state->previous_mode == CM::FPS, "Previous FPS");
    sys.switchMode("p1", CM::FleetCommand);
    assertTrue(state->previous_mode == CM::Cockpit, "Previous Cockpit");
}

static void testControlModeSwitchesCounted() {
    std::cout << "\n=== ControlModeContext: ModeSwitchesCounted ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ControlModeContextState>(e);

    sys.switchMode("p1", CM::FPS);
    sys.switchMode("p1", CM::Cockpit);
    sys.switchMode("p1", CM::FleetCommand);
    assertTrue(sys.getModeSwitches("p1") == 3, "3 switches");
}

static void testControlTimeAccumulation() {
    std::cout << "\n=== ControlModeContext: TimeAccumulation ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ControlModeContextState>(e);

    sys.update(3.0f);
    assertTrue(approxEqual(state->time_in_current_mode, 3.0f), "Time 3s");

    sys.switchMode("p1", CM::FPS);
    sys.update(2.0f);
    assertTrue(approxEqual(state->time_in_current_mode, 2.0f), "Time resets on switch");
}

static void testControlSameModeNoOp() {
    std::cout << "\n=== ControlModeContext: SameModeNoOp ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ControlModeContextState>(e);

    assertTrue(!sys.switchMode("p1", CM::SpaceUI), "Same mode returns false");
    assertTrue(sys.getModeSwitches("p1") == 0, "No switch counted");

    sys.switchMode("p1", CM::FPS);
    assertTrue(!sys.switchMode("p1", CM::FPS), "Same FPS returns false");
    assertTrue(sys.getModeSwitches("p1") == 1, "Still only 1 switch");
}

static void testControlMissingEntity() {
    std::cout << "\n=== ControlModeContext: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::ControlModeContextSystem sys(&world);

    assertTrue(!sys.switchMode("x", CM::FPS), "Switch on missing");
    assertTrue(sys.getCurrentMode("x") == CM::SpaceUI, "Mode on missing");
    assertTrue(!sys.isMouseCaptured("x"), "Mouse on missing");
    assertTrue(!sys.isSidebarVisible("x"), "Sidebar on missing");
    assertTrue(!sys.isCrosshairVisible("x"), "Crosshair on missing");
    assertTrue(sys.getModeSwitches("x") == 0, "Switches on missing");
}

void run_control_mode_context_system_tests() {
    testControlDefaultState();
    testControlSwitchToFPS();
    testControlSwitchToCockpit();
    testControlSwitchToFleetCommand();
    testControlSwitchToStationMenu();
    testControlSwitchToBuildMode();
    testControlPreviousModeTracked();
    testControlModeSwitchesCounted();
    testControlTimeAccumulation();
    testControlSameModeNoOp();
    testControlMissingEntity();
}
