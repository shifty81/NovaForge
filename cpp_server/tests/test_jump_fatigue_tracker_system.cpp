// Tests for: JumpFatigueTrackerSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/jump_fatigue_tracker_system.h"

using namespace atlas;

// ==================== JumpFatigueTrackerSystem Tests ====================

static void testJumpFatigueCreate() {
    std::cout << "\n=== JumpFatigueTracker: Create ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    assertTrue(sys.initialize("cap1"), "Init succeeds");
    assertTrue(approxEqual(sys.getBlueTimer("cap1"), 0.0f), "Zero blue timer");
    assertTrue(approxEqual(sys.getOrangeTimer("cap1"), 0.0f), "Zero orange timer");
    assertTrue(!sys.isJumpRestricted("cap1"), "Not restricted");
    assertTrue(approxEqual(sys.getFatigueMultiplier("cap1"), 1.0f), "Default multiplier");
    assertTrue(approxEqual(sys.getDecayRate("cap1"), 1.0f), "Default decay rate");
    assertTrue(sys.getTotalJumps("cap1") == 0, "Zero jumps");
    assertTrue(sys.getTotalFatiguePenalties("cap1") == 0, "Zero penalties");
}

static void testJumpFatigueInvalidInit() {
    std::cout << "\n=== JumpFatigueTracker: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    assertTrue(!sys.initialize("missing"), "Missing entity fails");
}

static void testJumpFatigueRecordJump() {
    std::cout << "\n=== JumpFatigueTracker: RecordJump ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");

    assertTrue(sys.recordJump("cap1", 5.0f), "Record 5 LY jump");
    assertTrue(approxEqual(sys.getBlueTimer("cap1"), 5.0f), "Blue timer = 5");
    assertTrue(approxEqual(sys.getOrangeTimer("cap1"), 5.0f), "Orange timer = 5");
    assertTrue(sys.getTotalJumps("cap1") == 1, "1 jump recorded");
    assertTrue(!sys.isJumpRestricted("cap1"), "Not restricted under 300");

    assertTrue(!sys.recordJump("cap1", 0.0f), "Zero distance rejected");
    assertTrue(!sys.recordJump("cap1", -1.0f), "Negative distance rejected");
    assertTrue(!sys.recordJump("missing", 5.0f), "Missing entity rejected");
}

static void testJumpFatigueRestriction() {
    std::cout << "\n=== JumpFatigueTracker: Restriction ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");

    // Jump enough to trigger restriction (orange > 300)
    sys.recordJump("cap1", 200.0f);
    assertTrue(!sys.isJumpRestricted("cap1"), "Not restricted at 200");

    sys.recordJump("cap1", 150.0f); // total 350 > 300
    assertTrue(sys.isJumpRestricted("cap1"), "Restricted at 350");
    assertTrue(sys.getTotalFatiguePenalties("cap1") == 1, "1 penalty");
}

static void testJumpFatigueInvalidOperations() {
    std::cout << "\n=== JumpFatigueTracker: InvalidOperations ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");

    assertTrue(!sys.setFatigueMultiplier("cap1", 0.0f), "Zero multiplier rejected");
    assertTrue(!sys.setFatigueMultiplier("cap1", -1.0f), "Negative multiplier rejected");
    assertTrue(!sys.setFatigueMultiplier("missing", 1.0f), "Missing entity rejected");
    assertTrue(!sys.setDecayRate("cap1", 0.0f), "Zero decay rejected");
    assertTrue(!sys.setDecayRate("cap1", -1.0f), "Negative decay rejected");
    assertTrue(!sys.setDecayRate("missing", 1.0f), "Missing entity rejected");
    assertTrue(!sys.resetTimers("missing"), "Reset missing entity rejected");
}

static void testJumpFatigueUpdateDecay() {
    std::cout << "\n=== JumpFatigueTracker: UpdateDecay ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");
    sys.recordJump("cap1", 100.0f);

    // decay_rate = 1.0/s, after 30s: 100 - 30 = 70
    sys.update(30.0f);
    float blue = sys.getBlueTimer("cap1");
    float orange = sys.getOrangeTimer("cap1");
    assertTrue(blue > 69.0f && blue < 71.0f, "Blue timer ~70");
    assertTrue(orange > 69.0f && orange < 71.0f, "Orange timer ~70");
}

static void testJumpFatigueDecayToZero() {
    std::cout << "\n=== JumpFatigueTracker: DecayToZero ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");
    sys.recordJump("cap1", 10.0f);

    // Decay for longer than the timer
    sys.update(20.0f);
    assertTrue(approxEqual(sys.getBlueTimer("cap1"), 0.0f), "Blue clamped to 0");
    assertTrue(approxEqual(sys.getOrangeTimer("cap1"), 0.0f), "Orange clamped to 0");
}

