// Tests for: Incursion System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/incursion_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Incursion System Tests ====================

static void testIncursionCreate() {
    std::cout << "\n=== Incursion: Create ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    assertTrue(sys.initialize("inc1", "incursion_alpha", "system_jita", 2), "Init succeeds");
    assertTrue(sys.getInfluence("inc1") > 0.9f, "Influence starts at 1.0");
    assertTrue(sys.getParticipantCount("inc1") == 0, "No participants initially");
    assertTrue(sys.getState("inc1") == 0, "State is Pending");
    assertTrue(approxEqual(sys.getRewardPool("inc1"), 0.0f), "Reward pool is 0");
    assertTrue(sys.getTotalWavesDefeated("inc1") == 0, "No waves defeated");
}

static void testIncursionAddWave() {
    std::cout << "\n=== Incursion: AddWave ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    assertTrue(sys.addWave("inc1", 1, "Frigate", 5), "Add wave 1 succeeds");
    assertTrue(sys.addWave("inc1", 2, "Cruiser", 3), "Add wave 2 succeeds");
    assertTrue(sys.addWave("inc1", 3, "Battleship", 1), "Add wave 3 succeeds");
}

static void testIncursionDuplicate() {
    std::cout << "\n=== Incursion: Duplicate ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    sys.addWave("inc1", 1, "Frigate", 5);
    assertTrue(!sys.addWave("inc1", 1, "Cruiser", 3), "Duplicate wave rejected");
    sys.joinIncursion("inc1", "player1");
    assertTrue(!sys.joinIncursion("inc1", "player1"), "Duplicate participant rejected");
}

static void testIncursionSpawn() {
    std::cout << "\n=== Incursion: Spawn ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    sys.addWave("inc1", 1, "Frigate", 5);
    assertTrue(sys.spawnWave("inc1", 1), "Spawn wave succeeds");
    assertTrue(!sys.spawnWave("inc1", 1), "Double spawn rejected");
    assertTrue(!sys.spawnWave("inc1", 99), "Spawn nonexistent wave fails");
}

static void testIncursionDefeat() {
    std::cout << "\n=== Incursion: Defeat ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    sys.addWave("inc1", 1, "Frigate", 5);
    sys.spawnWave("inc1", 1);
    float before = sys.getInfluence("inc1");
    assertTrue(sys.defeatWave("inc1", 1), "Defeat wave succeeds");
    assertTrue(sys.getInfluence("inc1") < before, "Influence decreased");
    assertTrue(sys.getTotalWavesDefeated("inc1") == 1, "1 wave defeated");
    assertTrue(!sys.defeatWave("inc1", 1), "Double defeat rejected");
}

static void testIncursionJoin() {
    std::cout << "\n=== Incursion: Join ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    assertTrue(sys.joinIncursion("inc1", "player1"), "Join succeeds");
    assertTrue(sys.joinIncursion("inc1", "player2"), "Join second succeeds");
    assertTrue(sys.getParticipantCount("inc1") == 2, "2 participants");
    assertTrue(sys.leaveIncursion("inc1", "player1"), "Leave succeeds");
    assertTrue(sys.getParticipantCount("inc1") == 1, "1 participant after leave");
    assertTrue(!sys.leaveIncursion("inc1", "player1"), "Double leave fails");
}

static void testIncursionRewards() {
    std::cout << "\n=== Incursion: Rewards ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    assertTrue(sys.contributeRewards("inc1", 1000.0f), "Contribute succeeds");
    assertTrue(sys.contributeRewards("inc1", 500.0f), "Contribute again succeeds");
    assertTrue(approxEqual(sys.getRewardPool("inc1"), 1500.0f), "Pool is 1500");
}

static void testIncursionStateTransition() {
    std::cout << "\n=== Incursion: StateTransition ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);

    auto* entity = world.getEntity("inc1");
    auto* inc = entity->getComponent<components::Incursion>();
    inc->max_waves = 2;
    inc->state = components::Incursion::IncursionState::Active;

    sys.addWave("inc1", 1, "Frigate", 5);
    sys.addWave("inc1", 2, "Cruiser", 3);

    sys.defeatWave("inc1", 1);
    sys.update(0.1f);
    assertTrue(sys.getState("inc1") == 1, "Still Active after 1 wave defeated");

    sys.defeatWave("inc1", 2);
    sys.update(0.1f);
    assertTrue(sys.getState("inc1") == 2, "Withdrawing after all waves defeated");

    inc->influence = 0.0f;
    sys.update(0.1f);
    assertTrue(sys.getState("inc1") == 3, "Defeated when influence is 0");
}

static void testIncursionMaxLimit() {
    std::cout << "\n=== Incursion: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);

    auto* entity = world.getEntity("inc1");
    auto* inc = entity->getComponent<components::Incursion>();
    inc->max_waves = 2;
    inc->max_participants = 2;

    sys.addWave("inc1", 1, "Frigate", 5);
    sys.addWave("inc1", 2, "Cruiser", 3);
    assertTrue(!sys.addWave("inc1", 3, "Battleship", 1), "Max waves enforced");

    sys.joinIncursion("inc1", "p1");
    sys.joinIncursion("inc1", "p2");
    assertTrue(!sys.joinIncursion("inc1", "p3"), "Max participants enforced");
}

static void testIncursionMissing() {
    std::cout << "\n=== Incursion: Missing ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "i1", "s1", 1), "Init fails on missing");
    assertTrue(!sys.addWave("nonexistent", 1, "Frigate", 5), "AddWave fails on missing");
    assertTrue(!sys.spawnWave("nonexistent", 1), "SpawnWave fails on missing");
    assertTrue(!sys.defeatWave("nonexistent", 1), "DefeatWave fails on missing");
    assertTrue(!sys.joinIncursion("nonexistent", "p1"), "Join fails on missing");
    assertTrue(!sys.leaveIncursion("nonexistent", "p1"), "Leave fails on missing");
    assertTrue(!sys.contributeRewards("nonexistent", 100.0f), "Contribute fails on missing");
    assertTrue(approxEqual(sys.getInfluence("nonexistent"), 0.0f), "0 influence on missing");
    assertTrue(sys.getParticipantCount("nonexistent") == 0, "0 participants on missing");
    assertTrue(sys.getState("nonexistent") == 0, "0 state on missing");
    assertTrue(approxEqual(sys.getRewardPool("nonexistent"), 0.0f), "0 pool on missing");
    assertTrue(sys.getTotalWavesDefeated("nonexistent") == 0, "0 defeated on missing");
}


void run_incursion_system_tests() {
    testIncursionCreate();
    testIncursionAddWave();
    testIncursionDuplicate();
    testIncursionSpawn();
    testIncursionDefeat();
    testIncursionJoin();
    testIncursionRewards();
    testIncursionStateTransition();
    testIncursionMaxLimit();
    testIncursionMissing();
}
