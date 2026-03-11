// Tests for: WarpScramblerSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/warp_scrambler_system.h"

using namespace atlas;

// ==================== WarpScramblerSystem Tests ====================

static void testScramblerInit() {
    std::cout << "\n=== WarpScrambler: Init ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getScramblerCount("ship1") == 0, "Zero scramblers initially");
    assertTrue(sys.getTotalScramblerPoints("ship1") == 0, "Zero points initially");
    assertTrue(!sys.isWarpScrambled("ship1"), "Not scrambled initially");
    assertTrue(sys.getTotalScrambles("ship1") == 0, "Zero total scrambles initially");
    assertTrue(!sys.isMwdDisabled("ship1"), "MWD not disabled initially");
}

static void testScramblerInitFails() {
    std::cout << "\n=== WarpScrambler: InitFails ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Fails on missing entity");
}

static void testScramblerApplyDisruptor() {
    std::cout << "\n=== WarpScrambler: ApplyDisruptor ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Warp disruptor: 1 point, longer range, not a scrambler
    assertTrue(sys.applyScrambler("ship1", "disr1", "src1", 1, 24000.0f, 5.0f, false),
               "Apply disruptor");
    assertTrue(sys.getScramblerCount("ship1") == 1, "One module");
    assertTrue(sys.isWarpScrambled("ship1"), "Is scrambled after disruptor");
    assertTrue(sys.getTotalScramblerPoints("ship1") == 1, "One scramble point");
    assertTrue(!sys.isMwdDisabled("ship1"), "MWD not disabled by disruptor");
    assertTrue(sys.getTotalScrambles("ship1") == 1, "Total incremented");
}

static void testScramblerApplyScrambler() {
    std::cout << "\n=== WarpScrambler: ApplyScrambler ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Warp scrambler: 2 points, short range, is_scrambler=true
    assertTrue(sys.applyScrambler("ship1", "scr1", "src1", 2, 9000.0f, 5.0f, true),
               "Apply scrambler");
    assertTrue(sys.getTotalScramblerPoints("ship1") == 2, "Two scramble points");
    assertTrue(sys.isWarpScrambled("ship1"), "Is scrambled");
    assertTrue(sys.isMwdDisabled("ship1"), "MWD disabled by scrambler");
}

static void testScramblerApplyValidation() {
    std::cout << "\n=== WarpScrambler: ApplyValidation ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(!sys.applyScrambler("ship1", "", "src1", 1, 9000.0f, 5.0f, false),
               "Empty scrambler_id rejected");
    assertTrue(!sys.applyScrambler("ship1", "s1", "", 1, 9000.0f, 5.0f, false),
               "Empty source_id rejected");
    assertTrue(!sys.applyScrambler("ship1", "s1", "src1", 0, 9000.0f, 5.0f, false),
               "Zero points rejected");
    assertTrue(!sys.applyScrambler("ship1", "s1", "src1", -1, 9000.0f, 5.0f, false),
               "Negative points rejected");
    assertTrue(!sys.applyScrambler("ship1", "s1", "src1", 1, 0.0f, 5.0f, false),
               "Zero range rejected");
    assertTrue(!sys.applyScrambler("ship1", "s1", "src1", 1, 9000.0f, 0.0f, false),
               "Zero cycle_time rejected");

    // Duplicate
    sys.applyScrambler("ship1", "s1", "src1", 1, 9000.0f, 5.0f, false);
    assertTrue(!sys.applyScrambler("ship1", "s1", "src2", 1, 9000.0f, 5.0f, false),
               "Duplicate scrambler_id rejected");
}

static void testScramblerRemove() {
    std::cout << "\n=== WarpScrambler: Remove ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.applyScrambler("ship1", "d1", "src1", 1, 24000.0f, 5.0f, false);
    sys.applyScrambler("ship1", "d2", "src2", 1, 24000.0f, 5.0f, false);

    assertTrue(sys.removeScrambler("ship1", "d1"), "Remove first");
    assertTrue(sys.getScramblerCount("ship1") == 1, "One remaining");
    assertTrue(sys.isWarpScrambled("ship1"), "Still scrambled");
    assertTrue(!sys.removeScrambler("ship1", "nonexistent"), "Remove unknown fails");

    assertTrue(sys.removeScrambler("ship1", "d2"), "Remove last");
    assertTrue(sys.getScramblerCount("ship1") == 0, "Zero scramblers");
    assertTrue(!sys.isWarpScrambled("ship1"), "Not scrambled after all removed");
    assertTrue(sys.getTotalScramblerPoints("ship1") == 0, "Zero points");
}

