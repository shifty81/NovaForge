// Tests for: Local Reputation System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/local_reputation_system.h"
#include "systems/reputation_system.h"

using namespace atlas;

// ==================== Local Reputation System Tests ====================

static void testLocalReputationDefaults() {
    std::cout << "\n=== Local Reputation Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys1");
    addComp<components::LocalReputation>(e);

    systems::LocalReputationSystem sys(&world);
    assertTrue(approxEqual(sys.getReputation("sys1", "player1"), 0.0f), "Default reputation is 0");
    assertTrue(sys.getStanding("sys1", "player1") == "Neutral", "Default standing is Neutral");
}

static void testLocalReputationModify() {
    std::cout << "\n=== Local Reputation Modify ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys2");
    addComp<components::LocalReputation>(e);

    systems::LocalReputationSystem sys(&world);
    sys.modifyReputation("sys2", "player1", 30.0f);
    assertTrue(approxEqual(sys.getReputation("sys2", "player1"), 30.0f), "Reputation modified");
    sys.modifyReputation("sys2", "player1", 200.0f);
    assertTrue(approxEqual(sys.getReputation("sys2", "player1"), 100.0f), "Reputation clamped to 100");
}

static void testLocalReputationDecay() {
    std::cout << "\n=== Local Reputation Decay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys3");
    auto* rep = addComp<components::LocalReputation>(e);
    rep->reputation_decay_rate = 1.0f;

    systems::LocalReputationSystem sys(&world);
    sys.modifyReputation("sys3", "player1", 10.0f);
    sys.update(5.0f);
    assertTrue(sys.getReputation("sys3", "player1") < 10.0f, "Reputation decayed toward 0");
}

static void testLocalReputationStanding() {
    std::cout << "\n=== Local Reputation Standing ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys4");
    addComp<components::LocalReputation>(e);

    systems::LocalReputationSystem sys(&world);
    sys.modifyReputation("sys4", "p1", -60.0f);
    assertTrue(sys.getStanding("sys4", "p1") == "Hostile", "Hostile at -60");
    sys.modifyReputation("sys4", "p1", 30.0f); // now -30
    assertTrue(sys.getStanding("sys4", "p1") == "Unfriendly", "Unfriendly at -30");

    auto* e2 = world.createEntity("sys5");
    addComp<components::LocalReputation>(e2);
    sys.modifyReputation("sys5", "p2", 60.0f);
    assertTrue(sys.getStanding("sys5", "p2") == "Allied", "Allied at 60");
    sys.modifyReputation("sys5", "p3", 20.0f);
    assertTrue(sys.getStanding("sys5", "p3") == "Friendly", "Friendly at 20");
}


void run_local_reputation_system_tests() {
    testLocalReputationDefaults();
    testLocalReputationModify();
    testLocalReputationDecay();
    testLocalReputationStanding();
}
