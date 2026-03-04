// Tests for: Phase 2: Background Simulation System Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/background_simulation_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Phase 2: Background Simulation System Tests ====================

static void testSimStarSystemStateDefaults() {
    std::cout << "\n=== SimStarSystemState Defaults ===" << std::endl;
    ecs::World world;
    auto* sys = world.createEntity("system_alpha");
    auto* state = addComp<components::SimStarSystemState>(sys);

    assertTrue(approxEqual(state->traffic_level, 0.5f), "Default traffic 0.5");
    assertTrue(approxEqual(state->economic_index, 0.5f), "Default economy 0.5");
    assertTrue(approxEqual(state->security_level, 0.5f), "Default security 0.5");
    assertTrue(approxEqual(state->threat_level, 0.0f), "Default threat 0.0");
    assertTrue(approxEqual(state->pirate_activity, 0.0f), "Default pirate activity 0.0");
    assertTrue(approxEqual(state->resource_availability, 1.0f), "Default resources 1.0");
    assertTrue(!state->pirate_surge, "No pirate surge by default");
    assertTrue(!state->resource_shortage, "No shortage by default");
    assertTrue(!state->lockdown, "No lockdown by default");
}

static void testBackgroundSimThreatDecay() {
    std::cout << "\n=== Background Sim: Threat Decay ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_beta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.5f;

    bgSim.update(10.0f);  // 10 seconds

    assertTrue(state->threat_level < 0.5f, "Threat decayed after tick");
    assertTrue(state->threat_level > 0.0f, "Threat not fully gone after short tick");
}

static void testBackgroundSimEconomyRecovery() {
    std::cout << "\n=== Background Sim: Economy Recovery ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_gamma");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.2f;

    bgSim.update(10.0f);

    assertTrue(state->economic_index > 0.2f, "Economy recovered from low");
    assertTrue(state->economic_index <= 0.5f, "Economy doesn't exceed baseline");
}

static void testBackgroundSimResourceRegen() {
    std::cout << "\n=== Background Sim: Resource Regen ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_delta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->resource_availability = 0.3f;

    bgSim.update(10.0f);

    assertTrue(state->resource_availability > 0.3f, "Resources regenerated");
    assertTrue(state->resource_availability <= 1.0f, "Resources don't exceed max");
}

static void testBackgroundSimPirateSurge() {
    std::cout << "\n=== Background Sim: Pirate Surge Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_epsilon");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->pirate_activity = 0.8f;  // above threshold (0.7)

    bgSim.update(1.0f);

    assertTrue(state->pirate_surge, "Pirate surge triggered at high activity");
    assertTrue(state->event_timer > 0.0f, "Event timer set");
    assertTrue(bgSim.isEventActive("system_epsilon", "pirate_surge"),
               "isEventActive returns true for pirate_surge");
}

static void testBackgroundSimResourceShortage() {
    std::cout << "\n=== Background Sim: Resource Shortage Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_zeta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->resource_availability = 0.1f;  // below threshold (0.2)

    bgSim.update(1.0f);

    assertTrue(state->resource_shortage, "Resource shortage triggered");
    assertTrue(bgSim.isEventActive("system_zeta", "resource_shortage"),
               "isEventActive returns true for shortage");
}

static void testBackgroundSimLockdown() {
    std::cout << "\n=== Background Sim: Lockdown Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_eta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.9f;  // above threshold (0.8)

    bgSim.update(1.0f);

    assertTrue(state->lockdown, "Lockdown triggered at extreme threat");
    assertTrue(bgSim.isEventActive("system_eta", "lockdown"),
               "isEventActive returns true for lockdown");
}

static void testBackgroundSimEventTimerExpiry() {
    std::cout << "\n=== Background Sim: Event Timer Expiry ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);
    bgSim.event_duration = 10.0f;  // short duration for test

    auto* sys = world.createEntity("system_theta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->pirate_activity = 0.8f;

    bgSim.update(1.0f);
    assertTrue(state->pirate_surge, "Surge active initially");

    // Lower activity and wait for timer to expire
    state->pirate_activity = 0.1f;
    bgSim.update(15.0f);  // exceeds event_duration

    assertTrue(!state->pirate_surge, "Surge cleared after timer + conditions subsided");
}

static void testBackgroundSimQuerySystems() {
    std::cout << "\n=== Background Sim: Query Systems with Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys1 = world.createEntity("system_1");
    auto* state1 = addComp<components::SimStarSystemState>(sys1);
    state1->pirate_activity = 0.9f;

    auto* sys2 = world.createEntity("system_2");
    auto* state2 = addComp<components::SimStarSystemState>(sys2);
    state2->pirate_activity = 0.1f;

    bgSim.update(1.0f);

    auto surged = bgSim.getSystemsWithEvent("pirate_surge");
    assertTrue(surged.size() == 1, "Only one system has pirate surge");
    assertTrue(surged[0] == "system_1", "Correct system has surge");
}

static void testBackgroundSimPirateGrowth() {
    std::cout << "\n=== Background Sim: Pirate Growth in Low-Sec ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_lowsec");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.1f;  // low security
    state->pirate_activity = 0.3f;

    bgSim.update(10.0f);

    assertTrue(state->pirate_activity > 0.3f, "Pirate activity grew in low-sec");
}

static void testBackgroundSimPriceModifier() {
    std::cout << "\n=== Background Sim: Price Modifier ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_market");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->resource_availability = 0.3f;  // scarce
    state->trade_volume = 0.2f;           // low trade

    bgSim.update(1.0f);

    assertTrue(state->price_modifier > 1.0f, "Prices increase when resources scarce");
    assertTrue(state->price_modifier <= 2.0f, "Price modifier capped at 2.0");
}

static void testBackgroundSimNoEventOnNonEntity() {
    std::cout << "\n=== Background Sim: No Event on Missing Entity ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    assertTrue(!bgSim.isEventActive("nonexistent", "pirate_surge"),
               "No event on missing entity");
    assertTrue(bgSim.getSystemState("nonexistent") == nullptr,
               "Null state for missing entity");
}


void run_background_simulation_system_tests() {
    testSimStarSystemStateDefaults();
    testBackgroundSimThreatDecay();
    testBackgroundSimEconomyRecovery();
    testBackgroundSimResourceRegen();
    testBackgroundSimPirateSurge();
    testBackgroundSimResourceShortage();
    testBackgroundSimLockdown();
    testBackgroundSimEventTimerExpiry();
    testBackgroundSimQuerySystems();
    testBackgroundSimPirateGrowth();
    testBackgroundSimPriceModifier();
    testBackgroundSimNoEventOnNonEntity();
}
