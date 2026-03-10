// Tests for: Reaction Formula System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/reaction_formula_system.h"

using namespace atlas;

// ==================== Reaction Formula System Tests ====================

static void testReactionFormulaCreate() {
    std::cout << "\n=== ReactionFormula: Create ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    assertTrue(sys.initialize("reactor1", "tatara_01"), "Init succeeds");
    assertTrue(sys.getActiveReactionCount("reactor1") == 0, "No active reactions");
    assertTrue(sys.getTotalCompleted("reactor1") == 0, "0 completed");
    assertTrue(approxEqual(sys.getEfficiency("reactor1"), 1.0f), "Efficiency 1.0");
}

static void testReactionFormulaStartReaction() {
    std::cout << "\n=== ReactionFormula: StartReaction ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");

    assertTrue(sys.startReaction("reactor1", "job_1", "formula_unrefined_ferrofluid",
                                 100.0f, "ferrofluid", 50),
               "Start reaction");
    assertTrue(sys.getActiveReactionCount("reactor1") == 1, "1 active");
    assertTrue(approxEqual(sys.getReactionProgress("reactor1", "job_1"), 0.0f),
               "0 progress");
    assertTrue(!sys.isReactionCompleted("reactor1", "job_1"), "Not completed");
}

static void testReactionFormulaAddInputs() {
    std::cout << "\n=== ReactionFormula: AddInputs ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");
    sys.startReaction("reactor1", "job_1", "formula_1", 100.0f, "output_mat", 10);

    assertTrue(sys.addInput("reactor1", "job_1", "moon_goo_a", 100),
               "Add input 1");
    assertTrue(sys.addInput("reactor1", "job_1", "moon_goo_b", 200),
               "Add input 2");
    // Cannot add input to nonexistent job
    assertTrue(!sys.addInput("reactor1", "job_999", "moon_goo_a", 50),
               "Add input to missing job fails");
}

static void testReactionFormulaProgress() {
    std::cout << "\n=== ReactionFormula: Progress ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");
    sys.startReaction("reactor1", "job_1", "formula_1", 100.0f, "output_mat", 10);

    sys.update(50.0f);
    assertTrue(approxEqual(sys.getReactionProgress("reactor1", "job_1"), 50.0f),
               "50s progress");
    assertTrue(!sys.isReactionCompleted("reactor1", "job_1"), "Not completed at 50s");
}

static void testReactionFormulaCompletion() {
    std::cout << "\n=== ReactionFormula: Completion ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");
    sys.startReaction("reactor1", "job_1", "formula_1", 100.0f, "output_mat", 10);

    sys.update(105.0f);
    assertTrue(sys.isReactionCompleted("reactor1", "job_1"), "Completed");
    assertTrue(sys.getTotalCompleted("reactor1") == 1, "1 completed");
    assertTrue(sys.getActiveReactionCount("reactor1") == 0, "0 active after completion");
    // Progress clamped to time_required
    assertTrue(approxEqual(sys.getReactionProgress("reactor1", "job_1"), 100.0f),
               "Progress clamped");
}

static void testReactionFormulaCancelReaction() {
    std::cout << "\n=== ReactionFormula: CancelReaction ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");
    sys.startReaction("reactor1", "job_1", "formula_1", 100.0f, "output_mat", 10);

    assertTrue(sys.cancelReaction("reactor1", "job_1"), "Cancel succeeds");
    assertTrue(sys.getActiveReactionCount("reactor1") == 0, "0 active after cancel");
    assertTrue(!sys.cancelReaction("reactor1", "job_1"), "Can't cancel again");
}

static void testReactionFormulaEfficiency() {
    std::cout << "\n=== ReactionFormula: Efficiency ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");

    sys.setEfficiency("reactor1", 2.0f);
    assertTrue(approxEqual(sys.getEfficiency("reactor1"), 2.0f), "Efficiency set to 2.0");

    // Job with 100s at 2x efficiency should complete in 50s
    sys.startReaction("reactor1", "job_1", "formula_1", 100.0f, "output_mat", 10);
    sys.update(55.0f); // 55 * 2.0 = 110 progress
    assertTrue(sys.isReactionCompleted("reactor1", "job_1"), "Completed with 2x efficiency");
}

static void testReactionFormulaMaxConcurrent() {
    std::cout << "\n=== ReactionFormula: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    world.createEntity("reactor1");
    sys.initialize("reactor1");

    // Max concurrent is 3
    assertTrue(sys.startReaction("reactor1", "j1", "f1", 100.0f), "Start reaction 1");
    assertTrue(sys.startReaction("reactor1", "j2", "f2", 100.0f), "Start reaction 2");
    assertTrue(sys.startReaction("reactor1", "j3", "f3", 100.0f), "Start reaction 3");
    assertTrue(!sys.startReaction("reactor1", "j4", "f4", 100.0f),
               "Reaction 4 rejected (max)");
    assertTrue(sys.getActiveReactionCount("reactor1") == 3, "3 active reactions");
}

static void testReactionFormulaMissing() {
    std::cout << "\n=== ReactionFormula: Missing ===" << std::endl;
    ecs::World world;
    systems::ReactionFormulaSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.startReaction("nonexistent", "j1", "f1"),
               "Start fails on missing");
    assertTrue(!sys.addInput("nonexistent", "j1", "mat1", 100),
               "AddInput fails on missing");
    assertTrue(!sys.cancelReaction("nonexistent", "j1"), "Cancel fails on missing");
    assertTrue(!sys.setEfficiency("nonexistent", 2.0f), "SetEfficiency fails on missing");
    assertTrue(sys.getActiveReactionCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getReactionProgress("nonexistent", "j1"), 0.0f),
               "0 progress on missing");
    assertTrue(!sys.isReactionCompleted("nonexistent", "j1"), "Not completed on missing");
    assertTrue(approxEqual(sys.getEfficiency("nonexistent"), 0.0f), "0 efficiency on missing");
}

void run_reaction_formula_system_tests() {
    testReactionFormulaCreate();
    testReactionFormulaStartReaction();
    testReactionFormulaAddInputs();
    testReactionFormulaProgress();
    testReactionFormulaCompletion();
    testReactionFormulaCancelReaction();
    testReactionFormulaEfficiency();
    testReactionFormulaMaxConcurrent();
    testReactionFormulaMissing();
}