static void testJumpFatigueRestrictionLifted() {
    std::cout << "\n=== JumpFatigueTracker: RestrictionLifted ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");

    sys.recordJump("cap1", 400.0f); // orange = 400, restricted
    assertTrue(sys.isJumpRestricted("cap1"), "Restricted");

    // Decay orange to 300 → restriction lifted
    // 400 - 100 = 300, need 100s at rate 1.0
    sys.update(100.0f);
    float orange = sys.getOrangeTimer("cap1");
    assertTrue(orange <= 300.0f, "Orange <= 300");
    assertTrue(!sys.isJumpRestricted("cap1"), "Restriction lifted");
}

static void testJumpFatigueMultiplier() {
    std::cout << "\n=== JumpFatigueTracker: Multiplier ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");
    sys.setFatigueMultiplier("cap1", 2.0f);

    sys.recordJump("cap1", 50.0f); // fatigue = 50 * 2 = 100
    assertTrue(approxEqual(sys.getBlueTimer("cap1"), 100.0f), "Blue = 100 with 2x multiplier");
    assertTrue(approxEqual(sys.getOrangeTimer("cap1"), 100.0f), "Orange = 100 with 2x multiplier");
}

static void testJumpFatigueBoundary() {
    std::cout << "\n=== JumpFatigueTracker: Boundary ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");

    // Max blue = 600, max orange = 36000
    sys.recordJump("cap1", 1000.0f); // exceeds max_blue (600)
    float blue = sys.getBlueTimer("cap1");
    float orange = sys.getOrangeTimer("cap1");
    assertTrue(approxEqual(blue, 600.0f), "Blue clamped to max 600");
    assertTrue(approxEqual(orange, 1000.0f), "Orange = 1000 (under 36000 max)");
}

static void testJumpFatigueMissing() {
    std::cout << "\n=== JumpFatigueTracker: Missing ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    assertTrue(approxEqual(sys.getBlueTimer("x"), 0.0f), "Default blue on missing");
    assertTrue(approxEqual(sys.getOrangeTimer("x"), 0.0f), "Default orange on missing");
    assertTrue(!sys.isJumpRestricted("x"), "Default restricted on missing");
    assertTrue(approxEqual(sys.getFatigueMultiplier("x"), 0.0f), "Default multiplier on missing");
    assertTrue(approxEqual(sys.getDecayRate("x"), 0.0f), "Default decay on missing");
    assertTrue(sys.getTotalJumps("x") == 0, "Default jumps on missing");
    assertTrue(sys.getTotalFatiguePenalties("x") == 0, "Default penalties on missing");
}

static void testJumpFatigueReset() {
    std::cout << "\n=== JumpFatigueTracker: Reset ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");
    sys.recordJump("cap1", 400.0f); // restricted
    assertTrue(sys.isJumpRestricted("cap1"), "Restricted before reset");

    assertTrue(sys.resetTimers("cap1"), "Reset succeeds");
    assertTrue(approxEqual(sys.getBlueTimer("cap1"), 0.0f), "Blue reset to 0");
    assertTrue(approxEqual(sys.getOrangeTimer("cap1"), 0.0f), "Orange reset to 0");
    assertTrue(!sys.isJumpRestricted("cap1"), "No longer restricted");
}

static void testJumpFatigueCombined() {
    std::cout << "\n=== JumpFatigueTracker: Combined ===" << std::endl;
    ecs::World world;
    systems::JumpFatigueTrackerSystem sys(&world);
    world.createEntity("cap1");
    sys.initialize("cap1");

    sys.setFatigueMultiplier("cap1", 1.5f);
    sys.setDecayRate("cap1", 2.0f);

    // Jump 100 LY → fatigue = 100 * 1.5 = 150
    sys.recordJump("cap1", 100.0f);
    assertTrue(approxEqual(sys.getBlueTimer("cap1"), 150.0f), "Blue = 150");
    assertTrue(sys.getTotalJumps("cap1") == 1, "1 jump");

    // Jump again 200 LY → fatigue = 200 * 1.5 = 300; total = 450
    sys.recordJump("cap1", 200.0f);
    assertTrue(sys.isJumpRestricted("cap1"), "Restricted after 2nd jump");
    assertTrue(sys.getTotalJumps("cap1") == 2, "2 jumps");

    // Decay for 50s at rate 2.0 → -100
    sys.update(50.0f);
    float orange = sys.getOrangeTimer("cap1");
    assertTrue(orange > 349.0f && orange < 351.0f, "Orange ~350 after decay");
    assertTrue(sys.isJumpRestricted("cap1"), "Still restricted");

    sys.resetTimers("cap1");
    assertTrue(!sys.isJumpRestricted("cap1"), "Unrestricted after reset");
    assertTrue(sys.getTotalJumps("cap1") == 2, "Jumps count preserved");
}

void run_jump_fatigue_tracker_system_tests() {
    testJumpFatigueCreate();
    testJumpFatigueInvalidInit();
    testJumpFatigueRecordJump();
    testJumpFatigueRestriction();
    testJumpFatigueInvalidOperations();
    testJumpFatigueUpdateDecay();
    testJumpFatigueDecayToZero();
    testJumpFatigueRestrictionLifted();
    testJumpFatigueMultiplier();
    testJumpFatigueBoundary();
    testJumpFatigueMissing();
    testJumpFatigueReset();
    testJumpFatigueCombined();
}
