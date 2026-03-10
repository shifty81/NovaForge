// Tests for: Incursion System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/incursion_system.h"

using namespace atlas;

// ==================== Incursion System Tests ====================

static void testIncursionCreate() {
    std::cout << "\n=== Incursion: Create ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    assertTrue(sys.initialize("inc1", "constellation_abc"), "Init succeeds");
    assertTrue(sys.getSiteCount("inc1") == 0, "No sites initially");
    assertTrue(approxEqual(sys.getInfluence("inc1"), 100.0f), "Influence starts at 100");
    assertTrue(!sys.isWithdrawn("inc1"), "Not withdrawn initially");
    assertTrue(sys.getCompletedSiteCount("inc1") == 0, "0 completed sites");
    assertTrue(approxEqual(sys.getTotalLPPaid("inc1"), 0.0f), "0 LP paid");
    assertTrue(sys.getFleetSize("inc1") == 0, "No fleet members");
}

static void testIncursionSpawnSite() {
    std::cout << "\n=== Incursion: SpawnSite ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");

    assertTrue(sys.spawnSite("inc1", "site_v1",
               components::IncursionState::Tier::Vanguard), "Spawn vanguard");
    assertTrue(sys.getSiteCount("inc1") == 1, "1 site");

    assertTrue(sys.spawnSite("inc1", "site_a1",
               components::IncursionState::Tier::Assault), "Spawn assault");
    assertTrue(sys.getSiteCount("inc1") == 2, "2 sites");

    assertTrue(sys.spawnSite("inc1", "site_hq1",
               components::IncursionState::Tier::Headquarters), "Spawn HQ");
    assertTrue(sys.getSiteCount("inc1") == 3, "3 sites");
}

static void testIncursionCompleteSite() {
    std::cout << "\n=== Incursion: CompleteSite ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");
    sys.spawnSite("inc1", "site_v1", components::IncursionState::Tier::Vanguard);

    assertTrue(sys.completeSite("inc1", "site_v1"), "Complete vanguard");
    assertTrue(sys.getCompletedSiteCount("inc1") == 1, "1 completed");
    // Base LP for Vanguard = 1000, fleet_bonus = max(0,1) = 1
    assertTrue(approxEqual(sys.getTotalLPPaid("inc1"), 1000.0f), "1000 LP paid");

    // Cannot complete same site twice
    assertTrue(!sys.completeSite("inc1", "site_v1"), "Cannot re-complete");
    assertTrue(sys.getCompletedSiteCount("inc1") == 1, "Still 1 completed");
}

static void testIncursionFleetCoordination() {
    std::cout << "\n=== Incursion: FleetCoordination ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");
    sys.spawnSite("inc1", "site_a1", components::IncursionState::Tier::Assault);

    // Register 3 fleet members for this site
    assertTrue(sys.registerFleetMember("inc1", "site_a1", "pilot_1"),
               "Register pilot 1");
    assertTrue(sys.registerFleetMember("inc1", "site_a1", "pilot_2"),
               "Register pilot 2");
    assertTrue(sys.registerFleetMember("inc1", "site_a1", "pilot_3"),
               "Register pilot 3");
    assertTrue(sys.getFleetSize("inc1") == 3, "3 fleet members");

    // Complete site: base LP = 2500 * 3 fleet = 7500
    sys.completeSite("inc1", "site_a1");
    assertTrue(approxEqual(sys.getTotalLPPaid("inc1"), 7500.0f),
               "7500 LP paid (2500 * 3 fleet)");
}

