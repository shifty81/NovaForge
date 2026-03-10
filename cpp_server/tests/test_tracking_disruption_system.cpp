// Tests for: TrackingDisruptionSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/tracking_disruption_system.h"

using namespace atlas;

// ==================== TrackingDisruptionSystem Tests ====================

static void testTrackingDisruptionInit() {
    std::cout << "\n=== TrackingDisruption: Init ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f), "Init succeeds");
    assertTrue(sys.getTrackingDisruptorCount("ship1") == 0, "Zero tracking disruptors");
    assertTrue(sys.getGuidanceDisruptorCount("ship1") == 0, "Zero guidance disruptors");
    assertTrue(!sys.isDisrupted("ship1"), "Not disrupted initially");
    assertTrue(approxEqual(sys.getEffectiveTrackingSpeed("ship1"), 1.0f),
               "Tracking speed at base");
    assertTrue(approxEqual(sys.getEffectiveOptimalRange("ship1"), 50.0f),
               "Optimal range at base");
    assertTrue(approxEqual(sys.getEffectiveExplosionRadius("ship1"), 40.0f),
               "Explosion radius at base");
    assertTrue(approxEqual(sys.getEffectiveExplosionVelocity("ship1"), 200.0f),
               "Explosion velocity at base");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testTrackingDisruptionApplyTracking() {
    std::cout << "\n=== TrackingDisruption: ApplyTrackingDisruptor ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);

    assertTrue(sys.applyTrackingDisruptor("ship1", "e1", 0.30f, 0.30f, 5.0f),
               "Apply tracking disruptor succeeds");
    assertTrue(sys.getTrackingDisruptorCount("ship1") == 1, "1 tracking disruptor");
    assertTrue(sys.isDisrupted("ship1"), "Disrupted after apply");
    assertTrue(sys.getTotalTrackingDisruptorsApplied("ship1") == 1, "1 total tracking applied");

    // Effective tracking speed: 1.0 * (1 - 0.30) = 0.70
    assertTrue(approxEqual(sys.getEffectiveTrackingSpeed("ship1"), 0.70f),
               "Tracking speed reduced to 0.70");
    // Effective optimal range: 50 * (1 - 0.30) = 35
    assertTrue(approxEqual(sys.getEffectiveOptimalRange("ship1"), 35.0f),
               "Optimal range reduced to 35");
}

static void testTrackingDisruptionApplyGuidance() {
    std::cout << "\n=== TrackingDisruption: ApplyGuidanceDisruptor ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);

    assertTrue(sys.applyGuidanceDisruptor("ship1", "e1", 0.50f, 0.30f, 5.0f),
               "Apply guidance disruptor succeeds");
    assertTrue(sys.getGuidanceDisruptorCount("ship1") == 1, "1 guidance disruptor");
    assertTrue(sys.isDisrupted("ship1"), "Disrupted after guidance apply");
    assertTrue(sys.getTotalGuidanceDisruptorsApplied("ship1") == 1, "1 total guidance applied");

    // Explosion radius: 40 * (1 + 0.50) = 60
    assertTrue(approxEqual(sys.getEffectiveExplosionRadius("ship1"), 60.0f),
               "Explosion radius increased to 60");
    // Explosion velocity: 200 * (1 - 0.30) = 140
    assertTrue(approxEqual(sys.getEffectiveExplosionVelocity("ship1"), 140.0f),
               "Explosion velocity reduced to 140");
}

static void testTrackingDisruptionMultipleTracking() {
    std::cout << "\n=== TrackingDisruption: MultipleTrackingDisruptors ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);

    sys.applyTrackingDisruptor("ship1", "e1", 0.20f, 0.20f, 5.0f);
    sys.applyTrackingDisruptor("ship1", "e2", 0.10f, 0.10f, 5.0f);

    assertTrue(sys.getTrackingDisruptorCount("ship1") == 2, "2 tracking disruptors");
    // Additive: (0.20+0.10) = 0.30 total reduction
    // 1.0 * (1 - 0.30) = 0.70
    assertTrue(approxEqual(sys.getEffectiveTrackingSpeed("ship1"), 0.70f),
               "Tracking speed additive: 1.0*(1-0.30)=0.70");
    // 50 * (1 - 0.30) = 35
    assertTrue(approxEqual(sys.getEffectiveOptimalRange("ship1"), 35.0f),
               "Optimal range additive: 50*(1-0.30)=35");
}

static void testTrackingDisruptionDuplicatePrevention() {
    std::cout << "\n=== TrackingDisruption: DuplicatePrevention ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);

    sys.applyTrackingDisruptor("ship1", "e1", 0.30f, 0.30f, 5.0f);
    assertTrue(!sys.applyTrackingDisruptor("ship1", "e1", 0.10f, 0.10f, 5.0f),
               "Duplicate tracking disruptor source rejected");
    sys.applyGuidanceDisruptor("ship1", "e2", 0.30f, 0.30f, 5.0f);
    assertTrue(!sys.applyGuidanceDisruptor("ship1", "e2", 0.10f, 0.10f, 5.0f),
               "Duplicate guidance disruptor source rejected");
    assertTrue(sys.getTrackingDisruptorCount("ship1") == 1, "1 tracking disruptor");
    assertTrue(sys.getGuidanceDisruptorCount("ship1") == 1, "1 guidance disruptor");
}

static void testTrackingDisruptionValidation() {
    std::cout << "\n=== TrackingDisruption: Validation ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);

    assertTrue(!sys.applyTrackingDisruptor("ship1", "",    0.30f, 0.30f, 5.0f), "Empty source rejected");
    assertTrue(!sys.applyTrackingDisruptor("ship1", "e1", -0.1f, 0.30f, 5.0f), "Negative track red rejected");
    assertTrue(!sys.applyTrackingDisruptor("ship1", "e1",  0.30f, -0.1f, 5.0f),"Negative range red rejected");
    assertTrue(!sys.applyTrackingDisruptor("ship1", "e1",  0.30f, 0.30f, 0.0f),"Zero cycle time rejected");

    assertTrue(!sys.applyGuidanceDisruptor("ship1", "",    0.30f, 0.30f, 5.0f), "Guidance empty source rejected");
    assertTrue(!sys.applyGuidanceDisruptor("ship1", "e1", -0.1f, 0.30f, 5.0f), "Negative radius inc rejected");
    assertTrue(!sys.applyGuidanceDisruptor("ship1", "e1",  0.30f, -0.1f, 5.0f),"Negative vel red rejected");
    assertTrue(!sys.applyGuidanceDisruptor("ship1", "e1",  0.30f, 0.30f, 0.0f),"Guidance zero cycle rejected");
    assertTrue(sys.getTrackingDisruptorCount("ship1") == 0, "No disruptors after failed validates");
    assertTrue(sys.getGuidanceDisruptorCount("ship1") == 0, "No guidance after failed validates");
}

static void testTrackingDisruptionRemoveTracking() {
    std::cout << "\n=== TrackingDisruption: RemoveTrackingDisruptor ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);
    sys.applyTrackingDisruptor("ship1", "e1", 0.30f, 0.30f, 5.0f);
    sys.applyTrackingDisruptor("ship1", "e2", 0.20f, 0.20f, 5.0f);

    assertTrue(sys.removeTrackingDisruptor("ship1", "e1"), "Remove tracking disruptor succeeds");
    assertTrue(sys.getTrackingDisruptorCount("ship1") == 1, "1 tracking disruptor remaining");
    // Only e2 left: 1.0*(1-0.20)=0.80, 50*(1-0.20)=40
    assertTrue(approxEqual(sys.getEffectiveTrackingSpeed("ship1"), 0.80f),
               "Tracking speed recalculated to 0.80");
    assertTrue(approxEqual(sys.getEffectiveOptimalRange("ship1"), 40.0f),
               "Optimal range recalculated to 40");

    assertTrue(!sys.removeTrackingDisruptor("ship1", "nonexistent"), "Remove nonexistent fails");
}

static void testTrackingDisruptionRemoveGuidance() {
    std::cout << "\n=== TrackingDisruption: RemoveGuidanceDisruptor ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);
    sys.applyGuidanceDisruptor("ship1", "e1", 0.50f, 0.30f, 5.0f);

    assertTrue(sys.removeGuidanceDisruptor("ship1", "e1"), "Remove guidance disruptor succeeds");
    assertTrue(sys.getGuidanceDisruptorCount("ship1") == 0, "0 guidance disruptors");
    assertTrue(!sys.isDisrupted("ship1"), "Not disrupted after all removed");
    assertTrue(approxEqual(sys.getEffectiveExplosionRadius("ship1"), 40.0f),
               "Explosion radius restored");
    assertTrue(approxEqual(sys.getEffectiveExplosionVelocity("ship1"), 200.0f),
               "Explosion velocity restored");
}

static void testTrackingDisruptionClear() {
    std::cout << "\n=== TrackingDisruption: Clear ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);
    sys.applyTrackingDisruptor("ship1", "e1", 0.30f, 0.30f, 5.0f);
    sys.applyGuidanceDisruptor("ship1", "e2", 0.50f, 0.30f, 5.0f);

    assertTrue(sys.clearTrackingDisruptors("ship1"), "Clear tracking disruptors succeeds");
    assertTrue(sys.getTrackingDisruptorCount("ship1") == 0, "0 tracking disruptors after clear");
    assertTrue(approxEqual(sys.getEffectiveTrackingSpeed("ship1"), 1.0f),
               "Tracking speed restored after clear");

    assertTrue(sys.clearGuidanceDisruptors("ship1"), "Clear guidance disruptors succeeds");
    assertTrue(sys.getGuidanceDisruptorCount("ship1") == 0, "0 guidance disruptors after clear");
    assertTrue(!sys.isDisrupted("ship1"), "Not disrupted after all clears");
}

static void testTrackingDisruptionCycleTick() {
    std::cout << "\n=== TrackingDisruption: CycleTick ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1.0f, 50.0f, 40.0f, 200.0f);
    sys.applyTrackingDisruptor("ship1", "e1", 0.30f, 0.30f, 4.0f);
    sys.applyGuidanceDisruptor("ship1", "e2", 0.30f, 0.30f, 4.0f);

    sys.update(4.1f);
    auto* comp = world.getEntity("ship1")->getComponent<components::TrackingDisruptionState>();
    assertTrue(approxEqual(comp->tracking_disruptors[0].cycle_elapsed, 0.1f, 0.05f),
               "Tracking disruptor cycle elapsed reset after cycle");
    assertTrue(approxEqual(comp->guidance_disruptors[0].cycle_elapsed, 0.1f, 0.05f),
               "Guidance disruptor cycle elapsed reset after cycle");
}

static void testTrackingDisruptionMissing() {
    std::cout << "\n=== TrackingDisruption: Missing ===" << std::endl;
    ecs::World world;
    systems::TrackingDisruptionSystem sys(&world);

    assertTrue(!sys.applyTrackingDisruptor("nx", "e1", 0.3f, 0.3f, 5.0f),
               "ApplyTracking fails on missing entity");
    assertTrue(!sys.removeTrackingDisruptor("nx", "e1"),
               "RemoveTracking fails on missing entity");
    assertTrue(!sys.clearTrackingDisruptors("nx"),
               "ClearTracking fails on missing entity");
    assertTrue(!sys.applyGuidanceDisruptor("nx", "e1", 0.3f, 0.3f, 5.0f),
               "ApplyGuidance fails on missing entity");
    assertTrue(!sys.removeGuidanceDisruptor("nx", "e1"),
               "RemoveGuidance fails on missing entity");
    assertTrue(!sys.clearGuidanceDisruptors("nx"),
               "ClearGuidance fails on missing entity");
    assertTrue(!sys.isDisrupted("nx"),                                "isDisrupted false on missing");
    assertTrue(sys.getTrackingDisruptorCount("nx") == 0,              "0 tracking disruptors on missing");
    assertTrue(sys.getGuidanceDisruptorCount("nx") == 0,              "0 guidance disruptors on missing");
    assertTrue(approxEqual(sys.getEffectiveTrackingSpeed("nx"), 0.0f),"0 tracking speed on missing");
    assertTrue(approxEqual(sys.getEffectiveOptimalRange("nx"), 0.0f), "0 optimal range on missing");
    assertTrue(sys.getTotalTrackingDisruptorsApplied("nx") == 0,      "0 total tracking on missing");
    assertTrue(sys.getTotalGuidanceDisruptorsApplied("nx") == 0,      "0 total guidance on missing");
}

void run_tracking_disruption_system_tests() {
    testTrackingDisruptionInit();
    testTrackingDisruptionApplyTracking();
    testTrackingDisruptionApplyGuidance();
    testTrackingDisruptionMultipleTracking();
    testTrackingDisruptionDuplicatePrevention();
    testTrackingDisruptionValidation();
    testTrackingDisruptionRemoveTracking();
    testTrackingDisruptionRemoveGuidance();
    testTrackingDisruptionClear();
    testTrackingDisruptionCycleTick();
    testTrackingDisruptionMissing();
}
