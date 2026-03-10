// Tests for: AbyssalFilamentSystem
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/abyssal_filament_system.h"

using namespace atlas;
using FT = components::AbyssalFilamentState::FilamentType;
using Tier = components::AbyssalFilamentState::Tier;

// ==================== AbyssalFilamentSystem Tests ====================

static void testAbyssalFilamentCreate() {
    std::cout << "\n=== AbyssalFilament: Create ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    assertTrue(sys.initialize("pilot1", "capsuleer_007"), "Init succeeds");
    assertTrue(!sys.isActive("pilot1"), "Not active initially");
    assertTrue(!sys.isRunComplete("pilot1"), "Not complete initially");
    assertTrue(sys.getCurrentPocket("pilot1") == 0, "Current pocket 0");
    assertTrue(sys.getPocketsCompleted("pilot1") == 0, "0 pockets completed");
    assertTrue(sys.getPocketsFailed("pilot1") == 0, "0 pockets failed");
    assertTrue(sys.getFilamentsConsumed("pilot1") == 0, "0 filaments consumed");
}

static void testAbyssalFilamentActivate() {
    std::cout << "\n=== AbyssalFilament: Activate ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    assertTrue(sys.activateFilament("pilot1", FT::Electrical, Tier::T1),
               "Activate T1 Electrical");
    assertTrue(sys.isActive("pilot1"), "Active after activation");
    assertTrue(sys.getFilamentsConsumed("pilot1") == 1, "1 filament consumed");
    assertTrue(sys.getCurrentPocket("pilot1") == 0, "Starting pocket 0");
    assertTrue(approxEqual(sys.getPocketTimeRemaining("pilot1"), 1200.0f),
               "20 min remaining");
}

static void testAbyssalFilamentDoubleActivate() {
    std::cout << "\n=== AbyssalFilament: DoubleActivate ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    sys.activateFilament("pilot1", FT::Gamma, Tier::T3);
    assertTrue(!sys.activateFilament("pilot1", FT::Gamma, Tier::T3),
               "Can't activate while active");
    assertTrue(sys.getFilamentsConsumed("pilot1") == 1, "Still 1 consumed");
}

static void testAbyssalFilamentCompletePockets() {
    std::cout << "\n=== AbyssalFilament: CompletePockets ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");
    sys.activateFilament("pilot1", FT::DarkMatter, Tier::T2);

    assertTrue(sys.completePocket("pilot1"), "Complete pocket 1");
    assertTrue(sys.getPocketsCompleted("pilot1") == 1, "1 pocket done");
    assertTrue(sys.getCurrentPocket("pilot1") == 1, "Now on pocket 1 (0-based)");
    assertTrue(sys.isActive("pilot1"), "Still active");

    assertTrue(sys.completePocket("pilot1"), "Complete pocket 2");
    assertTrue(sys.getPocketsCompleted("pilot1") == 2, "2 pockets done");

    assertTrue(sys.completePocket("pilot1"), "Complete pocket 3");
    assertTrue(sys.getPocketsCompleted("pilot1") == 3, "3 pockets done");
    assertTrue(!sys.isActive("pilot1"), "Inactive after all pockets done");
    assertTrue(sys.isRunComplete("pilot1"), "Run complete");
}

static void testAbyssalFilamentTimerExpiry() {
    std::cout << "\n=== AbyssalFilament: TimerExpiry ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");
    sys.activateFilament("pilot1", FT::Firestorm, Tier::T1);

    // Tick past full pocket time
    sys.update(1201.0f);
    assertTrue(!sys.isActive("pilot1"), "Inactive after timeout");
    assertTrue(sys.getPocketsFailed("pilot1") == 1, "1 pocket failed");
    assertTrue(!sys.isRunComplete("pilot1"), "Run not complete on fail");
    assertTrue(approxEqual(sys.getPocketTimeRemaining("pilot1"), 0.0f),
               "Timer at 0");
}

static void testAbyssalFilamentPartialTimer() {
    std::cout << "\n=== AbyssalFilament: PartialTimer ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");
    sys.activateFilament("pilot1", FT::ExoticPlasma, Tier::T5);

    sys.update(600.0f);
    assertTrue(approxEqual(sys.getPocketTimeRemaining("pilot1"), 600.0f),
               "600s remaining after 600s tick");
    assertTrue(sys.isActive("pilot1"), "Still active");
}

static void testAbyssalFilamentCancel() {
    std::cout << "\n=== AbyssalFilament: Cancel ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");
    sys.activateFilament("pilot1", FT::Electrical, Tier::T2);

    assertTrue(sys.cancelRun("pilot1"), "Cancel run");
    assertTrue(!sys.isActive("pilot1"), "Not active after cancel");
    assertTrue(!sys.isRunComplete("pilot1"), "Not complete");
    // Cancelling again fails
    assertTrue(!sys.cancelRun("pilot1"), "Cancel again fails");
}

static void testAbyssalFilamentMissing() {
    std::cout << "\n=== AbyssalFilament: Missing ===" << std::endl;
    ecs::World world;
    systems::AbyssalFilamentSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.activateFilament("nonexistent", FT::Electrical, Tier::T1),
               "Activate fails on missing");
    assertTrue(!sys.completePocket("nonexistent"), "Complete fails on missing");
    assertTrue(!sys.cancelRun("nonexistent"), "Cancel fails on missing");
    assertTrue(!sys.isActive("nonexistent"), "Not active on missing");
    assertTrue(!sys.isRunComplete("nonexistent"), "Not complete on missing");
    assertTrue(sys.getCurrentPocket("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getPocketsCompleted("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getPocketsFailed("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getFilamentsConsumed("nonexistent") == 0, "0 on missing");
    assertTrue(approxEqual(sys.getPocketTimeRemaining("nonexistent"), 0.0f),
               "0.0 on missing");
}

void run_abyssal_filament_system_tests() {
    testAbyssalFilamentCreate();
    testAbyssalFilamentActivate();
    testAbyssalFilamentDoubleActivate();
    testAbyssalFilamentCompletePockets();
    testAbyssalFilamentTimerExpiry();
    testAbyssalFilamentPartialTimer();
    testAbyssalFilamentCancel();
    testAbyssalFilamentMissing();
}
