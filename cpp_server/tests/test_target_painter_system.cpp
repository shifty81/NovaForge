// Tests for: TargetPainterSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/target_painter_system.h"

using namespace atlas;

// ==================== TargetPainterSystem Tests ====================

static void testPainterInit() {
    std::cout << "\n=== TargetPainter: Init ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 75.0f), "Init succeeds");
    assertTrue(sys.getPainterCount("ship1") == 0, "Zero painters initially");
    assertTrue(approxEqual(sys.getBaseSignature("ship1"), 75.0f), "Base signature stored");
    assertTrue(approxEqual(sys.getEffectiveSignature("ship1"), 75.0f), "Effective equals base initially");
    assertTrue(!sys.isPainted("ship1"), "Not painted initially");
    assertTrue(sys.getTotalPaintersApplied("ship1") == 0, "Zero total applied initially");
}

static void testPainterInitFails() {
    std::cout << "\n=== TargetPainter: InitFails ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 50.0f), "Fails on missing entity");
    world.createEntity("ship1");
    assertTrue(!sys.initialize("ship1", 0.0f), "Fails with zero signature");
    assertTrue(!sys.initialize("ship1", -10.0f), "Fails with negative signature");
}

static void testPainterApply() {
    std::cout << "\n=== TargetPainter: Apply ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(sys.applyPainter("ship1", "p1", "src1", 0.3f, 5.0f), "Apply first painter");
    assertTrue(sys.getPainterCount("ship1") == 1, "One painter");
    assertTrue(sys.isPainted("ship1"), "Is painted after apply");
    assertTrue(sys.getTotalPaintersApplied("ship1") == 1, "Total applied incremented");

    assertTrue(sys.applyPainter("ship1", "p2", "src2", 0.25f, 5.0f), "Apply second painter");
    assertTrue(sys.getPainterCount("ship1") == 2, "Two painters");
    assertTrue(sys.getTotalPaintersApplied("ship1") == 2, "Total applied = 2");
}

static void testPainterApplyValidation() {
    std::cout << "\n=== TargetPainter: ApplyValidation ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(!sys.applyPainter("ship1", "", "src1", 0.3f, 5.0f), "Empty painter_id rejected");
    assertTrue(!sys.applyPainter("ship1", "p1", "", 0.3f, 5.0f), "Empty source_id rejected");
    assertTrue(!sys.applyPainter("ship1", "p1", "src1", 0.0f, 5.0f), "Zero strength rejected");
    assertTrue(!sys.applyPainter("ship1", "p1", "src1", -0.1f, 5.0f), "Negative strength rejected");
    assertTrue(!sys.applyPainter("ship1", "p1", "src1", 1.1f, 5.0f), "Strength > 1 rejected");
    assertTrue(!sys.applyPainter("ship1", "p1", "src1", 0.3f, 0.0f), "Zero cycle_time rejected");
    assertTrue(!sys.applyPainter("ship1", "p1", "src1", 0.3f, -1.0f), "Negative cycle_time rejected");

    // Duplicate
    sys.applyPainter("ship1", "p1", "src1", 0.3f, 5.0f);
    assertTrue(!sys.applyPainter("ship1", "p1", "src2", 0.25f, 5.0f), "Duplicate painter_id rejected");
}

static void testPainterRemove() {
    std::cout << "\n=== TargetPainter: Remove ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    sys.applyPainter("ship1", "p1", "src1", 0.3f, 5.0f);
    sys.applyPainter("ship1", "p2", "src2", 0.25f, 5.0f);

    assertTrue(sys.removePainter("ship1", "p1"), "Remove first painter");
    assertTrue(sys.getPainterCount("ship1") == 1, "One painter remaining");
    assertTrue(sys.isPainted("ship1"), "Still painted");
    assertTrue(!sys.removePainter("ship1", "nonexistent"), "Remove unknown fails");

    assertTrue(sys.removePainter("ship1", "p2"), "Remove last painter");
    assertTrue(sys.getPainterCount("ship1") == 0, "Zero painters");
    assertTrue(!sys.isPainted("ship1"), "Not painted after all removed");
}

static void testPainterSignatureCalc() {
    std::cout << "\n=== TargetPainter: SignatureCalc ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    // Single 30% painter → effective = 50 * (1 + 0.3) = 65
    sys.applyPainter("ship1", "p1", "src1", 0.3f, 5.0f);
    assertTrue(approxEqual(sys.getEffectiveSignature("ship1"), 65.0f, 1.0f), "30% painter: 65m sig");

    // Second 50% painter → effective = 50 * 1.3 * 1.5 = 97.5
    sys.applyPainter("ship1", "p2", "src2", 0.5f, 5.0f);
    assertTrue(approxEqual(sys.getEffectiveSignature("ship1"), 97.5f, 1.0f), "Stacked painters: 97.5m");

    // Remove first → effective = 50 * 1.5 = 75
    sys.removePainter("ship1", "p1");
    assertTrue(approxEqual(sys.getEffectiveSignature("ship1"), 75.0f, 1.0f), "After remove: 75m");
}

