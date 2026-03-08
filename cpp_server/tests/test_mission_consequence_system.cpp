// Tests for: MissionConsequenceSystem
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/mission_consequence_system.h"

using namespace atlas;

// ==================== MissionConsequenceSystem Tests ====================

static void testMissionConsequenceCreate() {
    std::cout << "\n=== MissionConsequence: Create ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    assertTrue(sys.initializeConsequences("sys1", "alpha_centauri"), "Init succeeds");
    assertTrue(sys.getActiveCount("sys1") == 0, "Zero active consequences");
    assertTrue(sys.getPermanentCount("sys1") == 0, "Zero permanent consequences");
}

static void testMissionConsequenceInvalidInit() {
    std::cout << "\n=== MissionConsequence: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    assertTrue(!sys.initializeConsequences("missing", "sys_a"), "Missing entity fails");

    world.createEntity("sys1");
    assertTrue(sys.initializeConsequences("sys1", "sys_a"), "First init succeeds");
    assertTrue(!sys.initializeConsequences("sys1", "sys_a"), "Double init fails");
}

static void testMissionConsequenceTrigger() {
    std::cout << "\n=== MissionConsequence: Trigger ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "alpha_centauri");

    assertTrue(sys.triggerConsequence("sys1", "mission_1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 300.0f, "gallente", false), "Trigger standing change");
    assertTrue(sys.getActiveCount("sys1") == 1, "1 active consequence");
    assertTrue(sys.getPermanentCount("sys1") == 0, "0 permanent");

    assertTrue(sys.triggerConsequence("sys1", "mission_2",
        components::MissionConsequence::ConsequenceType::SecurityShift,
        -0.3f, 600.0f, "caldari", true), "Trigger permanent security shift");
    assertTrue(sys.getActiveCount("sys1") == 2, "2 active consequences");
    assertTrue(sys.getPermanentCount("sys1") == 1, "1 permanent");
}

static void testMissionConsequenceInvalidTrigger() {
    std::cout << "\n=== MissionConsequence: InvalidTrigger ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    assertTrue(!sys.triggerConsequence("missing", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 300.0f, "faction", false), "Trigger on missing entity fails");
}

static void testMissionConsequenceMagnitude() {
    std::cout << "\n=== MissionConsequence: Magnitude ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");

    sys.triggerConsequence("sys1", "m1",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.2f, 300.0f, "faction_a", false);
    sys.triggerConsequence("sys1", "m2",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.3f, 300.0f, "faction_b", false);

    float mag = sys.getMagnitude("sys1",
        components::MissionConsequence::ConsequenceType::PriceImpact);
    assertTrue(mag > 0.49f && mag < 0.51f, "Combined magnitude ~0.5");

    float standing_mag = sys.getMagnitude("sys1",
        components::MissionConsequence::ConsequenceType::StandingChange);
    assertTrue(approxEqual(standing_mag, 0.0f), "No standing consequences = 0.0");
}

static void testMissionConsequenceExpire() {
    std::cout << "\n=== MissionConsequence: Expire ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");

    sys.triggerConsequence("sys1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        1.0f, 300.0f, "faction_a", false);

    // Consequence ID is "csq_0" (based on times_triggered counter)
    assertTrue(sys.isConsequenceActive("sys1", "csq_0"), "csq_0 is active");
    assertTrue(sys.expireConsequence("sys1", "csq_0"), "Expire csq_0 succeeds");
    assertTrue(!sys.isConsequenceActive("sys1", "csq_0"), "csq_0 no longer active");
    assertTrue(sys.getActiveCount("sys1") == 0, "0 active after expire");

    assertTrue(!sys.expireConsequence("sys1", "csq_0"), "Double expire fails");
    assertTrue(!sys.expireConsequence("sys1", "nonexistent"), "Expire nonexistent fails");
}

static void testMissionConsequenceIsActive() {
    std::cout << "\n=== MissionConsequence: IsActive ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");

    assertTrue(!sys.isConsequenceActive("sys1", "csq_0"), "Not active before trigger");
    sys.triggerConsequence("sys1", "m1",
        components::MissionConsequence::ConsequenceType::SpawnChange,
        1.0f, 60.0f, "pirates", false);
    assertTrue(sys.isConsequenceActive("sys1", "csq_0"), "Active after trigger");
    assertTrue(!sys.isConsequenceActive("sys1", "csq_1"), "csq_1 not active");
    assertTrue(!sys.isConsequenceActive("missing", "csq_0"), "Missing entity returns false");
}

