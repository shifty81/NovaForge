// Tests for: MutaplasmidSystem
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/mutaplasmid_system.h"

using namespace atlas;
using Grade = components::MutaplasmidState::Grade;
using StatRoll = components::MutaplasmidState::StatRoll;

// ==================== MutaplasmidSystem Tests ====================

static void testMutaplasmidCreate() {
    std::cout << "\n=== Mutaplasmid: Create ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    assertTrue(sys.initialize("lab1", "abyssal_lab_01"), "Init succeeds");
    assertTrue(sys.getMutationCount("lab1") == 0, "0 mutations");
    assertTrue(sys.getTotalAttempted("lab1") == 0, "0 attempted");
    assertTrue(sys.getTotalCreated("lab1") == 0, "0 created");
}

static void testMutaplasmidQueueAndFinalize() {
    std::cout << "\n=== Mutaplasmid: QueueAndFinalize ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    StatRoll r1;
    r1.stat_name = "damage_modifier";
    r1.min_multiplier = 0.85f;
    r1.max_multiplier = 1.15f;
    stats.push_back(r1);

    assertTrue(sys.queueMutation("lab1", "damage_mod_ii", Grade::Unstable, stats),
               "Queue mutation");
    assertTrue(sys.getMutationCount("lab1") == 1, "1 pending mutation");
    assertTrue(sys.getTotalAttempted("lab1") == 1, "1 attempted");
    assertTrue(!sys.isMutationCreated("lab1", "damage_mod_ii"), "Not yet created");

    // Apply a roll at the maximum
    assertTrue(sys.applyRoll("lab1", "damage_mod_ii", 0, 1.15f), "Apply roll");

    // Finalize
    assertTrue(sys.finalizeMutation("lab1", "damage_mod_ii"), "Finalize");
    assertTrue(sys.isMutationCreated("lab1", "damage_mod_ii"), "Mutation created");
    assertTrue(sys.getTotalCreated("lab1") == 1, "1 created");
    assertTrue(approxEqual(sys.getOverallQuality("lab1", "damage_mod_ii"), 1.0f),
               "Quality 1.0 (max roll)");
}

static void testMutaplasmidQualityMin() {
    std::cout << "\n=== Mutaplasmid: QualityMin ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    StatRoll r;
    r.stat_name = "shield_hp";
    r.min_multiplier = 0.9f;
    r.max_multiplier = 1.1f;
    stats.push_back(r);

    sys.queueMutation("lab1", "shield_amp_ii", Grade::Decayed, stats);
    sys.applyRoll("lab1", "shield_amp_ii", 0, 0.9f);   // minimum roll
    sys.finalizeMutation("lab1", "shield_amp_ii");

    assertTrue(approxEqual(sys.getOverallQuality("lab1", "shield_amp_ii"), 0.0f),
               "Quality 0.0 (min roll)");
}

static void testMutaplasmidQualityMid() {
    std::cout << "\n=== Mutaplasmid: QualityMid ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    StatRoll r;
    r.stat_name = "activation_cost";
    r.min_multiplier = 0.8f;
    r.max_multiplier = 1.2f;
    stats.push_back(r);

    sys.queueMutation("lab1", "cap_mod_ii", Grade::Gravid, stats);
    sys.applyRoll("lab1", "cap_mod_ii", 0, 1.0f);   // midpoint roll
    sys.finalizeMutation("lab1", "cap_mod_ii");

    // quality = (1.0 - 0.8) / (1.2 - 0.8) = 0.5
    assertTrue(approxEqual(sys.getOverallQuality("lab1", "cap_mod_ii"), 0.5f),
               "Quality 0.5 (mid roll)");
}

