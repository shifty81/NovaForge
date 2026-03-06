// Tests for: WarpDisruption System Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_disruption_system.h"

using namespace atlas;

// ==================== WarpDisruption System Tests ====================

static void testWarpDisruptionCreate() {
    std::cout << "\n=== WarpDisruption: Create ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeDisruption("ship1", 1), "Init disruption succeeds");
    assertTrue(sys.getDisruptorCount("ship1") == 0, "No disruptors initially");
    assertTrue(sys.getTotalStrength("ship1") == 0, "0 total strength");
    assertTrue(!sys.isWarpBlocked("ship1"), "Warp not blocked initially");
    assertTrue(sys.getWarpCoreStrength("ship1") == 1, "Warp core strength = 1");
}

static void testWarpDisruptionApply() {
    std::cout << "\n=== WarpDisruption: Apply ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 1);
    assertTrue(sys.applyDisruptor("ship1", "tackler1", 1, 24000.0f), "Apply disruptor succeeds");
    assertTrue(sys.getDisruptorCount("ship1") == 1, "1 disruptor");
    assertTrue(sys.getTotalStrength("ship1") == 1, "Strength = 1");
    assertTrue(sys.isWarpBlocked("ship1"), "Warp blocked by disruptor");
}

static void testWarpDisruptionDuplicate() {
    std::cout << "\n=== WarpDisruption: Duplicate ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 1);
    sys.applyDisruptor("ship1", "tackler1", 1, 24000.0f);
    assertTrue(!sys.applyDisruptor("ship1", "tackler1", 2, 10000.0f), "Duplicate source rejected");
    assertTrue(sys.getDisruptorCount("ship1") == 1, "Still 1 disruptor");
}

static void testWarpDisruptionScrambler() {
    std::cout << "\n=== WarpDisruption: Scrambler ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 2);  // 2 warp core strength
    // Single disruptor (str=1) not enough
    sys.applyDisruptor("ship1", "tackler1", 1, 24000.0f);
    assertTrue(!sys.isWarpBlocked("ship1"), "1 point not enough vs 2 core");
    // Scrambler (str=2) applied
    sys.applyDisruptor("ship1", "tackler2", 2, 9000.0f);
    assertTrue(sys.isWarpBlocked("ship1"), "3 points blocks 2 core strength");
    assertTrue(sys.getTotalStrength("ship1") == 3, "Total strength = 3");
}

static void testWarpDisruptionRemove() {
    std::cout << "\n=== WarpDisruption: Remove ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 1);
    sys.applyDisruptor("ship1", "tackler1", 1, 24000.0f);
    assertTrue(sys.isWarpBlocked("ship1"), "Initially blocked");
    assertTrue(sys.removeDisruptor("ship1", "tackler1"), "Remove succeeds");
    assertTrue(sys.getDisruptorCount("ship1") == 0, "0 disruptors");
    assertTrue(!sys.isWarpBlocked("ship1"), "Warp unblocked");
    assertTrue(sys.getTotalEscapes("ship1") == 1, "1 escape counted");
}

static void testWarpDisruptionClearAll() {
    std::cout << "\n=== WarpDisruption: ClearAll ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 1);
    sys.applyDisruptor("ship1", "t1", 1, 24000.0f);
    sys.applyDisruptor("ship1", "t2", 2, 9000.0f);
    assertTrue(sys.clearAllDisruptors("ship1"), "Clear all succeeds");
    assertTrue(sys.getDisruptorCount("ship1") == 0, "0 disruptors after clear");
    assertTrue(!sys.isWarpBlocked("ship1"), "Warp unblocked after clear");
}

static void testWarpDisruptionMaxLimit() {
    std::cout << "\n=== WarpDisruption: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 20);
    auto* entity = world.getEntity("ship1");
    auto* wd = entity->getComponent<components::WarpDisruption>();
    wd->max_disruptors = 3;

    sys.applyDisruptor("ship1", "t1", 1, 24000.0f);
    sys.applyDisruptor("ship1", "t2", 1, 24000.0f);
    sys.applyDisruptor("ship1", "t3", 1, 24000.0f);
    assertTrue(!sys.applyDisruptor("ship1", "t4", 1, 24000.0f), "Max limit enforced");
    assertTrue(sys.getDisruptorCount("ship1") == 3, "Still 3 disruptors");
}

static void testWarpDisruptionStats() {
    std::cout << "\n=== WarpDisruption: Stats ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDisruption("ship1", 1);
    sys.applyDisruptor("ship1", "t1", 1, 24000.0f);
    sys.applyDisruptor("ship1", "t2", 1, 24000.0f);
    assertTrue(sys.getTotalDisruptionsApplied("ship1") == 2, "2 disruptions applied");
    sys.removeDisruptor("ship1", "t1");
    sys.removeDisruptor("ship1", "t2");
    assertTrue(sys.getTotalEscapes("ship1") == 1, "1 escape (when last removed)");
}

static void testWarpDisruptionMissing() {
    std::cout << "\n=== WarpDisruption: Missing ===" << std::endl;
    ecs::World world;
    systems::WarpDisruptionSystem sys(&world);
    assertTrue(!sys.initializeDisruption("nonexistent", 1), "Init fails on missing");
    assertTrue(!sys.applyDisruptor("nonexistent", "t1", 1, 24000.0f), "Apply fails on missing");
    assertTrue(!sys.removeDisruptor("nonexistent", "t1"), "Remove fails on missing");
    assertTrue(sys.getDisruptorCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getTotalStrength("nonexistent") == 0, "0 strength on missing");
    assertTrue(!sys.isWarpBlocked("nonexistent"), "Not blocked on missing");
    assertTrue(sys.getWarpCoreStrength("nonexistent") == 0, "0 core strength on missing");
    assertTrue(sys.getTotalDisruptionsApplied("nonexistent") == 0, "0 applied on missing");
    assertTrue(sys.getTotalEscapes("nonexistent") == 0, "0 escapes on missing");
    assertTrue(!sys.clearAllDisruptors("nonexistent"), "Clear fails on missing");
}

void run_warp_disruption_system_tests() {
    testWarpDisruptionCreate();
    testWarpDisruptionApply();
    testWarpDisruptionDuplicate();
    testWarpDisruptionScrambler();
    testWarpDisruptionRemove();
    testWarpDisruptionClearAll();
    testWarpDisruptionMaxLimit();
    testWarpDisruptionStats();
    testWarpDisruptionMissing();
}
