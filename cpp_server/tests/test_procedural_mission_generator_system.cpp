// Tests for: ProceduralMissionGenerator System Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/mission_generator_system.h"
#include "systems/procedural_mission_generator_system.h"

using namespace atlas;

// ==================== ProceduralMissionGenerator System Tests ====================

static void testProcMissionCreate() {
    std::cout << "\n=== ProceduralMissionGenerator: Create ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    assertTrue(sys.initialize("gen1", "gen_caldari", "caldari_navy"), "Init succeeds");
    assertTrue(sys.getAvailableCount("gen1") == 0, "No missions initially");
    assertTrue(sys.getCompletedCount("gen1") == 0, "No completed initially");
    assertTrue(sys.getTotalGenerated("gen1") == 0, "No generated initially");
}

static void testProcMissionGenerate() {
    std::cout << "\n=== ProceduralMissionGenerator: Generate ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    assertTrue(sys.generateMission("gen1", "m1", "Combat", 3, "system_jita"), "Generate succeeds");
    assertTrue(sys.getAvailableCount("gen1") == 1, "1 available mission");
    assertTrue(sys.getTotalGenerated("gen1") == 1, "1 total generated");
    assertTrue(sys.getMissionDifficulty("gen1", "m1") == 3, "Difficulty is 3");
}

static void testProcMissionDuplicate() {
    std::cout << "\n=== ProceduralMissionGenerator: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 3, "system_jita");
    assertTrue(!sys.generateMission("gen1", "m1", "Mining", 2, "system_jita"), "Duplicate rejected");
    assertTrue(sys.getAvailableCount("gen1") == 1, "Still 1 mission");
}

static void testProcMissionAccept() {
    std::cout << "\n=== ProceduralMissionGenerator: Accept ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 3, "system_jita");
    assertTrue(sys.acceptMission("gen1", "m1"), "Accept succeeds");
    assertTrue(sys.isMissionAccepted("gen1", "m1"), "Mission is accepted");
    assertTrue(!sys.acceptMission("gen1", "m1"), "Double accept rejected");
}

static void testProcMissionComplete() {
    std::cout << "\n=== ProceduralMissionGenerator: Complete ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 3, "system_jita");
    sys.acceptMission("gen1", "m1");
    assertTrue(sys.completeMission("gen1", "m1"), "Complete succeeds");
    assertTrue(sys.getCompletedCount("gen1") == 1, "1 completed");
    assertTrue(!sys.completeMission("gen1", "m1"), "Double complete rejected");
}

static void testProcMissionExpire() {
    std::cout << "\n=== ProceduralMissionGenerator: Expire ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Courier", 1, "system_jita");
    sys.acceptMission("gen1", "m1");
    // Advance past time limit to expire
    sys.update(5000.0f);
    assertTrue(!sys.completeMission("gen1", "m1"), "Can't complete expired mission");
}

static void testProcMissionReward() {
    std::cout << "\n=== ProceduralMissionGenerator: Reward ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 1, "system_jita");
    sys.generateMission("gen1", "m2", "Mining", 5, "system_jita");
    float r1 = sys.getMissionReward("gen1", "m1");
    float r2 = sys.getMissionReward("gen1", "m2");
    assertTrue(r1 > 0.0f, "Reward L1 > 0");
    assertTrue(r2 > r1, "Higher difficulty = higher reward");
}

static void testProcMissionDifficultyBias() {
    std::cout << "\n=== ProceduralMissionGenerator: DifficultyBias ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    auto* entity = world.getEntity("gen1");
    auto* gen = entity->getComponent<components::ProceduralMissionGenerator>();
    gen->difficulty_bias = 2;
    sys.generateMission("gen1", "m1", "Combat", 2, "system_jita");
    assertTrue(sys.getMissionDifficulty("gen1", "m1") == 4, "Bias adds 2 to difficulty");
}

static void testProcMissionMaxLimit() {
    std::cout << "\n=== ProceduralMissionGenerator: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    auto* entity = world.getEntity("gen1");
    auto* gen = entity->getComponent<components::ProceduralMissionGenerator>();
    gen->max_available = 2;
    sys.generateMission("gen1", "m1", "Combat", 1, "s1");
    sys.generateMission("gen1", "m2", "Mining", 2, "s1");
    assertTrue(!sys.generateMission("gen1", "m3", "Courier", 3, "s1"), "Max limit enforced");
    assertTrue(sys.getAvailableCount("gen1") == 2, "Still 2 missions");
}

static void testProcMissionMissing() {
    std::cout << "\n=== ProceduralMissionGenerator: Missing ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "g1", "f1"), "Init fails on missing");
    assertTrue(!sys.generateMission("nonexistent", "m1", "Combat", 1, "s1"), "Generate fails on missing");
    assertTrue(!sys.acceptMission("nonexistent", "m1"), "Accept fails on missing");
    assertTrue(!sys.completeMission("nonexistent", "m1"), "Complete fails on missing");
    assertTrue(!sys.expireMission("nonexistent", "m1"), "Expire fails on missing");
    assertTrue(!sys.removeMission("nonexistent", "m1"), "Remove fails on missing");
    assertTrue(sys.getAvailableCount("nonexistent") == 0, "0 available on missing");
    assertTrue(sys.getCompletedCount("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getMissionReward("nonexistent", "m1"), 0.0f), "0 reward on missing");
    assertTrue(sys.getMissionDifficulty("nonexistent", "m1") == 0, "0 difficulty on missing");
    assertTrue(!sys.isMissionAccepted("nonexistent", "m1"), "Not accepted on missing");
    assertTrue(sys.getTotalGenerated("nonexistent") == 0, "0 generated on missing");
}


void run_procedural_mission_generator_system_tests() {
    testProcMissionCreate();
    testProcMissionGenerate();
    testProcMissionDuplicate();
    testProcMissionAccept();
    testProcMissionComplete();
    testProcMissionExpire();
    testProcMissionReward();
    testProcMissionDifficultyBias();
    testProcMissionMaxLimit();
    testProcMissionMissing();
}
