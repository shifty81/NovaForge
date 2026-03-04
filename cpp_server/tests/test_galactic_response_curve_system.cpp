// Tests for: GalacticResponseCurve System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/galactic_response_curve_system.h"

using namespace atlas;

// ===== GalacticResponseCurve System Tests =====

static void testGalacticResponseCreate() {
    std::cout << "\n=== GalacticResponse: Create ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    assertTrue(sys.initializeFaction("faction1", "Solari"), "Init faction succeeds");
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 0.0f), "Initial threat is 0");
    assertTrue(sys.getResponseTier("faction1") == 0, "Initial tier is 0");
    assertTrue(sys.getReinforcementCount("faction1") == 0, "No reinforcements initially");
    assertTrue(sys.getReroutedSystemCount("faction1") == 0, "No rerouted systems");
    assertTrue(approxEqual(sys.getEscalationRate("faction1"), 1.0f), "Default escalation rate is 1.0");
    assertTrue(!sys.isFullMobilization("faction1"), "No full mobilization initially");
}

static void testGalacticResponseThreat() {
    std::cout << "\n=== GalacticResponse: Threat ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Solari");
    assertTrue(sys.reportThreat("faction1", 15.0f), "Report threat succeeds");
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 15.0f), "Threat is 15");
    sys.update(0.0f); // update tiers
    assertTrue(sys.getResponseTier("faction1") == 1, "Tier 1 (Alert) at threat 15");
}

static void testGalacticResponseTierEscalation() {
    std::cout << "\n=== GalacticResponse: TierEscalation ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Veyren");
    sys.reportThreat("faction1", 30.0f);
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 2, "Tier 2 (Mobilize) at threat 30");
    sys.reportThreat("faction1", 25.0f); // total: 55
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 3, "Tier 3 (Reinforce) at threat 55");
    sys.reportThreat("faction1", 30.0f); // total: 85
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 4, "Tier 4 (FullMobilization) at threat 85");
    assertTrue(sys.isFullMobilization("faction1"), "Full mobilization active");
}

static void testGalacticResponseReinforcement() {
    std::cout << "\n=== GalacticResponse: Reinforcement ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Keldari");
    sys.reportThreat("faction1", 55.0f);
    sys.update(0.0f); // tier 3
    assertTrue(sys.dispatchReinforcement("faction1"), "Dispatch at tier 3 succeeds");
    assertTrue(sys.getReinforcementCount("faction1") == 1, "1 reinforcement dispatched");
    assertTrue(sys.dispatchReinforcement("faction1"), "Second dispatch succeeds");
    assertTrue(sys.getReinforcementCount("faction1") == 2, "2 reinforcements dispatched");
}

static void testGalacticResponseReinforcementBlocked() {
    std::cout << "\n=== GalacticResponse: ReinforcementBlocked ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Aurelian");
    sys.reportThreat("faction1", 20.0f);
    sys.update(0.0f); // tier 2
    assertTrue(!sys.dispatchReinforcement("faction1"), "Dispatch blocked at tier 2");
    assertTrue(sys.getReinforcementCount("faction1") == 0, "No reinforcements dispatched");
}

static void testGalacticResponseReroute() {
    std::cout << "\n=== GalacticResponse: Reroute ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Solari");
    assertTrue(sys.rerouteTradeFor("faction1", "system_alpha"), "Reroute succeeds");
    assertTrue(sys.getReroutedSystemCount("faction1") == 1, "1 system rerouted");
    assertTrue(!sys.rerouteTradeFor("faction1", "system_alpha"), "Duplicate reroute fails");
    assertTrue(sys.rerouteTradeFor("faction1", "system_beta"), "Second reroute succeeds");
    assertTrue(sys.getReroutedSystemCount("faction1") == 2, "2 systems rerouted");
}

static void testGalacticResponseDecay() {
    std::cout << "\n=== GalacticResponse: Decay ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Veyren");
    sys.reportThreat("faction1", 30.0f);
    sys.update(10.0f); // decay: 30 - 0.1*10 = 29
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 29.0f), "Threat decayed to 29");
    assertTrue(sys.getResponseTier("faction1") == 2, "Still tier 2 at 29");
}

static void testGalacticResponseDecayToZero() {
    std::cout << "\n=== GalacticResponse: DecayToZero ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Keldari");
    sys.reportThreat("faction1", 5.0f);
    sys.update(100.0f); // decay: 5 - 0.1*100 = -5, clamped to 0
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 0.0f), "Threat decayed to 0");
    assertTrue(sys.getResponseTier("faction1") == 0, "Back to tier 0");
}

static void testGalacticResponseMultiThreat() {
    std::cout << "\n=== GalacticResponse: MultiThreat ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Solari");
    sys.reportThreat("faction1", 5.0f);
    sys.reportThreat("faction1", 8.0f);
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 13.0f), "Threat accumulated to 13");
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 1, "Tier 1 at 13");
}

static void testGalacticResponseMissing() {
    std::cout << "\n=== GalacticResponse: Missing ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    assertTrue(!sys.initializeFaction("nonexistent", "Test"), "Init fails on missing entity");
    assertTrue(!sys.reportThreat("nonexistent", 10.0f), "Report fails on missing");
    assertTrue(!sys.dispatchReinforcement("nonexistent"), "Dispatch fails on missing");
    assertTrue(!sys.rerouteTradeFor("nonexistent", "sys1"), "Reroute fails on missing");
    assertTrue(approxEqual(sys.getThreatLevel("nonexistent"), 0.0f), "0 threat on missing");
    assertTrue(sys.getResponseTier("nonexistent") == 0, "0 tier on missing");
    assertTrue(sys.getReinforcementCount("nonexistent") == 0, "0 reinforcements on missing");
    assertTrue(sys.getReroutedSystemCount("nonexistent") == 0, "0 rerouted on missing");
    assertTrue(approxEqual(sys.getEscalationRate("nonexistent"), 0.0f), "0 escalation on missing");
    assertTrue(!sys.isFullMobilization("nonexistent"), "No mobilization on missing");
}


void run_galactic_response_curve_system_tests() {
    testGalacticResponseCreate();
    testGalacticResponseThreat();
    testGalacticResponseTierEscalation();
    testGalacticResponseReinforcement();
    testGalacticResponseReinforcementBlocked();
    testGalacticResponseReroute();
    testGalacticResponseDecay();
    testGalacticResponseDecayToZero();
    testGalacticResponseMultiThreat();
    testGalacticResponseMissing();
}
