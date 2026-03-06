// Tests for: PlayerStandingSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/player_standing_system.h"

using namespace atlas;

// ==================== PlayerStandingSystem Tests ====================

static void testStandingAddFaction() {
    std::cout << "\n=== PlayerStanding: Add Faction ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    assertTrue(sys.addFaction("player_1", "caldari", "Caldari State"), "Add Caldari succeeds");
    assertTrue(sys.addFaction("player_1", "amarr", "Amarr Empire"), "Add Amarr succeeds");
    assertTrue(sys.getFactionCount("player_1") == 2, "2 factions tracked");
    assertTrue(sys.hasFaction("player_1", "caldari"), "Has Caldari");
    assertTrue(sys.hasFaction("player_1", "amarr"), "Has Amarr");
}

static void testStandingDuplicateFaction() {
    std::cout << "\n=== PlayerStanding: Duplicate Faction ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "caldari", "Caldari State");
    assertTrue(!sys.addFaction("player_1", "caldari", "Caldari State"), "Duplicate rejected");
    assertTrue(sys.getFactionCount("player_1") == 1, "Still 1 faction");
}

static void testStandingModify() {
    std::cout << "\n=== PlayerStanding: Modify ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "caldari", "Caldari State");
    assertTrue(sys.modifyStanding("player_1", "caldari", 3.0), "Modify standing succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getStanding("player_1", "caldari")), 3.0f), "Standing is 3.0");
    assertTrue(sys.getRank("player_1", "caldari") == 3, "Rank is 3 (Friend)");
}

static void testStandingClamp() {
    std::cout << "\n=== PlayerStanding: Clamp ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "caldari", "Caldari State");
    sys.modifyStanding("player_1", "caldari", 15.0);
    assertTrue(approxEqual(static_cast<float>(sys.getStanding("player_1", "caldari")), 10.0f), "Clamped to 10.0");

    sys.modifyStanding("player_1", "caldari", -25.0);
    assertTrue(approxEqual(static_cast<float>(sys.getStanding("player_1", "caldari")), -10.0f), "Clamped to -10.0");
}

static void testStandingRankThresholds() {
    std::cout << "\n=== PlayerStanding: Rank Thresholds ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "f1", "Test Faction");

    // Test positive ranks
    sys.modifyStanding("player_1", "f1", 0.1);
    assertTrue(sys.getRank("player_1", "f1") == 1, "Rank 1 at 0.1");

    sys.modifyStanding("player_1", "f1", 0.4);  // 0.5
    assertTrue(sys.getRank("player_1", "f1") == 2, "Rank 2 at 0.5");

    sys.modifyStanding("player_1", "f1", 1.5);  // 2.0
    assertTrue(sys.getRank("player_1", "f1") == 3, "Rank 3 at 2.0");

    sys.modifyStanding("player_1", "f1", 3.0);  // 5.0
    assertTrue(sys.getRank("player_1", "f1") == 4, "Rank 4 at 5.0");

    sys.modifyStanding("player_1", "f1", 3.0);  // 8.0
    assertTrue(sys.getRank("player_1", "f1") == 5, "Rank 5 at 8.0");
}

static void testStandingNegativeRanks() {
    std::cout << "\n=== PlayerStanding: Negative Ranks ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "f1", "Test");
    sys.modifyStanding("player_1", "f1", -0.5);
    assertTrue(sys.getRank("player_1", "f1") == -2, "Rank -2 at -0.5");

    sys.modifyStanding("player_1", "f1", -4.5);  // -5.0
    assertTrue(sys.getRank("player_1", "f1") == -4, "Rank -4 at -5.0");

    sys.modifyStanding("player_1", "f1", -5.0);  // -10.0
    assertTrue(sys.getRank("player_1", "f1") == -5, "Rank -5 at -10.0");
}

static void testStandingNotifications() {
    std::cout << "\n=== PlayerStanding: Notifications ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "caldari", "Caldari");
    // Standing starts at 0 (rank 0). Move to rank 3.
    sys.modifyStanding("player_1", "caldari", 3.0);
    assertTrue(sys.getNotificationCount("player_1") == 1, "1 notification on rank change");

    // Move from rank 3 to rank 5
    sys.modifyStanding("player_1", "caldari", 6.0);
    assertTrue(sys.getNotificationCount("player_1") == 2, "2 notifications total");
}

static void testStandingNoNotificationSameRank() {
    std::cout << "\n=== PlayerStanding: No Notification Same Rank ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    sys.addFaction("player_1", "caldari", "Caldari");
    sys.modifyStanding("player_1", "caldari", 3.0);   // rank 0 → 3
    sys.modifyStanding("player_1", "caldari", 0.5);   // rank stays 3

    assertTrue(sys.getNotificationCount("player_1") == 1, "No extra notification");
}

static void testStandingRankNames() {
    std::cout << "\n=== PlayerStanding: Rank Names ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    assertTrue(sys.getRankName(5) == "Hero", "Rank 5 is Hero");
    assertTrue(sys.getRankName(0) == "Neutral", "Rank 0 is Neutral");
    assertTrue(sys.getRankName(-5) == "Nemesis", "Rank -5 is Nemesis");
    assertTrue(sys.getRankName(3) == "Friend", "Rank 3 is Friend");
    assertTrue(sys.getRankName(-3) == "Hostile", "Rank -3 is Hostile");
}

static void testStandingModifyNonexistentFaction() {
    std::cout << "\n=== PlayerStanding: Modify Nonexistent Faction ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerStanding>(e);

    assertTrue(!sys.modifyStanding("player_1", "fake", 1.0), "Modify fails for unknown faction");
}

static void testStandingMissingEntity() {
    std::cout << "\n=== PlayerStanding: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    assertTrue(!sys.addFaction("nonexistent", "f1", "Test"), "Add fails on missing");
    assertTrue(!sys.modifyStanding("nonexistent", "f1", 1.0), "Modify fails on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getStanding("nonexistent", "f1")), 0.0f), "0 standing on missing");
    assertTrue(sys.getRank("nonexistent", "f1") == 0, "0 rank on missing");
    assertTrue(sys.getFactionCount("nonexistent") == 0, "0 factions on missing");
    assertTrue(sys.getNotificationCount("nonexistent") == 0, "0 notifications on missing");
    assertTrue(!sys.hasFaction("nonexistent", "f1"), "No faction on missing");
}

static void testStandingMaxNotifications() {
    std::cout << "\n=== PlayerStanding: Max Notifications ===" << std::endl;
    ecs::World world;
    systems::PlayerStandingSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* ps = addComp<components::PlayerStanding>(e);
    ps->max_notifications = 2;

    sys.addFaction("player_1", "f1", "Faction1");
    // Create 3 rank changes to test eviction
    sys.modifyStanding("player_1", "f1", 0.1);    // 0→1
    sys.modifyStanding("player_1", "f1", 0.4);    // 1→2
    sys.modifyStanding("player_1", "f1", 1.5);    // 2→3

    assertTrue(sys.getNotificationCount("player_1") == 2, "Max notifications enforced");
}

void run_player_standing_system_tests() {
    testStandingAddFaction();
    testStandingDuplicateFaction();
    testStandingModify();
    testStandingClamp();
    testStandingRankThresholds();
    testStandingNegativeRanks();
    testStandingNotifications();
    testStandingNoNotificationSameRank();
    testStandingRankNames();
    testStandingModifyNonexistentFaction();
    testStandingMissingEntity();
    testStandingMaxNotifications();
}
