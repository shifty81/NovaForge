// Tests for: MissionConsequence Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/mission_consequence_system.h"

using namespace atlas;

// ==================== MissionConsequence Tests ====================

static void testConsequenceInit() {
    std::cout << "\n=== MissionConsequence: Init ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    assertTrue(sys.initializeConsequences("sys_1", "sol"), "Consequences initialized");
    assertTrue(sys.getActiveCount("sys_1") == 0, "No active consequences initially");
    assertTrue(!sys.initializeConsequences("sys_1", "sol"), "Duplicate init fails");
}

static void testConsequenceTrigger() {
    std::cout << "\n=== MissionConsequence: Trigger ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    assertTrue(sys.triggerConsequence("sys_1", "mission_1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 300.0f, "empire", false), "Consequence triggered");
    assertTrue(sys.getActiveCount("sys_1") == 1, "1 active consequence");
    assertTrue(sys.isConsequenceActive("sys_1", "csq_0"), "Consequence csq_0 is active");
}

static void testConsequenceExpiry() {
    std::cout << "\n=== MissionConsequence: Expiry ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::SecurityShift,
        1.0f, 5.0f, "police", false);
    assertTrue(sys.getActiveCount("sys_1") == 1, "1 active before expiry");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.getActiveCount("sys_1") == 0, "0 active after expiry");
}

static void testConsequencePermanent() {
    std::cout << "\n=== MissionConsequence: Permanent ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::TerritoryShift,
        2.0f, 10.0f, "rebels", true);
    for (int i = 0; i < 100; i++) sys.update(1.0f);
    assertTrue(sys.getActiveCount("sys_1") == 1, "Permanent consequence persists");
    assertTrue(sys.getPermanentCount("sys_1") == 1, "1 permanent consequence");
}

static void testConsequenceMagnitude() {
    std::cout << "\n=== MissionConsequence: Magnitude ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.3f, 300.0f, "traders", false);
    sys.triggerConsequence("sys_1", "m2",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.7f, 300.0f, "traders", false);
    float mag = sys.getMagnitude("sys_1",
        components::MissionConsequence::ConsequenceType::PriceImpact);
    assertTrue(approxEqual(mag, 1.0f), "Magnitudes sum to 1.0");
}

static void testConsequenceDecay() {
    std::cout << "\n=== MissionConsequence: Decay ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::SpawnChange,
        1.0f, 100.0f, "pirates", false);
    assertTrue(sys.getActiveCount("sys_1") == 1, "Active before decay");
    sys.update(50.0f);
    assertTrue(sys.getActiveCount("sys_1") == 1, "Still active at 50s");
    sys.update(60.0f);
    assertTrue(sys.getActiveCount("sys_1") == 0, "Expired after 110s total");
}

static void testConsequenceManualExpire() {
    std::cout << "\n=== MissionConsequence: Manual Expire ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        1.0f, 300.0f, "faction_a", false);
    assertTrue(sys.isConsequenceActive("sys_1", "csq_0"), "Active before manual expire");
    assertTrue(sys.expireConsequence("sys_1", "csq_0"), "Manual expire succeeds");
    assertTrue(sys.getActiveCount("sys_1") == 0, "0 active after manual expire");
    assertTrue(!sys.expireConsequence("sys_1", "csq_0"), "Double expire fails");
}

static void testConsequenceMultipleTypes() {
    std::cout << "\n=== MissionConsequence: Multiple Types ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 300.0f, "empire", false);
    sys.triggerConsequence("sys_1", "m2",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.8f, 300.0f, "traders", false);
    float standing = sys.getMagnitude("sys_1",
        components::MissionConsequence::ConsequenceType::StandingChange);
    float price = sys.getMagnitude("sys_1",
        components::MissionConsequence::ConsequenceType::PriceImpact);
    assertTrue(approxEqual(standing, 0.5f), "Standing magnitude is 0.5");
    assertTrue(approxEqual(price, 0.8f), "Price magnitude is 0.8");
}

static void testConsequenceCount() {
    std::cout << "\n=== MissionConsequence: Count ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    for (int i = 0; i < 5; i++) {
        sys.triggerConsequence("sys_1", "m" + std::to_string(i),
            components::MissionConsequence::ConsequenceType::ReputationBoost,
            0.1f, 300.0f, "faction", false);
    }
    assertTrue(sys.getActiveCount("sys_1") == 5, "5 active consequences");
    sys.triggerConsequence("sys_1", "m5",
        components::MissionConsequence::ConsequenceType::ResourceDepletion,
        0.2f, 300.0f, "miners", true);
    assertTrue(sys.getActiveCount("sys_1") == 6, "6 total after permanent");
    assertTrue(sys.getPermanentCount("sys_1") == 1, "1 permanent");
}

static void testConsequenceMissing() {
    std::cout << "\n=== MissionConsequence: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    assertTrue(!sys.initializeConsequences("nonexistent", "sol"), "Init fails on missing");
    assertTrue(sys.getActiveCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(approxEqual(sys.getMagnitude("nonexistent",
        components::MissionConsequence::ConsequenceType::StandingChange), 0.0f), "Magnitude 0 on missing");
    assertTrue(sys.getPermanentCount("nonexistent") == 0, "Permanent 0 on missing");
}


void run_mission_consequence_tests() {
    testConsequenceInit();
    testConsequenceTrigger();
    testConsequenceExpiry();
    testConsequencePermanent();
    testConsequenceMagnitude();
    testConsequenceDecay();
    testConsequenceManualExpire();
    testConsequenceMultipleTypes();
    testConsequenceCount();
    testConsequenceMissing();
}