static void testMutaplasmidMultipleStats() {
    std::cout << "\n=== Mutaplasmid: MultipleStats ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    for (int i = 0; i < 3; i++) {
        StatRoll r;
        r.stat_name = "stat_" + std::to_string(i);
        r.min_multiplier = 0.0f;
        r.max_multiplier = 1.0f;
        stats.push_back(r);
    }

    sys.queueMutation("lab1", "complex_mod", Grade::Overloaded, stats);

    // Roll: 0.0, 0.5, 1.0 → average quality = (0 + 0.5 + 1.0) / 3 ≈ 0.5
    sys.applyRoll("lab1", "complex_mod", 0, 0.0f);
    sys.applyRoll("lab1", "complex_mod", 1, 0.5f);
    sys.applyRoll("lab1", "complex_mod", 2, 1.0f);
    sys.finalizeMutation("lab1", "complex_mod");

    float q = sys.getOverallQuality("lab1", "complex_mod");
    assertTrue(q > 0.49f && q < 0.51f, "Quality ~0.5 for mixed rolls");
}

static void testMutaplasmidFinalizeBeforeRollFails() {
    std::cout << "\n=== Mutaplasmid: FinalizeBeforeRollFails ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    StatRoll r;
    r.stat_name = "damage";
    r.min_multiplier = 0.9f;
    r.max_multiplier = 1.1f;
    stats.push_back(r);

    sys.queueMutation("lab1", "unrolled_mod", Grade::Unstable, stats);
    // Don't apply any rolls
    assertTrue(!sys.finalizeMutation("lab1", "unrolled_mod"),
               "Finalize fails without rolls");
    assertTrue(!sys.isMutationCreated("lab1", "unrolled_mod"), "Not created");
}

static void testMutaplasmidMaxPending() {
    std::cout << "\n=== Mutaplasmid: MaxPending ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    StatRoll r;
    r.stat_name = "s";
    r.min_multiplier = 0.9f;
    r.max_multiplier = 1.1f;
    stats.push_back(r);

    for (int i = 0; i < 5; i++) {
        assertTrue(sys.queueMutation("lab1", "mod_" + std::to_string(i),
                                     Grade::Unstable, stats),
                   "Queue mod " + std::to_string(i));
    }
    assertTrue(!sys.queueMutation("lab1", "mod_overflow", Grade::Unstable, stats),
               "Overflow rejected");
    assertTrue(sys.getMutationCount("lab1") == 5, "5 pending (max)");
}

static void testMutaplasmidRollClamping() {
    std::cout << "\n=== Mutaplasmid: RollClamping ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    std::vector<StatRoll> stats;
    StatRoll r;
    r.stat_name = "speed";
    r.min_multiplier = 0.8f;
    r.max_multiplier = 1.2f;
    stats.push_back(r);

    sys.queueMutation("lab1", "prop_mod", Grade::Gravid, stats);
    // Roll above max → should clamp to max
    sys.applyRoll("lab1", "prop_mod", 0, 9.9f);
    sys.finalizeMutation("lab1", "prop_mod");
    assertTrue(approxEqual(sys.getOverallQuality("lab1", "prop_mod"), 1.0f),
               "Clamped to max → quality 1.0");
}

static void testMutaplasmidMissing() {
    std::cout << "\n=== Mutaplasmid: Missing ===" << std::endl;
    ecs::World world;
    systems::MutaplasmidSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    std::vector<StatRoll> stats;
    assertTrue(!sys.queueMutation("nonexistent", "mod1", Grade::Unstable, stats),
               "Queue fails on missing");
    assertTrue(!sys.applyRoll("nonexistent", "mod1", 0, 1.0f),
               "ApplyRoll fails on missing");
    assertTrue(!sys.finalizeMutation("nonexistent", "mod1"),
               "Finalize fails on missing");
    assertTrue(sys.getMutationCount("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getTotalAttempted("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getTotalCreated("nonexistent") == 0, "0 on missing");
    assertTrue(approxEqual(sys.getOverallQuality("nonexistent", "mod1"), 0.0f),
               "0.0 quality on missing");
    assertTrue(!sys.isMutationCreated("nonexistent", "mod1"),
               "Not created on missing");
}

void run_mutaplasmid_system_tests() {
    testMutaplasmidCreate();
    testMutaplasmidQueueAndFinalize();
    testMutaplasmidQualityMin();
    testMutaplasmidQualityMid();
    testMutaplasmidMultipleStats();
    testMutaplasmidFinalizeBeforeRollFails();
    testMutaplasmidMaxPending();
    testMutaplasmidRollClamping();
    testMutaplasmidMissing();
}
