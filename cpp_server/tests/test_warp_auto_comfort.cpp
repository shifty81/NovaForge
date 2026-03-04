// Tests for: WarpAutoComfort Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_auto_comfort_system.h"

using namespace atlas;

// ==================== WarpAutoComfort Tests ====================

static void testWarpAutoComfortDefaults() {
    std::cout << "\n=== WarpAutoComfort defaults ===" << std::endl;
    components::WarpAutoComfort c;
    assertTrue(approxEqual(c.target_fps, 60.0f), "target_fps default 60");
    assertTrue(approxEqual(c.current_fps, 60.0f), "current_fps default 60");
    assertTrue(approxEqual(c.comfort_reduction, 0.0f), "comfort_reduction default 0");
    assertTrue(!c.ultrawide_detected, "ultrawide default false");
    assertTrue(approxEqual(c.max_distortion_ultrawide, 0.5f), "max_distortion_uw default 0.5");
}

static void testComfortReductionIncreasesOnLowFPS() {
    std::cout << "\n=== Comfort reduction increases on low FPS ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        30.0f, 60.0f, 0.0f, 1.0f);
    assertTrue(result > 0.0f, "reduction increased on low FPS");
}

static void testComfortReductionDecreasesOnGoodFPS() {
    std::cout << "\n=== Comfort reduction decreases on good FPS ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        60.0f, 60.0f, 0.5f, 1.0f);
    assertTrue(result < 0.5f, "reduction decreased on good FPS");
}

static void testComfortReductionClamped() {
    std::cout << "\n=== Comfort reduction clamped ===" << std::endl;
    float r = 0.0f;
    for (int i = 0; i < 100; ++i) {
        r = systems::WarpAutoComfortSystem::computeComfortReduction(20.0f, 60.0f, r, 0.5f);
    }
    assertTrue(r <= 1.0f && r >= 0.0f, "reduction stays in [0,1]");
}

static void testComfortHysteresis() {
    std::cout << "\n=== Comfort hysteresis ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        52.0f, 60.0f, 0.3f, 1.0f);
    assertTrue(approxEqual(result, 0.3f), "no change in hysteresis band");
}

static void testApplyComfortReducesIntensity() {
    std::cout << "\n=== Apply comfort reduces intensity ===" << std::endl;
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.5f, false, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 0.7f), "motion scaled to 0.7");
    assertTrue(approxEqual(blur, 0.7f), "blur scaled to 0.7");
}

static void testApplyComfortUltrawideClamp() {
    std::cout << "\n=== Apply comfort ultrawide clamp ===" << std::endl;
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.0f, true, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 1.0f), "motion unchanged without FPS reduction");
    assertTrue(approxEqual(blur, 0.5f), "blur clamped to 0.5 on ultrawide");
}

static void testApplyComfortCombined() {
    std::cout << "\n=== Apply comfort combined ===" << std::endl;
    float motion = 1.0f, blur = 0.8f;
    systems::WarpAutoComfortSystem::applyComfort(0.3f, true, 0.5f, motion, blur);
    assertTrue(motion < 1.0f, "motion reduced by comfort");
    assertTrue(blur <= 0.5f, "blur clamped by ultrawide after comfort reduction");
}

static void testAutoComfortSystemReducesOnLowFPS() {
    std::cout << "\n=== AutoComfort system reduces on low FPS ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_auto_comfort");
    auto* comfort = addComp<components::WarpAutoComfort>(e);
    comfort->target_fps = 60.0f;
    comfort->current_fps = 30.0f;
    auto* access = addComp<components::WarpAccessibility>(e);
    access->motion_intensity = 1.0f;
    access->blur_intensity = 1.0f;

    systems::WarpAutoComfortSystem sys(&world);
    for (int i = 0; i < 10; ++i) {
        access->motion_intensity = 1.0f;
        access->blur_intensity = 1.0f;
        sys.update(0.033f);
    }

    assertTrue(comfort->comfort_reduction > 0.0f, "comfort_reduction increased");
    assertTrue(access->motion_intensity < 1.0f, "motion reduced after system tick");
}


void run_warp_auto_comfort_tests() {
    testWarpAutoComfortDefaults();
    testComfortReductionIncreasesOnLowFPS();
    testComfortReductionDecreasesOnGoodFPS();
    testComfortReductionClamped();
    testComfortHysteresis();
    testApplyComfortReducesIntensity();
    testApplyComfortUltrawideClamp();
    testApplyComfortCombined();
    testAutoComfortSystemReducesOnLowFPS();
}
