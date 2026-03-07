// Tests for: MissionRewardSystem
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/mission_reward_system.h"

using namespace atlas;

// ==================== MissionRewardSystem Tests ====================

static void testRewardAdd() {
    std::cout << "\n=== MissionReward: Add ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    assertTrue(sys.addReward("player_1", "m_001", 50000.0, "caldari", 0.5, "item_1", 10), "Add reward succeeds");
    assertTrue(sys.hasReward("player_1", "m_001"), "Reward exists");
    assertTrue(sys.getPendingCount("player_1") == 1, "1 pending reward");
    assertTrue(approxEqual(static_cast<float>(sys.getRewardIsc("player_1", "m_001")), 50000.0f), "ISC is 50000");
    assertTrue(sys.getRewardItemId("player_1", "m_001") == "item_1", "Item ID correct");
    assertTrue(sys.getRewardItemQuantity("player_1", "m_001") == 10, "Item qty is 10");
}

static void testRewardCollect() {
    std::cout << "\n=== MissionReward: Collect ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    sys.addReward("player_1", "m_001", 50000.0, "caldari", 0.5, "", 0);
    assertTrue(sys.collectReward("player_1", "m_001"), "Collect succeeds");
    assertTrue(sys.isCollected("player_1", "m_001"), "Marked as collected");
    assertTrue(sys.getTotalCollected("player_1") == 1, "1 total collected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscEarned("player_1")), 50000.0f), "50K ISC earned");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalStandingGained("player_1")), 0.5f), "0.5 standing gained");
}

static void testRewardDoubleCollect() {
    std::cout << "\n=== MissionReward: Double Collect ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    sys.addReward("player_1", "m_001", 50000.0, "caldari", 0.5, "", 0);
    sys.collectReward("player_1", "m_001");
    assertTrue(!sys.collectReward("player_1", "m_001"), "Double collect rejected");
    assertTrue(sys.getTotalCollected("player_1") == 1, "Still 1 collected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscEarned("player_1")), 50000.0f), "ISC not doubled");
}

static void testRewardDuplicateMission() {
    std::cout << "\n=== MissionReward: Duplicate Mission ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    sys.addReward("player_1", "m_001", 50000.0, "caldari", 0.5, "", 0);
    assertTrue(!sys.addReward("player_1", "m_001", 100000.0, "caldari", 1.0, "", 0), "Duplicate mission rejected");
    assertTrue(approxEqual(static_cast<float>(sys.getRewardIsc("player_1", "m_001")), 50000.0f), "Original ISC unchanged");
}

static void testRewardMultiple() {
    std::cout << "\n=== MissionReward: Multiple ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    sys.addReward("player_1", "m_001", 10000.0, "caldari", 0.1, "", 0);
    sys.addReward("player_1", "m_002", 20000.0, "amarr", 0.2, "item_a", 5);
    sys.addReward("player_1", "m_003", 30000.0, "gallente", 0.3, "item_b", 3);

    assertTrue(sys.getPendingCount("player_1") == 3, "3 pending");
    sys.collectReward("player_1", "m_001");
    sys.collectReward("player_1", "m_003");
    assertTrue(sys.getPendingCount("player_1") == 1, "1 still pending");
    assertTrue(sys.getTotalCollected("player_1") == 2, "2 collected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscEarned("player_1")), 40000.0f), "40K ISC total");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalStandingGained("player_1")), 0.4f), "0.4 standing total");
}

static void testRewardMaxPending() {
    std::cout << "\n=== MissionReward: Max Pending ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* mr = addComp<components::MissionReward>(e);
    mr->max_pending = 3;

    sys.addReward("player_1", "m_001", 1000.0, "f1", 0.1, "", 0);
    sys.addReward("player_1", "m_002", 2000.0, "f2", 0.2, "", 0);
    sys.addReward("player_1", "m_003", 3000.0, "f3", 0.3, "", 0);
    assertTrue(!sys.addReward("player_1", "m_004", 4000.0, "f4", 0.4, "", 0), "Max pending enforced");

    // Collect one, now should allow adding
    sys.collectReward("player_1", "m_001");
    assertTrue(sys.addReward("player_1", "m_004", 4000.0, "f4", 0.4, "", 0), "Add after collect succeeds");
}

static void testRewardCollectNonexistent() {
    std::cout << "\n=== MissionReward: Collect Nonexistent ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    assertTrue(!sys.collectReward("player_1", "fake_mission"), "Collect nonexistent fails");
}

static void testRewardMissingEntity() {
    std::cout << "\n=== MissionReward: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    assertTrue(!sys.addReward("nonexistent", "m", 100.0, "f", 0.1, "", 0), "Add fails on missing");
    assertTrue(!sys.collectReward("nonexistent", "m"), "Collect fails on missing");
    assertTrue(!sys.isCollected("nonexistent", "m"), "Not collected on missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(sys.getTotalCollected("nonexistent") == 0, "0 collected on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscEarned("nonexistent")), 0.0f), "0 ISC on missing");
    assertTrue(!sys.hasReward("nonexistent", "m"), "No reward on missing");
}

static void testRewardTimestamp() {
    std::cout << "\n=== MissionReward: Timestamp ===" << std::endl;
    ecs::World world;
    systems::MissionRewardSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::MissionReward>(e);

    sys.addReward("player_1", "m_001", 50000.0, "caldari", 0.5, "", 0);
    sys.update(15.0f);
    sys.collectReward("player_1", "m_001");

    auto* mr = e->getComponent<components::MissionReward>();
    assertTrue(mr->rewards[0].collected, "Reward is collected");
    assertTrue(approxEqual(mr->rewards[0].collected_at, 15.0f), "Collected at timestamp 15");
}

void run_mission_reward_system_tests() {
    testRewardAdd();
    testRewardCollect();
    testRewardDoubleCollect();
    testRewardDuplicateMission();
    testRewardMultiple();
    testRewardMaxPending();
    testRewardCollectNonexistent();
    testRewardMissingEntity();
    testRewardTimestamp();
}
