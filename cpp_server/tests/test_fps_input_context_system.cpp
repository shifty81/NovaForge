// Tests for: FPS Input Context System
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fps_input_context_system.h"

using namespace atlas;

// ==================== FPS Input Context System Tests ====================

static void testFPSInputContextRegister() {
    std::cout << "\n=== FPS Input Context Register ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);

    assertTrue(sys.registerPlayer("player1"), "Register player");
    assertTrue(!sys.registerPlayer("player1"), "Duplicate register fails");
    assertTrue(sys.playerCount() == 1, "Player count is 1");
}

static void testFPSInputContextDefaultTactical() {
    std::cout << "\n=== FPS Input Context Default Tactical ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    assertTrue(sys.isInTacticalContext("player1"), "Default context is Tactical");
    assertTrue(!sys.isInFPSContext("player1"), "Not in FPS context by default");
    assertTrue(!sys.isInCockpitContext("player1"), "Not in Cockpit context by default");
}

static void testFPSInputContextSwitchToFPS() {
    std::cout << "\n=== FPS Input Context Switch to FPS ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    assertTrue(sys.setContext("player1", systems::FPSInputContextSystem::InputContext::FPS),
               "Switch to FPS succeeds");
    assertTrue(sys.isInFPSContext("player1"), "Now in FPS context");
    assertTrue(!sys.isInTacticalContext("player1"), "No longer in Tactical context");
}

static void testFPSInputContextSwitchToCockpit() {
    std::cout << "\n=== FPS Input Context Switch to Cockpit ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    assertTrue(sys.setContext("player1", systems::FPSInputContextSystem::InputContext::Cockpit),
               "Switch to Cockpit succeeds");
    assertTrue(sys.isInCockpitContext("player1"), "Now in Cockpit context");
    assertTrue(!sys.isInFPSContext("player1"), "Not in FPS context");
    assertTrue(!sys.isInTacticalContext("player1"), "Not in Tactical context");
}

static void testFPSInputContextSetContextUnregistered() {
    std::cout << "\n=== FPS Input Context Unregistered Player ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);

    assertTrue(!sys.setContext("unknown", systems::FPSInputContextSystem::InputContext::FPS),
               "setContext fails for unregistered player");
    assertTrue(sys.isInTacticalContext("unknown"),
               "Unregistered player defaults to Tactical");
}

static void testFPSInputContextUnregister() {
    std::cout << "\n=== FPS Input Context Unregister ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    assertTrue(sys.unregisterPlayer("player1"), "Unregister succeeds");
    assertTrue(!sys.unregisterPlayer("player1"), "Double unregister fails");
    assertTrue(sys.playerCount() == 0, "Player count is 0");
}

static void testFPSInputContextFPSKeyMappings() {
    std::cout << "\n=== FPS Input Context Key Mappings ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);

    // GLFW key codes: W=87, A=65, S=83, D=68, Space=32, LShift=340, C=67, E=69, R=82, F=70, Tab=258, V=86
    using Action = systems::FPSInputContextSystem::FPSAction;

    assertTrue(sys.getFPSActionForKey(87)  == Action::MoveForward,  "W = MoveForward");
    assertTrue(sys.getFPSActionForKey(83)  == Action::MoveBackward, "S = MoveBackward");
    assertTrue(sys.getFPSActionForKey(65)  == Action::MoveLeft,     "A = MoveLeft");
    assertTrue(sys.getFPSActionForKey(68)  == Action::MoveRight,    "D = MoveRight");
    assertTrue(sys.getFPSActionForKey(32)  == Action::Jump,         "Space = Jump");
    assertTrue(sys.getFPSActionForKey(340) == Action::Sprint,       "LShift = Sprint");
    assertTrue(sys.getFPSActionForKey(67)  == Action::Crouch,       "C = Crouch");
    assertTrue(sys.getFPSActionForKey(341) == Action::Crouch,       "LCtrl = Crouch");
    assertTrue(sys.getFPSActionForKey(69)  == Action::Interact,     "E = Interact");
    assertTrue(sys.getFPSActionForKey(82)  == Action::Reload,       "R = Reload");
    assertTrue(sys.getFPSActionForKey(70)  == Action::ToggleFlashlight, "F = ToggleFlashlight");
    assertTrue(sys.getFPSActionForKey(258) == Action::Inventory,    "Tab = Inventory");
    assertTrue(sys.getFPSActionForKey(86)  == Action::ToggleViewMode, "V = ToggleViewMode");
    assertTrue(sys.getFPSActionForKey(999) == Action::None,         "Unknown key = None");
}

static void testFPSInputContextShouldConsumeKey() {
    std::cout << "\n=== FPS Input Context Should Consume Key ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    // In Tactical context, FPS keys should NOT be consumed
    assertTrue(!sys.shouldConsumeKey("player1", 87), "W not consumed in Tactical");
    assertTrue(!sys.shouldConsumeKey("player1", 83), "S not consumed in Tactical");
    assertTrue(!sys.shouldConsumeKey("player1", 32), "Space not consumed in Tactical");

    // Switch to FPS context
    sys.setContext("player1", systems::FPSInputContextSystem::InputContext::FPS);

    // In FPS context, FPS keys SHOULD be consumed
    assertTrue(sys.shouldConsumeKey("player1", 87),  "W consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 65),  "A consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 83),  "S consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 68),  "D consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 32),  "Space consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 340), "LShift consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 67),  "C consumed in FPS");
    assertTrue(sys.shouldConsumeKey("player1", 69),  "E consumed in FPS");

    // Unknown keys should not be consumed even in FPS
    assertTrue(!sys.shouldConsumeKey("player1", 999), "Unknown key not consumed in FPS");
}

