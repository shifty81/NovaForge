// Tests for: Visual Cue System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "components/narrative_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/supply_demand_system.h"
#include "systems/visual_cue_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Visual Cue System Tests ====================

static void testVisualCueDefaults() {
    std::cout << "\n=== Visual Cue Defaults ===" << std::endl;
    components::VisualCue cue;
    assertTrue(cue.lockdown_active == false, "Default lockdown_active is false");
    assertTrue(cue.lockdown_intensity == 0.0f, "Default lockdown_intensity is 0");
    assertTrue(cue.traffic_density == 0.0f, "Default traffic_density is 0");
    assertTrue(cue.traffic_ship_count == 0, "Default traffic_ship_count is 0");
    assertTrue(cue.threat_glow == 0.0f, "Default threat_glow is 0");
    assertTrue(cue.prosperity_indicator == 0.5f, "Default prosperity_indicator is 0.5");
    assertTrue(cue.pirate_warning == 0.0f, "Default pirate_warning is 0");
    assertTrue(cue.resource_highlight == 0.5f, "Default resource_highlight is 0.5");
    assertTrue(cue.dominant_faction.empty(), "Default dominant_faction is empty");
    assertTrue(cue.faction_influence_strength == 0.0f, "Default faction_influence_strength is 0");
}

static void testVisualCueLockdown() {
    std::cout << "\n=== Visual Cue Lockdown ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_alpha");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->threat_level = 0.9f;
    state->security_level = 0.5f;

    vcSys.update(1.0f);
    assertTrue(vcSys.isLockdownActive("system_alpha") == true, "High threat triggers lockdown");

    state->threat_level = 0.1f;
    state->security_level = 0.1f;
    vcSys.update(1.0f);
    assertTrue(vcSys.isLockdownActive("system_alpha") == true, "Low security triggers lockdown");

    state->threat_level = 0.3f;
    state->security_level = 0.5f;
    vcSys.update(1.0f);
    assertTrue(vcSys.isLockdownActive("system_alpha") == false, "Moderate values no lockdown");
}

static void testVisualCueTrafficDensity() {
    std::cout << "\n=== Visual Cue Traffic Density ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_beta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->traffic_level = 0.75f;

    vcSys.update(1.0f);
    float td = vcSys.getTrafficDensity("system_beta");
    assertTrue(td > 0.74f && td < 0.76f, "Traffic density mapped from sim state");
    auto* cue = sys->getComponent<components::VisualCue>();
    assertTrue(cue->traffic_ship_count == 75, "Traffic ship count is traffic * 100");
}

static void testVisualCueThreatGlow() {
    std::cout << "\n=== Visual Cue Threat Glow ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_gamma");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->threat_level = 0.6f;

    vcSys.update(1.0f);
    float glow = vcSys.getThreatGlow("system_gamma");
    assertTrue(glow > 0.59f && glow < 0.61f, "Threat glow maps from threat_level");
}

static void testVisualCueProsperity() {
    std::cout << "\n=== Visual Cue Prosperity ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_delta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->economic_index = 0.85f;

    vcSys.update(1.0f);
    float p = vcSys.getProsperityIndicator("system_delta");
    assertTrue(p > 0.84f && p < 0.86f, "Prosperity maps from economic_index");
}

static void testVisualCuePirateWarning() {
    std::cout << "\n=== Visual Cue Pirate Warning ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_epsilon");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->pirate_activity = 0.7f;

    vcSys.update(1.0f);
    float pw = vcSys.getPirateWarning("system_epsilon");
    assertTrue(pw > 0.69f && pw < 0.71f, "Pirate warning maps from pirate_activity");
}

static void testSupplyDemandDefaults() {
    std::cout << "\n=== Supply/Demand Defaults ===" << std::endl;
    components::SupplyDemand sd;
    assertTrue(sd.commodities.empty(), "Default commodities is empty");
    assertTrue(approxEqual(sd.price_elasticity, 0.5f), "Default price_elasticity is 0.5");
    assertTrue(approxEqual(sd.npc_activity_modifier, 1.0f), "Default npc_activity_modifier is 1.0");
    assertTrue(approxEqual(sd.price_floor_multiplier, 0.2f), "Default price_floor_multiplier is 0.2");
    assertTrue(approxEqual(sd.price_ceiling_multiplier, 5.0f), "Default price_ceiling_multiplier is 5.0");
    assertTrue(approxEqual(sd.supply_decay_rate, 0.01f), "Default supply_decay_rate is 0.01");
    assertTrue(approxEqual(sd.demand_drift_rate, 0.005f), "Default demand_drift_rate is 0.005");
}