static void testMissionConsequenceTimeExpiry() {
    std::cout << "\n=== MissionConsequence: TimeExpiry ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");

    // Non-permanent consequence with 10s duration
    sys.triggerConsequence("sys1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 10.0f, "faction_a", false);
    assertTrue(sys.getActiveCount("sys1") == 1, "1 active before expiry");

    // Advance 5s — still active
    sys.update(5.0f);
    assertTrue(sys.getActiveCount("sys1") == 1, "Still active at 5s");

    // Advance past duration — should be removed
    sys.update(6.0f);
    assertTrue(sys.getActiveCount("sys1") == 0, "Expired after 11s");
}

static void testMissionConsequencePermanentSurvivesExpiry() {
    std::cout << "\n=== MissionConsequence: PermanentSurvivesExpiry ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");

    // Permanent consequence
    sys.triggerConsequence("sys1", "m1",
        components::MissionConsequence::ConsequenceType::TerritoryShift,
        1.0f, 10.0f, "faction_a", true);

    // Non-permanent consequence
    sys.triggerConsequence("sys1", "m2",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 10.0f, "faction_b", false);

    assertTrue(sys.getActiveCount("sys1") == 2, "2 active");
    assertTrue(sys.getPermanentCount("sys1") == 1, "1 permanent");

    // Expire both by time
    sys.update(15.0f);
    assertTrue(sys.getActiveCount("sys1") == 1, "Permanent survives, non-permanent expired");
    assertTrue(sys.getPermanentCount("sys1") == 1, "Still 1 permanent");
}

static void testMissionConsequenceMultipleTypes() {
    std::cout << "\n=== MissionConsequence: MultipleTypes ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");

    sys.triggerConsequence("sys1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        1.0f, 300.0f, "f1", false);
    sys.triggerConsequence("sys1", "m2",
        components::MissionConsequence::ConsequenceType::SecurityShift,
        -0.5f, 300.0f, "f2", false);
    sys.triggerConsequence("sys1", "m3",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.3f, 300.0f, "f3", false);
    sys.triggerConsequence("sys1", "m4",
        components::MissionConsequence::ConsequenceType::ResourceDepletion,
        2.0f, 300.0f, "f4", true);

    assertTrue(sys.getActiveCount("sys1") == 4, "4 active consequences");
    assertTrue(approxEqual(sys.getMagnitude("sys1",
        components::MissionConsequence::ConsequenceType::StandingChange), 1.0f),
        "Standing magnitude 1.0");
    assertTrue(approxEqual(sys.getMagnitude("sys1",
        components::MissionConsequence::ConsequenceType::ResourceDepletion), 2.0f),
        "Resource depletion magnitude 2.0");
}

static void testMissionConsequenceUpdate() {
    std::cout << "\n=== MissionConsequence: Update ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys1");
    sys.initializeConsequences("sys1", "system_a");
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testMissionConsequenceMissing() {
    std::cout << "\n=== MissionConsequence: Missing ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    assertTrue(sys.getActiveCount("x") == 0, "Default active count on missing");
    assertTrue(sys.getPermanentCount("x") == 0, "Default permanent count on missing");
    assertTrue(approxEqual(sys.getMagnitude("x",
        components::MissionConsequence::ConsequenceType::StandingChange), 0.0f),
        "Default magnitude on missing");
    assertTrue(!sys.isConsequenceActive("x", "csq_0"), "Default isActive on missing");
    assertTrue(!sys.expireConsequence("x", "csq_0"), "Expire on missing fails");
}

void run_mission_consequence_system_tests() {
    testMissionConsequenceCreate();
    testMissionConsequenceInvalidInit();
    testMissionConsequenceTrigger();
    testMissionConsequenceInvalidTrigger();
    testMissionConsequenceMagnitude();
    testMissionConsequenceExpire();
    testMissionConsequenceIsActive();
    testMissionConsequenceTimeExpiry();
    testMissionConsequencePermanentSurvivesExpiry();
    testMissionConsequenceMultipleTypes();
    testMissionConsequenceUpdate();
    testMissionConsequenceMissing();
}
