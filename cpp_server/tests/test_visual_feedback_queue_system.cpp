// Tests for: VisualFeedbackQueueSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/visual_feedback_queue_system.h"

using namespace atlas;

// ==================== VisualFeedbackQueueSystem Tests ====================

static void testFeedbackQueueCreate() {
    std::cout << "\n=== FeedbackQueue: Create ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");

    assertTrue(sys.createQueue("vfq1"), "Create queue");
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "No active effects");
    assertTrue(sys.getTotalQueued("vfq1") == 0, "Total queued is 0");
    assertTrue(sys.getTotalExpired("vfq1") == 0, "Total expired is 0");
}

static void testFeedbackQueueEffect() {
    std::cout << "\n=== FeedbackQueue: QueueEffect ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    int id = sys.queueEffect("vfq1", 1, 0.8f, 2.0f, 5, "damage_hit");
    assertTrue(id >= 0, "Effect queued with valid ID");
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "1 active effect");
    assertTrue(sys.getTotalQueued("vfq1") == 1, "Total queued is 1");
}

static void testFeedbackQueueMultipleEffects() {
    std::cout << "\n=== FeedbackQueue: MultipleEffects ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    sys.queueEffect("vfq1", 1, 0.5f, 2.0f, 3, "damage");
    sys.queueEffect("vfq1", 2, 1.0f, 3.0f, 5, "shield_ripple");
    sys.queueEffect("vfq1", 1, 0.3f, 1.0f, 1, "minor_hit");

    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "3 active effects");
    assertTrue(sys.getTotalQueued("vfq1") == 3, "Total queued is 3");
}

static void testFeedbackQueueCategoryFilter() {
    std::cout << "\n=== FeedbackQueue: CategoryFilter ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    sys.queueEffect("vfq1", 1, 0.5f, 2.0f, 3, "damage");
    sys.queueEffect("vfq1", 2, 1.0f, 3.0f, 5, "shield");
    sys.queueEffect("vfq1", 1, 0.3f, 1.0f, 1, "damage_2");

    assertTrue(sys.getEffectsByCategory("vfq1", 1) == 2, "2 category-1 effects");
    assertTrue(sys.getEffectsByCategory("vfq1", 2) == 1, "1 category-2 effect");
    assertTrue(sys.getEffectsByCategory("vfq1", 3) == 0, "0 category-3 effects");
}

static void testFeedbackQueueHighestPriority() {
    std::cout << "\n=== FeedbackQueue: HighestPriority ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    sys.queueEffect("vfq1", 1, 0.5f, 2.0f, 3, "low");
    sys.queueEffect("vfq1", 1, 0.8f, 2.0f, 10, "high");
    sys.queueEffect("vfq1", 2, 1.0f, 2.0f, 7, "medium");

    assertTrue(sys.getHighestPriority("vfq1") == 10, "Highest priority is 10");
}

static void testFeedbackQueueHighestPriorityEmpty() {
    std::cout << "\n=== FeedbackQueue: HighestPriorityEmpty ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    assertTrue(sys.getHighestPriority("vfq1") == -1, "Highest priority is -1 when empty");
}

static void testFeedbackQueueRemoveEffect() {
    std::cout << "\n=== FeedbackQueue: RemoveEffect ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    int id = sys.queueEffect("vfq1", 1, 0.5f, 2.0f, 3, "damage");
    assertTrue(sys.removeEffect("vfq1", id), "Remove effect");
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "No active effects after remove");
}

static void testFeedbackQueueRemoveNonexistent() {
    std::cout << "\n=== FeedbackQueue: RemoveNonexistent ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    assertTrue(!sys.removeEffect("vfq1", 999), "Cannot remove nonexistent effect");
}

static void testFeedbackQueueEffectExpiry() {
    std::cout << "\n=== FeedbackQueue: EffectExpiry ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");

    sys.queueEffect("vfq1", 1, 0.5f, 1.0f, 3, "short_effect");
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "1 active effect");

    // Advance past lifetime
    sys.update(1.5f);
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "Effect expired");
    assertTrue(sys.getTotalExpired("vfq1") == 1, "Total expired is 1");
}

static void testFeedbackQueueFadingBeforeExpiry() {
    std::cout << "\n=== FeedbackQueue: FadingBeforeExpiry ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");

    sys.queueEffect("vfq1", 1, 0.5f, 1.0f, 3, "fade_effect");

    // Advance to near expiry (lifetime < 0.3)
    sys.update(0.75f);
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    assertTrue(vfq->effects.size() == 1, "Still active");
    assertTrue(vfq->effects[0].fading, "Effect is fading");
}

static void testFeedbackQueueMaxEffectsEviction() {
    std::cout << "\n=== FeedbackQueue: MaxEffectsEviction ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");

    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    vfq->max_effects = 2;

    sys.queueEffect("vfq1", 1, 0.5f, 5.0f, 1, "low_priority");
    sys.queueEffect("vfq1", 1, 0.5f, 5.0f, 5, "medium_priority");

    // Queue a higher priority effect - should evict lowest
    int id = sys.queueEffect("vfq1", 1, 1.0f, 5.0f, 10, "high_priority");
    assertTrue(id >= 0, "High priority effect queued");
    assertTrue(sys.getActiveEffectCount("vfq1") == 2, "Still 2 effects (lowest evicted)");
    assertTrue(sys.getHighestPriority("vfq1") == 10, "High priority present");
}

static void testFeedbackQueueInactiveSkipsUpdate() {
    std::cout << "\n=== FeedbackQueue: InactiveSkipsUpdate ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");

    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    vfq->active = false;

    sys.queueEffect("vfq1", 1, 0.5f, 0.5f, 3, "short_effect");
    sys.update(2.0f);

    // Effect should NOT expire because queue is inactive
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "Effect not expired while inactive");
}

static void testFeedbackQueueMissingEntity() {
    std::cout << "\n=== FeedbackQueue: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);

    assertTrue(!sys.createQueue("ghost"), "Create fails for missing");
    assertTrue(sys.queueEffect("ghost", 1, 0.5f, 1.0f, 3, "d") == -1, "Queue returns -1 for missing");
    assertTrue(!sys.removeEffect("ghost", 1), "Remove fails for missing");
    assertTrue(sys.getActiveEffectCount("ghost") == 0, "Count 0 for missing");
    assertTrue(sys.getEffectsByCategory("ghost", 1) == 0, "Category 0 for missing");
    assertTrue(sys.getHighestPriority("ghost") == -1, "Priority -1 for missing");
    assertTrue(sys.getTotalQueued("ghost") == 0, "Total queued 0 for missing");
    assertTrue(sys.getTotalExpired("ghost") == 0, "Total expired 0 for missing");
}

void run_visual_feedback_queue_system_tests() {
    testFeedbackQueueCreate();
    testFeedbackQueueEffect();
    testFeedbackQueueMultipleEffects();
    testFeedbackQueueCategoryFilter();
    testFeedbackQueueHighestPriority();
    testFeedbackQueueHighestPriorityEmpty();
    testFeedbackQueueRemoveEffect();
    testFeedbackQueueRemoveNonexistent();
    testFeedbackQueueEffectExpiry();
    testFeedbackQueueFadingBeforeExpiry();
    testFeedbackQueueMaxEffectsEviction();
    testFeedbackQueueInactiveSkipsUpdate();
    testFeedbackQueueMissingEntity();
}
