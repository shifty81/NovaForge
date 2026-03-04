// Tests for: VisualFeedbackQueue Tests
#include "test_log.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/visual_feedback_queue_system.h"

using namespace atlas;

// ==================== VisualFeedbackQueue Tests ====================

static void testVisualFeedbackCreate() {
    std::cout << "\n=== VisualFeedbackQueue: Create ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    assertTrue(sys.createQueue("vfq1"), "Create queue succeeds");
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    assertTrue(vfq != nullptr, "Component exists");
    assertTrue(vfq->active, "Active by default");
    assertTrue(vfq->max_effects == 20, "Default max effects is 20");
    assertTrue(vfq->total_effects_queued == 0, "No effects queued");
    assertTrue(vfq->effects.empty(), "Effects list empty");
}

static void testVisualFeedbackQueue() {
    std::cout << "\n=== VisualFeedbackQueue: Queue ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    int id = sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Hit");
    assertTrue(id > 0, "Effect id is positive");
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "One active effect");
    assertTrue(sys.getTotalQueued("vfq1") == 1, "Total queued is 1");
}

static void testVisualFeedbackMultiple() {
    std::cout << "\n=== VisualFeedbackQueue: Multiple ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Hit1");
    sys.queueEffect("vfq1", 1, 0.5f, 3.0f, 2, "Shield");
    sys.queueEffect("vfq1", 2, 0.8f, 1.5f, 0, "Heal");
    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "Three active effects");
    assertTrue(sys.getTotalQueued("vfq1") == 3, "Total queued is 3");
}

static void testVisualFeedbackExpiry() {
    std::cout << "\n=== VisualFeedbackQueue: Expiry ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 1.0f, 1, "Hit");
    sys.update(1.5f);
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "Effect expired");
    assertTrue(sys.getTotalExpired("vfq1") == 1, "One expired");
}

static void testVisualFeedbackFading() {
    std::cout << "\n=== VisualFeedbackQueue: Fading ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 1.0f, 1, "Hit");
    sys.update(0.8f); // lifetime = 0.2, which is < 0.3
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    assertTrue(vfq->effects.size() == 1, "Effect still alive");
    assertTrue(vfq->effects[0].fading, "Effect is fading");
}

static void testVisualFeedbackPriority() {
    std::cout << "\n=== VisualFeedbackQueue: Priority ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Normal");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 3, "Critical");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 0, "Low");
    assertTrue(sys.getHighestPriority("vfq1") == 3, "Highest priority is Critical (3)");
}

static void testVisualFeedbackOverflow() {
    std::cout << "\n=== VisualFeedbackQueue: Overflow ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    vfq->max_effects = 3;
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "A");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 0, "B");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 2, "C");
    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "At max capacity");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 3, "D");
    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "Still at max after overflow");
    assertTrue(sys.getHighestPriority("vfq1") == 3, "Highest is now 3");
}

static void testVisualFeedbackRemove() {
    std::cout << "\n=== VisualFeedbackQueue: Remove ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    int id = sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Hit");
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "One effect before remove");
    assertTrue(sys.removeEffect("vfq1", id), "Remove succeeds");
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "Zero effects after remove");
}

static void testVisualFeedbackCategory() {
    std::cout << "\n=== VisualFeedbackQueue: Category ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Dmg1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Dmg2");
    sys.queueEffect("vfq1", 1, 1.0f, 2.0f, 1, "Shield");
    assertTrue(sys.getEffectsByCategory("vfq1", 0) == 2, "Two damage effects");
    assertTrue(sys.getEffectsByCategory("vfq1", 1) == 1, "One shield effect");
    assertTrue(sys.getEffectsByCategory("vfq1", 2) == 0, "Zero heal effects");
}

static void testVisualFeedbackMissing() {
    std::cout << "\n=== VisualFeedbackQueue: Missing ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    assertTrue(!sys.createQueue("nonexistent"), "Create fails on missing");
    assertTrue(sys.queueEffect("nonexistent", 0, 1.0f, 1.0f, 1, "X") == -1, "Queue fails on missing");
    assertTrue(!sys.removeEffect("nonexistent", 1), "Remove fails on missing");
    assertTrue(sys.getActiveEffectCount("nonexistent") == 0, "0 effects on missing");
    assertTrue(sys.getHighestPriority("nonexistent") == -1, "-1 priority on missing");
    assertTrue(sys.getTotalQueued("nonexistent") == 0, "0 queued on missing");
    assertTrue(sys.getTotalExpired("nonexistent") == 0, "0 expired on missing");
}


void run_visual_feedback_queue_tests() {
    testVisualFeedbackCreate();
    testVisualFeedbackQueue();
    testVisualFeedbackMultiple();
    testVisualFeedbackExpiry();
    testVisualFeedbackFading();
    testVisualFeedbackPriority();
    testVisualFeedbackOverflow();
    testVisualFeedbackRemove();
    testVisualFeedbackCategory();
    testVisualFeedbackMissing();
}
