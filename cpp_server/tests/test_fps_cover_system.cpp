// Tests for: FPSCoverSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_cover_system.h"

using namespace atlas;

// ==================== FPSCoverSystem Tests ====================

static void testCoverAddPoint() {
    std::cout << "\n=== FPSCover: Add Cover Point ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    addComp<components::FPSCover>(e);

    assertTrue(sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 1, 90.0f), "Add half cover");
    assertTrue(sys.addCoverPoint("room_1", "crate_b", 10.0f, 0.0f, 0.0f, 2, 180.0f), "Add full cover");
    assertTrue(sys.getCoverPointCount("room_1") == 2, "2 cover points");
    assertTrue(!sys.addCoverPoint("room_1", "wall_a", 0.0f, 0.0f, 0.0f, 1, 0.0f), "Dup rejected");
}

static void testCoverEnter() {
    std::cout << "\n=== FPSCover: Enter Cover ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.1f;

    sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 1, 90.0f);
    assertTrue(sys.enterCover("room_1", "wall_a"), "Enter cover");
    assertTrue(sys.getCoverState("room_1") == 3, "Transitioning");

    sys.update(0.2f);  // Complete transition
    assertTrue(sys.getCoverState("room_1") == 1, "InCover");
    assertTrue(sys.getTotalCoversUsed("room_1") == 1, "1 cover used");
}

static void testCoverLeave() {
    std::cout << "\n=== FPSCover: Leave Cover ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.01f;

    sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 1, 90.0f);
    sys.enterCover("room_1", "wall_a");
    sys.update(0.1f);  // Complete transition

    assertTrue(sys.leaveCover("room_1"), "Leave cover");
    sys.update(0.1f);
    assertTrue(sys.getCoverState("room_1") == 0, "Exposed");
}

static void testCoverPeek() {
    std::cout << "\n=== FPSCover: Peek ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.01f;

    sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 1, 90.0f);
    sys.enterCover("room_1", "wall_a");
    sys.update(0.1f);

    assertTrue(sys.startPeek("room_1"), "Start peek");
    assertTrue(sys.getCoverState("room_1") == 2, "Peeking");

    sys.update(1.0f);  // Peek for 1s
    assertTrue(cov->peek_duration > 0.0f, "Peek duration tracking");

    assertTrue(sys.stopPeek("room_1"), "Stop peek");
    assertTrue(sys.getCoverState("room_1") == 1, "Back to InCover");
}

static void testCoverDamageReduction() {
    std::cout << "\n=== FPSCover: Damage Reduction ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.01f;

    // Exposed = no reduction
    assertTrue(approxEqual(sys.getDamageReduction("room_1"), 0.0f), "No reduction when exposed");

    // Half cover
    sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 1, 90.0f);
    sys.enterCover("room_1", "wall_a");
    sys.update(0.1f);
    assertTrue(approxEqual(sys.getDamageReduction("room_1"), 0.5f), "50% in half cover");

    sys.leaveCover("room_1");
    sys.update(0.1f);

    // Full cover
    sys.addCoverPoint("room_1", "crate_b", 10.0f, 0.0f, 0.0f, 2, 180.0f);
    sys.enterCover("room_1", "crate_b");
    sys.update(0.1f);
    assertTrue(approxEqual(sys.getDamageReduction("room_1"), 0.85f), "85% in full cover");
}

static void testCoverPeekReduction() {
    std::cout << "\n=== FPSCover: Peek Reduction ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.01f;

    sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 2, 90.0f);
    sys.enterCover("room_1", "wall_a");
    sys.update(0.1f);
    sys.startPeek("room_1");

    assertTrue(approxEqual(sys.getDamageReduction("room_1"), 0.25f), "25% when peeking");
}

static void testCoverDestructible() {
    std::cout << "\n=== FPSCover: Destructible ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.01f;

    sys.addCoverPoint("room_1", "box_a", 5.0f, 0.0f, 0.0f, 3, 90.0f);  // Destructible
    sys.enterCover("room_1", "box_a");
    sys.update(0.1f);
    assertTrue(sys.getCoverState("room_1") == 1, "InCover behind destructible");

    assertTrue(sys.damageCoverPoint("room_1", "box_a", 60.0f), "Damage cover");
    assertTrue(sys.damageCoverPoint("room_1", "box_a", 60.0f), "Destroy cover");

    assertTrue(sys.getCoverState("room_1") == 0, "Exposed after cover destroyed");
    assertTrue(sys.getTotalCoversDestroyed("room_1") == 1, "1 cover destroyed");
}

static void testCoverMaxPoints() {
    std::cout << "\n=== FPSCover: Max Points ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->max_cover_points = 2;

    sys.addCoverPoint("room_1", "a", 0.0f, 0.0f, 0.0f, 1, 0.0f);
    sys.addCoverPoint("room_1", "b", 5.0f, 0.0f, 0.0f, 1, 0.0f);
    assertTrue(!sys.addCoverPoint("room_1", "c", 10.0f, 0.0f, 0.0f, 1, 0.0f), "Max enforced");
}

static void testCoverRemovePoint() {
    std::cout << "\n=== FPSCover: Remove Point ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    auto* cov = addComp<components::FPSCover>(e);
    cov->transition_time = 0.01f;

    sys.addCoverPoint("room_1", "wall_a", 5.0f, 0.0f, 0.0f, 1, 90.0f);
    sys.enterCover("room_1", "wall_a");
    sys.update(0.1f);

    assertTrue(sys.removeCoverPoint("room_1", "wall_a"), "Remove succeeds");
    assertTrue(sys.getCoverPointCount("room_1") == 0, "0 points");
    assertTrue(sys.getCoverState("room_1") == 0, "Exposed after cover removed");
}

static void testCoverMissingEntity() {
    std::cout << "\n=== FPSCover: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    assertTrue(!sys.addCoverPoint("nope", "a", 0, 0, 0, 1, 0), "Add fails");
    assertTrue(!sys.enterCover("nope", "a"), "Enter fails");
    assertTrue(!sys.leaveCover("nope"), "Leave fails");
    assertTrue(!sys.startPeek("nope"), "Peek fails");
    assertTrue(sys.getCoverState("nope") == 0, "State 0");
    assertTrue(approxEqual(sys.getDamageReduction("nope"), 0.0f), "0 reduction");
    assertTrue(sys.getCoverPointCount("nope") == 0, "0 points");
}

static void testCoverCannotEnterNone() {
    std::cout << "\n=== FPSCover: Cannot Enter None Type ===" << std::endl;
    ecs::World world;
    systems::FPSCoverSystem sys(&world);

    auto* e = world.createEntity("room_1");
    addComp<components::FPSCover>(e);

    sys.addCoverPoint("room_1", "open_a", 5.0f, 0.0f, 0.0f, 0, 90.0f);  // None type
    assertTrue(!sys.enterCover("room_1", "open_a"), "Can't enter None-type cover");
}

void run_fps_cover_system_tests() {
    testCoverAddPoint();
    testCoverEnter();
    testCoverLeave();
    testCoverPeek();
    testCoverDamageReduction();
    testCoverPeekReduction();
    testCoverDestructible();
    testCoverMaxPoints();
    testCoverRemovePoint();
    testCoverMissingEntity();
    testCoverCannotEnterNone();
}
