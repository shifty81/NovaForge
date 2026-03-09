// Tests for: WarpAutoComfortSystem
#include "test_log.h"
#include "components/game_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_auto_comfort_system.h"

using namespace atlas;

// ==================== WarpAutoComfortSystem Tests ====================

static void testComfortReductionNoChange() {
    std::cout << "\n=== WarpAutoComfort: ReductionNoChange ===" << std::endl;
    // FPS between low (48) and high (57) thresholds: no change (hysteresis)
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        52.0f, 60.0f, 0.3f, 0.016f);
    assertTrue(approxEqual(result, 0.3f), "Reduction unchanged in hysteresis band");
}

static void testComfortReductionIncreases() {
    std::cout << "\n=== WarpAutoComfort: ReductionIncreases ===" << std::endl;
    // FPS below low threshold (48): reduction increases
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        40.0f, 60.0f, 0.2f, 1.0f);
    assertTrue(result > 0.2f, "Reduction increased when FPS is low");
}

static void testComfortReductionDecreases() {
    std::cout << "\n=== WarpAutoComfort: ReductionDecreases ===" << std::endl;
    // FPS above high threshold (57): reduction decreases
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        60.0f, 60.0f, 0.5f, 1.0f);
    assertTrue(result < 0.5f, "Reduction decreased when FPS is good");
}

static void testComfortReductionClampedToZero() {
    std::cout << "\n=== WarpAutoComfort: ReductionClampedToZero ===" << std::endl;
    // FPS very high with long delta: should not go below 0
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        120.0f, 60.0f, 0.1f, 10.0f);
    assertTrue(result >= 0.0f, "Reduction clamped to >= 0");
}

static void testComfortReductionClampedToOne() {
    std::cout << "\n=== WarpAutoComfort: ReductionClampedToOne ===" << std::endl;
    // FPS very low with long delta: should not exceed 1
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        5.0f, 60.0f, 0.9f, 10.0f);
    assertTrue(result <= 1.0f, "Reduction clamped to <= 1");
}

static void testComfortReductionZeroTargetFPS() {
    std::cout << "\n=== WarpAutoComfort: ZeroTargetFPS ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        60.0f, 0.0f, 0.5f, 1.0f);
    assertTrue(result == 0.0f, "Zero target FPS returns 0 reduction");
}

static void testApplyComfortFullEffects() {
    std::cout << "\n=== WarpAutoComfort: ApplyComfortFullEffects ===" << std::endl;
    // No reduction: effects unchanged
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.0f, false, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 1.0f), "Motion unchanged at 0 reduction");
    assertTrue(approxEqual(blur, 1.0f), "Blur unchanged at 0 reduction");
}

static void testApplyComfortMaxReduction() {
    std::cout << "\n=== WarpAutoComfort: ApplyComfortMaxReduction ===" << std::endl;
    // Max reduction (1.0): effects scaled to 40%
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(1.0f, false, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 0.4f), "Motion reduced to 40% at max reduction");
    assertTrue(approxEqual(blur, 0.4f), "Blur reduced to 40% at max reduction");
}

static void testApplyComfortHalfReduction() {
    std::cout << "\n=== WarpAutoComfort: ApplyComfortHalfReduction ===" << std::endl;
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.5f, false, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 0.7f), "Motion at 70% with 0.5 reduction");
    assertTrue(approxEqual(blur, 0.7f), "Blur at 70% with 0.5 reduction");
}

static void testApplyComfortUltrawideClamp() {
    std::cout << "\n=== WarpAutoComfort: UltrawideClamp ===" << std::endl;
    float motion = 1.0f, blur = 0.8f;
    systems::WarpAutoComfortSystem::applyComfort(0.0f, true, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 1.0f), "Motion unaffected by ultrawide");
    assertTrue(blur <= 0.5f, "Blur clamped to max_distortion_ultrawide");
}

static void testApplyComfortUltrawideWithReduction() {
    std::cout << "\n=== WarpAutoComfort: UltrawideWithReduction ===" << std::endl;
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.5f, true, 0.3f, motion, blur);
    // 0.5 reduction: blur = 1.0 * 0.7 = 0.7, then clamped to 0.3 (ultrawide)
    assertTrue(blur <= 0.3f, "Blur clamped by ultrawide after reduction");
    assertTrue(approxEqual(motion, 0.7f), "Motion reduced by 0.5 factor");
}

