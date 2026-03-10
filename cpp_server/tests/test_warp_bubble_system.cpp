// Tests for: Warp Disruption Bubble System
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_bubble_system.h"

using namespace atlas;

// ==================== Warp Bubble System Tests ====================

static void testWarpBubbleCreate() {
    std::cout << "\n=== WarpBubble: Create ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    assertTrue(sys.initialize("grid1", "null_sec_01"), "Init succeeds");
    assertTrue(sys.getBubbleCount("grid1") == 0, "No bubbles initially");
    assertTrue(sys.getActiveBubbleCount("grid1") == 0, "No active bubbles");
    assertTrue(sys.getTotalDeployed("grid1") == 0, "0 deployed");
    assertTrue(sys.getTotalShipsCaught("grid1") == 0, "0 ships caught");
}

static void testWarpBubbleDeploy() {
    std::cout << "\n=== WarpBubble: Deploy ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", "null_sec_01");

    assertTrue(sys.deployBubble("grid1", "interdictor_01", 20000.0f, 120.0f,
                                100.0f, 200.0f, 300.0f), "Deploy bubble");
    assertTrue(sys.getBubbleCount("grid1") == 1, "1 bubble");
    assertTrue(sys.getActiveBubbleCount("grid1") == 1, "1 active");
    assertTrue(sys.getTotalDeployed("grid1") == 1, "1 deployed total");
    assertTrue(!sys.isBubbleExpired("grid1", "bubble_0"), "Not expired");
    assertTrue(approxEqual(sys.getBubbleRemaining("grid1", "bubble_0"), 120.0f),
               "120s remaining");
}

static void testWarpBubbleLifetime() {
    std::cout << "\n=== WarpBubble: Lifetime ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1");
    sys.deployBubble("grid1", "interdictor_01", 20000.0f, 60.0f);

    // Tick 30 seconds
    sys.update(30.0f);
    assertTrue(approxEqual(sys.getBubbleRemaining("grid1", "bubble_0"), 30.0f),
               "30s remaining after 30s");
    assertTrue(sys.getBubbleCount("grid1") == 1, "Still 1 bubble");

    // Tick past lifetime
    sys.update(35.0f);
    // Bubble expired and removed on next tick
    assertTrue(sys.getBubbleCount("grid1") == 0, "Bubble removed after expiry");
}

static void testWarpBubbleCatchShip() {
    std::cout << "\n=== WarpBubble: CatchShip ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1");
    sys.deployBubble("grid1", "interdictor_01");

    assertTrue(sys.catchShip("grid1", "bubble_0"), "Catch ship 1");
    assertTrue(sys.catchShip("grid1", "bubble_0"), "Catch ship 2");
    assertTrue(sys.getTotalShipsCaught("grid1") == 2, "2 ships caught total");
}

static void testWarpBubbleRemove() {
    std::cout << "\n=== WarpBubble: Remove ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1");
    sys.deployBubble("grid1", "interdictor_01");

    assertTrue(sys.removeBubble("grid1", "bubble_0"), "Remove bubble");
    assertTrue(sys.getBubbleCount("grid1") == 0, "0 bubbles after remove");
    assertTrue(!sys.removeBubble("grid1", "bubble_0"), "Can't remove again");
}

static void testWarpBubbleMaxBubbles() {
    std::cout << "\n=== WarpBubble: MaxBubbles ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1");

    for (int i = 0; i < 10; i++) {
        assertTrue(sys.deployBubble("grid1", "dictor_" + std::to_string(i)),
                   "Deploy bubble " + std::to_string(i));
    }
    assertTrue(sys.getBubbleCount("grid1") == 10, "10 bubbles (max)");
    assertTrue(!sys.deployBubble("grid1", "overflow"), "Overflow rejected");
    assertTrue(sys.getTotalDeployed("grid1") == 10, "10 deployed total");
}

static void testWarpBubbleMultiple() {
    std::cout << "\n=== WarpBubble: Multiple ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1");

    // Deploy two bubbles with different lifetimes
    sys.deployBubble("grid1", "dictor_1", 20000.0f, 30.0f);
    sys.deployBubble("grid1", "dictor_2", 20000.0f, 90.0f);
    assertTrue(sys.getBubbleCount("grid1") == 2, "2 bubbles");

    // Tick 35s — first should expire
    sys.update(35.0f);
    assertTrue(sys.getBubbleCount("grid1") == 1, "1 bubble remaining");

    // Tick another 60s — second should expire
    sys.update(60.0f);
    assertTrue(sys.getBubbleCount("grid1") == 0, "All bubbles expired");
}

static void testWarpBubbleMissing() {
    std::cout << "\n=== WarpBubble: Missing ===" << std::endl;
    ecs::World world;
    systems::WarpBubbleSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.deployBubble("nonexistent", "d1"), "Deploy fails on missing");
    assertTrue(!sys.removeBubble("nonexistent", "b1"), "Remove fails on missing");
    assertTrue(!sys.catchShip("nonexistent", "b1"), "Catch fails on missing");
    assertTrue(sys.getBubbleCount("nonexistent") == 0, "0 bubbles on missing");
    assertTrue(sys.getActiveBubbleCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalDeployed("nonexistent") == 0, "0 deployed on missing");
    assertTrue(sys.getTotalShipsCaught("nonexistent") == 0, "0 caught on missing");
    assertTrue(approxEqual(sys.getBubbleRemaining("nonexistent", "b1"), 0.0f),
               "0 remaining on missing");
    assertTrue(sys.isBubbleExpired("nonexistent", "b1"), "Expired on missing");
}

void run_warp_bubble_system_tests() {
    testWarpBubbleCreate();
    testWarpBubbleDeploy();
    testWarpBubbleLifetime();
    testWarpBubbleCatchShip();
    testWarpBubbleRemove();
    testWarpBubbleMaxBubbles();
    testWarpBubbleMultiple();
    testWarpBubbleMissing();
}