static void testSupplyDemandAddCommodity() {
    std::cout << "\n=== Supply/Demand Add Commodity ===" << std::endl;
    components::SupplyDemand sd;
    sd.addCommodity("ore", 50.0f, 200.0f, 150.0f);
    assertTrue(sd.getCommodityCount() == 1, "One commodity added");
    auto* c = sd.getCommodity("ore");
    assertTrue(c != nullptr, "Commodity found by id");
    assertTrue(approxEqual(c->base_price, 50.0f), "Base price is 50");
    assertTrue(approxEqual(c->supply, 200.0f), "Initial supply is 200");
    assertTrue(approxEqual(c->demand, 150.0f), "Initial demand is 150");
    sd.addCommodity("ore", 999.0f, 999.0f, 999.0f);
    assertTrue(sd.getCommodityCount() == 1, "Duplicate commodity not added");
    assertTrue(sd.getCommodity("nonexistent") == nullptr, "Unknown commodity returns null");
}

static void testSupplyDemandPriceCalculation() {
    std::cout << "\n=== Supply/Demand Price Calculation ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_alpha");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 50.0f, 150.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 0.0f;

    sdSys.update(1.0f);
    auto* c = sd->getCommodity("ore");
    assertTrue(c->current_price > 100.0f, "Price increases when demand > supply");

    // Reset: supply > demand
    c->supply = 200.0f;
    c->demand = 50.0f;
    sdSys.update(1.0f);
    assertTrue(c->current_price < 100.0f, "Price decreases when supply > demand");
}

static void testSupplyDemandPriceFloor() {
    std::cout << "\n=== Supply/Demand Price Floor ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_floor");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 10000.0f, 1.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 0.0f;
    sd->price_elasticity = 10.0f;

    sdSys.update(1.0f);
    auto* c = sd->getCommodity("ore");
    float floor = 100.0f * sd->price_floor_multiplier;
    assertTrue(c->current_price >= floor, "Price does not go below floor");
    assertTrue(approxEqual(c->current_price, floor, 0.01f), "Price clamped to floor");
}

static void testSupplyDemandPriceCeiling() {
    std::cout << "\n=== Supply/Demand Price Ceiling ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_ceiling");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 0.1f, 10000.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 0.0f;
    sd->price_elasticity = 10.0f;

    sdSys.update(1.0f);
    auto* c = sd->getCommodity("ore");
    float ceiling = 100.0f * sd->price_ceiling_multiplier;
    assertTrue(c->current_price <= ceiling, "Price does not exceed ceiling");
    assertTrue(approxEqual(c->current_price, ceiling, 0.01f), "Price clamped to ceiling");
}

static void testSupplyDemandNPCModifier() {
    std::cout << "\n=== Supply/Demand NPC Modifier ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_npc");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 100.0f, 100.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 10.0f;

    float supply_before = sd->commodities[0].supply;
    sd->npc_activity_modifier = 2.0f;
    sdSys.update(1.0f);
    float supply_after_2x = sd->commodities[0].supply;
    float gained_2x = supply_after_2x - supply_before;

    // Reset
    sd->commodities[0].supply = 100.0f;
    sd->npc_activity_modifier = 1.0f;
    sdSys.update(1.0f);
    float supply_after_1x = sd->commodities[0].supply;
    float gained_1x = supply_after_1x - 100.0f;

    assertTrue(approxEqual(gained_2x, gained_1x * 2.0f, 0.01f), "2x NPC modifier doubles supply gain");
}


void run_visual_cue_system_tests() {
    testVisualCueDefaults();
    testVisualCueLockdown();
    testVisualCueTrafficDensity();
    testVisualCueThreatGlow();
    testVisualCueProsperity();
    testVisualCuePirateWarning();
    testSupplyDemandDefaults();
    testSupplyDemandAddCommodity();
    testSupplyDemandPriceCalculation();
    testSupplyDemandPriceFloor();
    testSupplyDemandPriceCeiling();
    testSupplyDemandNPCModifier();
}