static void testApplyComfortNonNegative() {
    std::cout << "\n=== WarpAutoComfort: NonNegative ===" << std::endl;
    float motion = 0.0f, blur = 0.0f;
    systems::WarpAutoComfortSystem::applyComfort(1.0f, false, 0.5f, motion, blur);
    assertTrue(motion >= 0.0f, "Motion non-negative");
    assertTrue(blur >= 0.0f, "Blur non-negative");
}

static void testWarpAutoComfortUpdateIntegration() {
    std::cout << "\n=== WarpAutoComfort: UpdateIntegration ===" << std::endl;
    ecs::World world;
    systems::WarpAutoComfortSystem sys(&world);
    auto* e = world.createEntity("warp1");
    auto* comfort = addComp<components::WarpAutoComfort>(e);
    auto* access = addComp<components::WarpAccessibility>(e);

    // Low FPS scenario: should increase comfort reduction
    comfort->target_fps = 60.0f;
    comfort->current_fps = 30.0f;
    comfort->comfort_reduction = 0.0f;
    access->motion_intensity = 1.0f;
    access->blur_intensity = 1.0f;

    sys.update(1.0f);

    assertTrue(comfort->comfort_reduction > 0.0f, "Reduction increased from low FPS");
    assertTrue(access->motion_intensity < 1.0f, "Motion reduced by comfort system");
    assertTrue(access->blur_intensity < 1.0f, "Blur reduced by comfort system");
}

static void testWarpAutoComfortHighFPS() {
    std::cout << "\n=== WarpAutoComfort: HighFPS ===" << std::endl;
    ecs::World world;
    systems::WarpAutoComfortSystem sys(&world);
    auto* e = world.createEntity("warp1");
    auto* comfort = addComp<components::WarpAutoComfort>(e);
    auto* access = addComp<components::WarpAccessibility>(e);

    // High FPS: should decrease comfort reduction
    comfort->target_fps = 60.0f;
    comfort->current_fps = 60.0f;
    comfort->comfort_reduction = 0.5f;
    access->motion_intensity = 1.0f;
    access->blur_intensity = 1.0f;

    sys.update(1.0f);

    assertTrue(comfort->comfort_reduction < 0.5f, "Reduction decreased at high FPS");
}

static void testWarpAutoComfortUltrawideDetection() {
    std::cout << "\n=== WarpAutoComfort: UltrawideDetection ===" << std::endl;
    ecs::World world;
    systems::WarpAutoComfortSystem sys(&world);
    auto* e = world.createEntity("warp1");
    auto* comfort = addComp<components::WarpAutoComfort>(e);
    auto* access = addComp<components::WarpAccessibility>(e);

    comfort->target_fps = 60.0f;
    comfort->current_fps = 60.0f;
    comfort->ultrawide_detected = true;
    comfort->max_distortion_ultrawide = 0.4f;
    access->motion_intensity = 1.0f;
    access->blur_intensity = 1.0f;

    sys.update(0.016f);

    assertTrue(access->blur_intensity <= 0.4f, "Blur clamped for ultrawide display");
}

static void testWarpAutoComfortNoAccessibilityComponent() {
    std::cout << "\n=== WarpAutoComfort: NoAccessibilityComponent ===" << std::endl;
    ecs::World world;
    systems::WarpAutoComfortSystem sys(&world);
    auto* e = world.createEntity("warp1");
    auto* comfort = addComp<components::WarpAutoComfort>(e);

    // No WarpAccessibility component - should not crash
    comfort->current_fps = 30.0f;
    comfort->target_fps = 60.0f;

    sys.update(1.0f);
    // Just verify no crash
    assertTrue(true, "Update without accessibility component does not crash");
}

void run_warp_auto_comfort_system_tests() {
    testComfortReductionNoChange();
    testComfortReductionIncreases();
    testComfortReductionDecreases();
    testComfortReductionClampedToZero();
    testComfortReductionClampedToOne();
    testComfortReductionZeroTargetFPS();
    testApplyComfortFullEffects();
    testApplyComfortMaxReduction();
    testApplyComfortHalfReduction();
    testApplyComfortUltrawideClamp();
    testApplyComfortUltrawideWithReduction();
    testApplyComfortNonNegative();
    testWarpAutoComfortUpdateIntegration();
    testWarpAutoComfortHighFPS();
    testWarpAutoComfortUltrawideDetection();
    testWarpAutoComfortNoAccessibilityComponent();
}