static void testScramblerPointsStacking() {
    std::cout << "\n=== WarpScrambler: PointsStacking ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Disruptor (1 pt) + Scrambler (2 pts) = 3 pts
    sys.applyScrambler("ship1", "d1", "src1", 1, 24000.0f, 5.0f, false);
    assertTrue(sys.getTotalScramblerPoints("ship1") == 1, "1 point after disruptor");

    sys.applyScrambler("ship1", "s1", "src2", 2, 9000.0f, 5.0f, true);
    assertTrue(sys.getTotalScramblerPoints("ship1") == 3, "3 points after scrambler added");
    assertTrue(sys.isMwdDisabled("ship1"), "MWD disabled by scrambler");

    sys.removeScrambler("ship1", "s1");
    assertTrue(sys.getTotalScramblerPoints("ship1") == 1, "1 point after scrambler removed");
    assertTrue(!sys.isMwdDisabled("ship1"), "MWD re-enabled after scrambler removed");
    assertTrue(sys.isWarpScrambled("ship1"), "Still scrambled by disruptor");
}

static void testScramblerClearAll() {
    std::cout << "\n=== WarpScrambler: ClearAll ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.applyScrambler("ship1", "d1", "src1", 1, 24000.0f, 5.0f, false);
    sys.applyScrambler("ship1", "s1", "src2", 2, 9000.0f, 5.0f, true);

    assertTrue(sys.getScramblerCount("ship1") == 2, "Two before clear");
    assertTrue(sys.clearScramblers("ship1"), "Clear succeeds");
    assertTrue(sys.getScramblerCount("ship1") == 0, "Zero after clear");
    assertTrue(!sys.isWarpScrambled("ship1"), "Not scrambled after clear");
    assertTrue(sys.getTotalScramblerPoints("ship1") == 0, "Zero points after clear");
}

static void testScramblerCapacity() {
    std::cout << "\n=== WarpScrambler: Capacity ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    auto* comp = world.getEntity("ship1")->getComponent<components::WarpScramblerState>();
    comp->max_scramblers = 2;

    assertTrue(sys.applyScrambler("ship1", "m1", "s1", 1, 9000.0f, 5.0f, false), "First ok");
    assertTrue(sys.applyScrambler("ship1", "m2", "s2", 1, 9000.0f, 5.0f, false), "Second ok");
    assertTrue(!sys.applyScrambler("ship1", "m3", "s3", 1, 9000.0f, 5.0f, false), "Third rejected at cap");
    assertTrue(sys.getScramblerCount("ship1") == 2, "Still at 2");
}

static void testScramblerTick() {
    std::cout << "\n=== WarpScrambler: Tick ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.applyScrambler("ship1", "d1", "src1", 1, 9000.0f, 5.0f, false);

    sys.update(3.0f);
    auto* comp = world.getEntity("ship1")->getComponent<components::WarpScramblerState>();
    assertTrue(comp != nullptr, "Component exists");
    assertTrue(approxEqual(comp->scramblers[0].cycle_elapsed, 3.0f), "Cycle elapsed after 3s");

    sys.update(3.0f);
    assertTrue(approxEqual(comp->scramblers[0].cycle_elapsed, 1.0f), "Elapsed wraps (6-5=1)");
    assertTrue(sys.isWarpScrambled("ship1"), "Still scrambled after tick");
}

static void testScramblerMissing() {
    std::cout << "\n=== WarpScrambler: Missing ===" << std::endl;
    ecs::World world;
    systems::WarpScramblerSystem sys(&world);

    assertTrue(!sys.applyScrambler("nonexistent", "d1", "s1", 1, 9000.0f, 5.0f, false),
               "Apply fails on missing");
    assertTrue(!sys.removeScrambler("nonexistent", "d1"), "Remove fails on missing");
    assertTrue(!sys.clearScramblers("nonexistent"), "Clear fails on missing");
    assertTrue(sys.getTotalScramblerPoints("nonexistent") == 0, "Zero points on missing");
    assertTrue(!sys.isWarpScrambled("nonexistent"), "Not scrambled on missing");
    assertTrue(sys.getScramblerCount("nonexistent") == 0, "Zero count on missing");
    assertTrue(sys.getActiveScramblerCount("nonexistent") == 0, "Zero active on missing");
    assertTrue(sys.getTotalScrambles("nonexistent") == 0, "Zero total on missing");
    assertTrue(!sys.isMwdDisabled("nonexistent"), "MWD not disabled on missing");
}

void run_warp_scrambler_system_tests() {
    testScramblerInit();
    testScramblerInitFails();
    testScramblerApplyDisruptor();
    testScramblerApplyScrambler();
    testScramblerApplyValidation();
    testScramblerRemove();
    testScramblerPointsStacking();
    testScramblerClearAll();
    testScramblerCapacity();
    testScramblerTick();
    testScramblerMissing();
}