static void testIncursionInfluence() {
    std::cout << "\n=== Incursion: Influence ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");

    assertTrue(approxEqual(sys.getInfluence("inc1"), 100.0f), "Starts at 100");

    // Apply negative delta
    assertTrue(sys.applyInfluenceDelta("inc1", -25.0f), "Apply -25 delta");
    assertTrue(approxEqual(sys.getInfluence("inc1"), 75.0f), "75 after -25");

    // Apply positive delta
    assertTrue(sys.applyInfluenceDelta("inc1", 10.0f), "Apply +10 delta");
    assertTrue(approxEqual(sys.getInfluence("inc1"), 85.0f), "85 after +10");

    // Clamp at 100
    assertTrue(sys.applyInfluenceDelta("inc1", 50.0f), "Apply +50 delta");
    assertTrue(approxEqual(sys.getInfluence("inc1"), 100.0f), "Clamped at 100");

    // Decay via update (rate = 0.1/s, 10s → -1.0)
    sys.update(10.0f);
    assertTrue(sys.getInfluence("inc1") < 100.0f, "Influence decayed");
    assertTrue(approxEqual(sys.getInfluence("inc1"), 99.0f), "~99 after 10s decay");
}

static void testIncursionWithdrawal() {
    std::cout << "\n=== Incursion: Withdrawal ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");

    assertTrue(!sys.isWithdrawn("inc1"), "Not withdrawn initially");

    // Drive influence to zero via delta
    sys.applyInfluenceDelta("inc1", -100.0f);
    assertTrue(approxEqual(sys.getInfluence("inc1"), 0.0f), "Influence at 0");
    assertTrue(sys.isWithdrawn("inc1"), "Withdrawn at 0 influence");
}

static void testIncursionMaxSites() {
    std::cout << "\n=== Incursion: MaxSites ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");

    for (int i = 0; i < 10; i++) {
        assertTrue(sys.spawnSite("inc1", "site_" + std::to_string(i)),
                   "Spawn site " + std::to_string(i));
    }
    assertTrue(sys.getSiteCount("inc1") == 10, "10 sites (max)");
    assertTrue(!sys.spawnSite("inc1", "overflow"), "Overflow rejected");
}

static void testIncursionMultipleTiers() {
    std::cout << "\n=== Incursion: MultipleTiers ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "const_01");

    // Spawn and complete one of each tier (no fleet → bonus = 1)
    sys.spawnSite("inc1", "sv", components::IncursionState::Tier::Vanguard);
    sys.spawnSite("inc1", "sa", components::IncursionState::Tier::Assault);
    sys.spawnSite("inc1", "sh", components::IncursionState::Tier::Headquarters);

    sys.completeSite("inc1", "sv");
    assertTrue(approxEqual(sys.getTotalLPPaid("inc1"), 1000.0f), "Vanguard 1000 LP");

    sys.completeSite("inc1", "sa");
    assertTrue(approxEqual(sys.getTotalLPPaid("inc1"), 3500.0f), "Assault adds 2500");

    sys.completeSite("inc1", "sh");
    assertTrue(approxEqual(sys.getTotalLPPaid("inc1"), 10500.0f), "HQ adds 7000");

    assertTrue(sys.getCompletedSiteCount("inc1") == 3, "3 sites completed");
}

static void testIncursionMissing() {
    std::cout << "\n=== Incursion: Missing ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.spawnSite("nonexistent", "site1"), "Spawn fails on missing");
    assertTrue(!sys.completeSite("nonexistent", "site1"), "Complete fails on missing");
    assertTrue(!sys.registerFleetMember("nonexistent", "site1", "pilot1"),
               "Register fails on missing");
    assertTrue(!sys.applyInfluenceDelta("nonexistent", -10.0f),
               "Influence delta fails on missing");
    assertTrue(sys.getSiteCount("nonexistent") == 0, "0 sites on missing");
    assertTrue(approxEqual(sys.getInfluence("nonexistent"), 0.0f), "0 influence on missing");
    assertTrue(!sys.isWithdrawn("nonexistent"), "Not withdrawn on missing");
    assertTrue(sys.getCompletedSiteCount("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getTotalLPPaid("nonexistent"), 0.0f), "0 LP on missing");
    assertTrue(sys.getFleetSize("nonexistent") == 0, "0 fleet on missing");
}

void run_incursion_system_tests() {
    testIncursionCreate();
    testIncursionSpawnSite();
    testIncursionCompleteSite();
    testIncursionFleetCoordination();
    testIncursionInfluence();
    testIncursionWithdrawal();
    testIncursionMaxSites();
    testIncursionMultipleTiers();
    testIncursionMissing();
}