static void testFPSInputContextMouseCapture() {
    std::cout << "\n=== FPS Input Context Mouse Capture ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    // Tactical: no mouse capture
    assertTrue(!sys.shouldCaptureMouse("player1"), "No mouse capture in Tactical");
    assertTrue(!sys.isFreeLookActive("player1"), "No free look in Tactical");

    // FPS: mouse captured and free look active
    sys.setContext("player1", systems::FPSInputContextSystem::InputContext::FPS);
    assertTrue(sys.shouldCaptureMouse("player1"), "Mouse captured in FPS");
    assertTrue(sys.isFreeLookActive("player1"), "Free look active in FPS");

    // Cockpit: no mouse capture (right-click look)
    sys.setContext("player1", systems::FPSInputContextSystem::InputContext::Cockpit);
    assertTrue(!sys.shouldCaptureMouse("player1"), "No mouse capture in Cockpit");
    assertTrue(!sys.isFreeLookActive("player1"), "No free look in Cockpit");
}

static void testFPSInputContextMultiplePlayers() {
    std::cout << "\n=== FPS Input Context Multiple Players ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");
    sys.registerPlayer("player2");
    sys.registerPlayer("player3");

    assertTrue(sys.playerCount() == 3, "3 players registered");

    sys.setContext("player1", systems::FPSInputContextSystem::InputContext::FPS);
    sys.setContext("player2", systems::FPSInputContextSystem::InputContext::Cockpit);
    // player3 stays Tactical

    assertTrue(sys.isInFPSContext("player1"), "Player1 in FPS");
    assertTrue(sys.isInCockpitContext("player2"), "Player2 in Cockpit");
    assertTrue(sys.isInTacticalContext("player3"), "Player3 in Tactical");

    // Each player's key consumption is independent
    assertTrue(sys.shouldConsumeKey("player1", 87), "Player1 consumes W (FPS)");
    assertTrue(!sys.shouldConsumeKey("player2", 87), "Player2 does not consume W (Cockpit)");
    assertTrue(!sys.shouldConsumeKey("player3", 87), "Player3 does not consume W (Tactical)");
}

static void testFPSInputContextNames() {
    std::cout << "\n=== FPS Input Context Names ===" << std::endl;
    using Ctx = systems::FPSInputContextSystem::InputContext;
    using Act = systems::FPSInputContextSystem::FPSAction;

    assertTrue(systems::FPSInputContextSystem::contextName(Ctx::Tactical) == "Tactical",
               "Tactical name");
    assertTrue(systems::FPSInputContextSystem::contextName(Ctx::FPS) == "FPS",
               "FPS name");
    assertTrue(systems::FPSInputContextSystem::contextName(Ctx::Cockpit) == "Cockpit",
               "Cockpit name");

    assertTrue(systems::FPSInputContextSystem::actionName(Act::MoveForward) == "MoveForward",
               "MoveForward action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::Jump) == "Jump",
               "Jump action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::Interact) == "Interact",
               "Interact action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::Sprint) == "Sprint",
               "Sprint action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::Crouch) == "Crouch",
               "Crouch action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::Inventory) == "Inventory",
               "Inventory action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::None) == "None",
               "None action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::BoardShip) == "BoardShip",
               "BoardShip action name");
    assertTrue(systems::FPSInputContextSystem::actionName(Act::ExitToStation) == "ExitToStation",
               "ExitToStation action name");
}

static void testFPSInputContextRoundTrip() {
    std::cout << "\n=== FPS Input Context Round-Trip ===" << std::endl;
    ecs::World world;
    systems::FPSInputContextSystem sys(&world);
    sys.registerPlayer("player1");

    using Ctx = systems::FPSInputContextSystem::InputContext;

    // Tactical -> FPS -> Cockpit -> Tactical
    assertTrue(sys.getContext("player1") == Ctx::Tactical, "Start Tactical");
    sys.setContext("player1", Ctx::FPS);
    assertTrue(sys.getContext("player1") == Ctx::FPS, "Switched to FPS");
    sys.setContext("player1", Ctx::Cockpit);
    assertTrue(sys.getContext("player1") == Ctx::Cockpit, "Switched to Cockpit");
    sys.setContext("player1", Ctx::Tactical);
    assertTrue(sys.getContext("player1") == Ctx::Tactical, "Back to Tactical");
}


void run_fps_input_context_system_tests() {
    testFPSInputContextRegister();
    testFPSInputContextDefaultTactical();
    testFPSInputContextSwitchToFPS();
    testFPSInputContextSwitchToCockpit();
    testFPSInputContextSetContextUnregistered();
    testFPSInputContextUnregister();
    testFPSInputContextFPSKeyMappings();
    testFPSInputContextShouldConsumeKey();
    testFPSInputContextMouseCapture();
    testFPSInputContextMultiplePlayers();
    testFPSInputContextNames();
    testFPSInputContextRoundTrip();
}
