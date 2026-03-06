// Tests for: FPSTerminalHackSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_terminal_hack_system.h"

using namespace atlas;

// ==================== FPSTerminalHackSystem Tests ====================

static void testHackStart() {
    std::cout << "\n=== FPSTerminalHack: Start Hack ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    addComp<components::FPSTerminalHack>(e);

    assertTrue(sys.getState("terminal_1") == 0, "Initial: Locked");
    assertTrue(sys.startHack("terminal_1", 0.0f), "Start hack");
    assertTrue(sys.getState("terminal_1") == 1, "State: Hacking");
    assertTrue(sys.getTotalHacksAttempted("terminal_1") == 1, "1 attempt");
    assertTrue(sys.getAttemptsRemaining("terminal_1") == 2, "2 remaining");
}

static void testHackSuccess() {
    std::cout << "\n=== FPSTerminalHack: Success ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    auto* hack = addComp<components::FPSTerminalHack>(e);
    hack->hack_speed = 1.0f;   // Very fast
    hack->time_limit = 60.0f;

    sys.startHack("terminal_1", 0.5f);
    sys.update(2.0f);  // Should complete quickly at speed 1.0 * (1+0.5) / 1 = 1.5/s
    assertTrue(sys.getState("terminal_1") == 2, "State: Success");
    assertTrue(sys.getTotalHacksSucceeded("terminal_1") == 1, "1 success");
}

static void testHackTimeoutFail() {
    std::cout << "\n=== FPSTerminalHack: Timeout Fail ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    auto* hack = addComp<components::FPSTerminalHack>(e);
    hack->hack_speed = 0.01f;  // Very slow
    hack->time_limit = 5.0f;
    hack->triggers_alarm_on_fail = true;

    sys.startHack("terminal_1", 0.0f);
    sys.update(6.0f);  // Time runs out

    assertTrue(sys.getState("terminal_1") == 4, "State: Alarmed");
    assertTrue(sys.isAlarmed("terminal_1"), "Alarm triggered");
    assertTrue(sys.getTotalAlarmsTriggered("terminal_1") == 1, "1 alarm");
}

static void testHackCancel() {
    std::cout << "\n=== FPSTerminalHack: Cancel ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    addComp<components::FPSTerminalHack>(e);

    sys.startHack("terminal_1", 0.0f);
    assertTrue(sys.cancelHack("terminal_1"), "Cancel succeeds");
    assertTrue(sys.getState("terminal_1") == 0, "Back to Locked");
    assertTrue(approxEqual(sys.getProgress("terminal_1"), 0.0f), "Progress reset");
}

static void testHackMaxAttempts() {
    std::cout << "\n=== FPSTerminalHack: Max Attempts ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    auto* hack = addComp<components::FPSTerminalHack>(e);
    hack->max_attempts = 2;
    hack->hack_speed = 0.01f;
    hack->time_limit = 1.0f;
    hack->triggers_alarm_on_fail = false;

    sys.startHack("terminal_1", 0.0f);
    sys.update(2.0f);  // Fail
    assertTrue(sys.getState("terminal_1") == 3, "Failed");

    sys.startHack("terminal_1", 0.0f);
    sys.update(2.0f);  // Fail again
    assertTrue(sys.getAttemptsRemaining("terminal_1") == 0, "0 remaining");
    assertTrue(!sys.startHack("terminal_1", 0.0f), "No more attempts");
}

static void testHackSecurityLevel() {
    std::cout << "\n=== FPSTerminalHack: Security Level ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    addComp<components::FPSTerminalHack>(e);

    assertTrue(sys.setSecurityLevel("terminal_1", 3), "Set level 3");
    assertTrue(sys.setSecurityLevel("terminal_1", 5), "Set level 5");
    assertTrue(!sys.setSecurityLevel("terminal_1", 0), "Level 0 rejected");
    assertTrue(!sys.setSecurityLevel("terminal_1", 6), "Level 6 rejected");
}

static void testHackAlreadyHacking() {
    std::cout << "\n=== FPSTerminalHack: Already Hacking ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    addComp<components::FPSTerminalHack>(e);

    sys.startHack("terminal_1", 0.0f);
    assertTrue(!sys.startHack("terminal_1", 0.5f), "Can't start while hacking");
}

static void testHackReset() {
    std::cout << "\n=== FPSTerminalHack: Reset ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    auto* hack = addComp<components::FPSTerminalHack>(e);
    hack->max_attempts = 1;
    hack->hack_speed = 0.01f;
    hack->time_limit = 1.0f;
    hack->triggers_alarm_on_fail = false;

    sys.startHack("terminal_1", 0.0f);
    sys.update(2.0f);  // Fail

    assertTrue(sys.resetTerminal("terminal_1"), "Reset succeeds");
    assertTrue(sys.getState("terminal_1") == 0, "Back to Locked");
    assertTrue(sys.getAttemptsRemaining("terminal_1") == 1, "Attempts restored");
    assertTrue(sys.startHack("terminal_1", 0.0f), "Can hack again after reset");
}

static void testHackProgress() {
    std::cout << "\n=== FPSTerminalHack: Progress Tracking ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    auto* hack = addComp<components::FPSTerminalHack>(e);
    hack->hack_speed = 0.1f;
    hack->time_limit = 100.0f;

    sys.startHack("terminal_1", 0.0f);
    sys.update(1.0f);

    float progress = sys.getProgress("terminal_1");
    assertTrue(progress > 0.0f, "Progress advancing");
    assertTrue(progress < 1.0f, "Not complete yet");
}

static void testHackMissingEntity() {
    std::cout << "\n=== FPSTerminalHack: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    assertTrue(!sys.startHack("nope", 0.0f), "Start fails");
    assertTrue(!sys.cancelHack("nope"), "Cancel fails");
    assertTrue(sys.getState("nope") == 0, "State is 0");
    assertTrue(approxEqual(sys.getProgress("nope"), 0.0f), "Progress is 0");
    assertTrue(sys.getAttemptsRemaining("nope") == 0, "0 attempts");
    assertTrue(!sys.isAlarmed("nope"), "Not alarmed");
    assertTrue(sys.getTotalHacksAttempted("nope") == 0, "0 attempted");
}

static void testHackAlreadySucceeded() {
    std::cout << "\n=== FPSTerminalHack: Already Succeeded ===" << std::endl;
    ecs::World world;
    systems::FPSTerminalHackSystem sys(&world);

    auto* e = world.createEntity("terminal_1");
    auto* hack = addComp<components::FPSTerminalHack>(e);
    hack->hack_speed = 10.0f;
    hack->time_limit = 60.0f;

    sys.startHack("terminal_1", 0.0f);
    sys.update(1.0f);  // Complete
    assertTrue(sys.getState("terminal_1") == 2, "Success");
    assertTrue(!sys.startHack("terminal_1", 0.0f), "Can't hack already succeeded");
}

void run_fps_terminal_hack_system_tests() {
    testHackStart();
    testHackSuccess();
    testHackTimeoutFail();
    testHackCancel();
    testHackMaxAttempts();
    testHackSecurityLevel();
    testHackAlreadyHacking();
    testHackReset();
    testHackProgress();
    testHackMissingEntity();
    testHackAlreadySucceeded();
}