static void testPainterSetBaseSignature() {
    std::cout << "\n=== TargetPainter: SetBaseSignature ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    sys.applyPainter("ship1", "p1", "src1", 0.5f, 5.0f);

    assertTrue(sys.setBaseSignature("ship1", 100.0f), "Set base signature");
    assertTrue(approxEqual(sys.getBaseSignature("ship1"), 100.0f), "Base updated");
    assertTrue(approxEqual(sys.getEffectiveSignature("ship1"), 150.0f, 1.0f), "Effective recomputed");

    assertTrue(!sys.setBaseSignature("ship1", 0.0f), "Zero signature rejected");
    assertTrue(!sys.setBaseSignature("ship1", -10.0f), "Negative signature rejected");
}

static void testPainterClearAll() {
    std::cout << "\n=== TargetPainter: ClearAll ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    sys.applyPainter("ship1", "p1", "src1", 0.3f, 5.0f);
    sys.applyPainter("ship1", "p2", "src2", 0.25f, 5.0f);

    assertTrue(sys.getPainterCount("ship1") == 2, "Two painters before clear");
    assertTrue(sys.clearPainters("ship1"), "Clear succeeds");
    assertTrue(sys.getPainterCount("ship1") == 0, "Zero painters after clear");
    assertTrue(!sys.isPainted("ship1"), "Not painted after clear");
    assertTrue(approxEqual(sys.getEffectiveSignature("ship1"), 50.0f), "Signature restored to base");
}

static void testPainterCapacity() {
    std::cout << "\n=== TargetPainter: Capacity ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    auto* comp = world.getEntity("ship1")->getComponent<components::TargetPainterState>();
    comp->max_painters = 2;

    assertTrue(sys.applyPainter("ship1", "p1", "s1", 0.3f, 5.0f), "First painter ok");
    assertTrue(sys.applyPainter("ship1", "p2", "s2", 0.3f, 5.0f), "Second painter ok");
    assertTrue(!sys.applyPainter("ship1", "p3", "s3", 0.3f, 5.0f), "Third painter rejected at cap");
    assertTrue(sys.getPainterCount("ship1") == 2, "Still at 2");
}

static void testPainterTick() {
    std::cout << "\n=== TargetPainter: Tick ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    sys.applyPainter("ship1", "p1", "src1", 0.3f, 5.0f);

    sys.update(3.0f);
    auto* comp = world.getEntity("ship1")->getComponent<components::TargetPainterState>();
    assertTrue(comp != nullptr, "Component exists");
    assertTrue(approxEqual(comp->painters[0].cycle_elapsed, 3.0f), "Cycle elapsed after 3s");

    sys.update(3.0f);
    assertTrue(approxEqual(comp->painters[0].cycle_elapsed, 1.0f), "Elapsed wraps after full cycle (6-5=1)");
}

static void testPainterMissing() {
    std::cout << "\n=== TargetPainter: Missing ===" << std::endl;
    ecs::World world;
    systems::TargetPainterSystem sys(&world);

    assertTrue(!sys.applyPainter("nonexistent", "p1", "s1", 0.3f, 5.0f), "Apply fails on missing");
    assertTrue(!sys.removePainter("nonexistent", "p1"), "Remove fails on missing");
    assertTrue(!sys.clearPainters("nonexistent"), "Clear fails on missing");
    assertTrue(!sys.setBaseSignature("nonexistent", 50.0f), "SetBase fails on missing");
    assertTrue(approxEqual(sys.getEffectiveSignature("nonexistent"), 0.0f), "Zero effective on missing");
    assertTrue(approxEqual(sys.getBaseSignature("nonexistent"), 0.0f), "Zero base on missing");
    assertTrue(sys.getPainterCount("nonexistent") == 0, "Zero painters on missing");
    assertTrue(sys.getActivePainterCount("nonexistent") == 0, "Zero active on missing");
    assertTrue(!sys.isPainted("nonexistent"), "Not painted on missing");
    assertTrue(sys.getTotalPaintersApplied("nonexistent") == 0, "Zero total on missing");
}

void run_target_painter_system_tests() {
    testPainterInit();
    testPainterInitFails();
    testPainterApply();
    testPainterApplyValidation();
    testPainterRemove();
    testPainterSignatureCalc();
    testPainterSetBaseSignature();
    testPainterClearAll();
    testPainterCapacity();
    testPainterTick();
    testPainterMissing();
}
