// Tests for: Phase 2: NPC Intent System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/npc_intent_system.h"

using namespace atlas;

// ==================== Phase 2: NPC Intent System Tests ====================

static void testSimNPCIntentDefaults() {
    std::cout << "\n=== SimNPCIntent Defaults ===" << std::endl;
    ecs::World world;
    auto* npc = world.createEntity("npc_trader_1");
    auto* intent = addComp<components::SimNPCIntent>(npc);

    assertTrue(intent->current_intent == components::SimNPCIntent::Intent::Idle,
               "Default intent is Idle");
    assertTrue(intent->archetype == components::SimNPCIntent::Archetype::Trader,
               "Default archetype is Trader");
    assertTrue(approxEqual(intent->wallet, 10000.0f, 1.0f), "Default wallet 10000");
    assertTrue(approxEqual(intent->cargo_fill, 0.0f), "Default cargo empty");
    assertTrue(!intent->intent_complete, "Intent not complete by default");
}

static void testNPCIntentArchetypeWeights() {
    std::cout << "\n=== NPC Intent: Archetype Weights ===" << std::endl;

    // Test Trader archetype
    components::SimNPCIntent trader_intent;
    trader_intent.archetype = components::SimNPCIntent::Archetype::Trader;
    systems::NPCIntentSystem::applyArchetypeWeights(&trader_intent);
    assertTrue(trader_intent.trade_weight > 0.8f, "Trader has high trade weight");
    assertTrue(trader_intent.haul_weight > 0.5f, "Trader has decent haul weight");

    // Test Pirate archetype
    components::SimNPCIntent pirate_intent;
    pirate_intent.archetype = components::SimNPCIntent::Archetype::Pirate;
    systems::NPCIntentSystem::applyArchetypeWeights(&pirate_intent);
    assertTrue(pirate_intent.hunt_weight > 0.8f, "Pirate has high hunt weight");
    assertTrue(pirate_intent.salvage_weight > 0.4f, "Pirate has decent salvage weight");

    // Test Miner archetype
    components::SimNPCIntent miner_intent;
    miner_intent.archetype = components::SimNPCIntent::Archetype::Miner;
    systems::NPCIntentSystem::applyArchetypeWeights(&miner_intent);
    assertTrue(miner_intent.mine_weight > 0.8f, "Miner has high mine weight");
    assertTrue(miner_intent.flee_weight > 0.6f, "Miner has high flee weight (cautious)");

    // Test Patrol archetype
    components::SimNPCIntent patrol_intent;
    patrol_intent.archetype = components::SimNPCIntent::Archetype::Patrol;
    systems::NPCIntentSystem::applyArchetypeWeights(&patrol_intent);
    assertTrue(patrol_intent.patrol_weight > 0.8f, "Patrol has high patrol weight");
    assertTrue(patrol_intent.escort_weight > 0.6f, "Patrol has decent escort weight");

    // Test Hauler archetype
    components::SimNPCIntent hauler_intent;
    hauler_intent.archetype = components::SimNPCIntent::Archetype::Hauler;
    systems::NPCIntentSystem::applyArchetypeWeights(&hauler_intent);
    assertTrue(hauler_intent.haul_weight > 0.8f, "Hauler has high haul weight");

    // Test Industrialist archetype
    components::SimNPCIntent ind_intent;
    ind_intent.archetype = components::SimNPCIntent::Archetype::Industrialist;
    systems::NPCIntentSystem::applyArchetypeWeights(&ind_intent);
    assertTrue(ind_intent.trade_weight > 0.6f, "Industrialist has decent trade weight");
    assertTrue(ind_intent.mine_weight > 0.5f, "Industrialist has decent mine weight");
}

static void testNPCIntentFleeOnLowHealth() {
    std::cout << "\n=== NPC Intent: Flee on Low Health ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;  // instant re-eval for testing

    auto* npc = world.createEntity("npc_hurt");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Patrol;
    intent->archetype = components::SimNPCIntent::Archetype::Patrol;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);

    auto* health = addComp<components::Health>(npc);
    health->hull_hp = 10.0f;
    health->hull_max = 100.0f;  // 10% hull

    intentSys.update(1.0f);

    assertTrue(intent->current_intent == components::SimNPCIntent::Intent::Flee,
               "NPC flees when hull below 25%");
}

static void testNPCIntentTraderInGoodEconomy() {
    std::cout << "\n=== NPC Intent: Trader Favors Trade in Good Economy ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;

    // Create system with good economy
    auto* sys = world.createEntity("system_rich");
    auto* sysState = addComp<components::SimStarSystemState>(sys);
    sysState->economic_index = 0.9f;
    sysState->security_level = 0.8f;
    sysState->resource_availability = 0.8f;
    sysState->trade_volume = 0.7f;

    auto* npc = world.createEntity("npc_trader");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Trader;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    intent->target_system_id = "system_rich";
    intent->cargo_fill = 0.6f;  // has cargo to sell
    addComp<components::Health>(npc);

    intentSys.update(1.0f);

    // Trader should choose Trade (high econ + cargo)
    auto scores = intentSys.scoreIntents("npc_trader");
    assertTrue(!scores.empty(), "Score intents returns results");
    assertTrue(scores[0].first == components::SimNPCIntent::Intent::Trade,
               "Trade scores highest for trader in good economy with cargo");
}

static void testNPCIntentMinerInResourceSystem() {
    std::cout << "\n=== NPC Intent: Miner Prefers Mining in Rich System ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;

    auto* sys = world.createEntity("system_ore");
    auto* sysState = addComp<components::SimStarSystemState>(sys);
    sysState->resource_availability = 0.9f;
    sysState->security_level = 0.8f;

    auto* npc = world.createEntity("npc_miner");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Miner;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    intent->target_system_id = "system_ore";
    addComp<components::Health>(npc);

    auto scores = intentSys.scoreIntents("npc_miner");
    assertTrue(!scores.empty(), "Miner score intents not empty");

    // Find mine score
    float mine_score = 0.0f;
    for (auto& [i, s] : scores) {
        if (i == components::SimNPCIntent::Intent::Mine) mine_score = s;
    }
    assertTrue(mine_score > 0.5f, "Mining scores high in resource-rich system");
}

static void testNPCIntentForceIntent() {
    std::cout << "\n=== NPC Intent: Force Intent ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto* npc = world.createEntity("npc_forced");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Idle;

    intentSys.forceIntent("npc_forced", components::SimNPCIntent::Intent::Dock);

    assertTrue(intent->current_intent == components::SimNPCIntent::Intent::Dock,
               "Intent forced to Dock");
    assertTrue(intent->previous_intent == components::SimNPCIntent::Intent::Idle,
               "Previous intent recorded");
    assertTrue(intent->intent_duration == 0.0f, "Intent duration reset");
}

static void testNPCIntentQueryByIntent() {
    std::cout << "\n=== NPC Intent: Query NPCs by Intent ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto* npc1 = world.createEntity("npc_patrol_1");
    auto* i1 = addComp<components::SimNPCIntent>(npc1);
    i1->current_intent = components::SimNPCIntent::Intent::Patrol;

    auto* npc2 = world.createEntity("npc_patrol_2");
    auto* i2 = addComp<components::SimNPCIntent>(npc2);
    i2->current_intent = components::SimNPCIntent::Intent::Patrol;

    auto* npc3 = world.createEntity("npc_trade_1");
    auto* i3 = addComp<components::SimNPCIntent>(npc3);
    i3->current_intent = components::SimNPCIntent::Intent::Trade;

    auto patrollers = intentSys.getNPCsWithIntent(components::SimNPCIntent::Intent::Patrol);
    assertTrue(patrollers.size() == 2, "Two patrolling NPCs found");

    auto traders = intentSys.getNPCsWithIntent(components::SimNPCIntent::Intent::Trade);
    assertTrue(traders.size() == 1, "One trading NPC found");
}

static void testNPCIntentQueryByArchetype() {
    std::cout << "\n=== NPC Intent: Query NPCs by Archetype ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto* npc1 = world.createEntity("npc_m1");
    auto* i1 = addComp<components::SimNPCIntent>(npc1);
    i1->archetype = components::SimNPCIntent::Archetype::Miner;

    auto* npc2 = world.createEntity("npc_m2");
    auto* i2 = addComp<components::SimNPCIntent>(npc2);
    i2->archetype = components::SimNPCIntent::Archetype::Miner;

    auto* npc3 = world.createEntity("npc_p1");
    auto* i3 = addComp<components::SimNPCIntent>(npc3);
    i3->archetype = components::SimNPCIntent::Archetype::Pirate;

    auto miners = intentSys.getNPCsByArchetype(components::SimNPCIntent::Archetype::Miner);
    assertTrue(miners.size() == 2, "Two miners found");

    auto pirates = intentSys.getNPCsByArchetype(components::SimNPCIntent::Archetype::Pirate);
    assertTrue(pirates.size() == 1, "One pirate found");
}

static void testNPCIntentCooldownPreventsReeval() {
    std::cout << "\n=== NPC Intent: Cooldown Prevents Re-evaluation ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 30.0f;  // 30 second cooldown

    auto* npc = world.createEntity("npc_cooldown");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Patrol;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    addComp<components::Health>(npc);

    // First update sets cooldown
    intentSys.update(1.0f);
    auto first_intent = intent->current_intent;

    // Second update within cooldown — intent should not change
    intentSys.update(1.0f);
    assertTrue(intent->current_intent == first_intent,
               "Intent unchanged during cooldown");
}

static void testNPCIntentDockOnFullCargo() {
    std::cout << "\n=== NPC Intent: Dock on Full Cargo ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;

    auto* npc = world.createEntity("npc_full_cargo");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Hauler;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    intent->cargo_fill = 0.95f;  // nearly full
    addComp<components::Health>(npc);

    auto scores = intentSys.scoreIntents("npc_full_cargo");
    float dock_score = 0.0f;
    for (auto& [i, s] : scores) {
        if (i == components::SimNPCIntent::Intent::Dock) dock_score = s;
    }
    assertTrue(dock_score > 0.5f, "Dock scores high when cargo full");
}

static void testNPCIntentGetIntentMissing() {
    std::cout << "\n=== NPC Intent: Get Intent on Missing Entity ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto result = intentSys.getIntent("nonexistent");
    assertTrue(result == components::SimNPCIntent::Intent::Idle,
               "Missing entity returns Idle");
}


void run_npc_intent_system_tests() {
    testSimNPCIntentDefaults();
    testNPCIntentArchetypeWeights();
    testNPCIntentFleeOnLowHealth();
    testNPCIntentTraderInGoodEconomy();
    testNPCIntentMinerInResourceSystem();
    testNPCIntentForceIntent();
    testNPCIntentQueryByIntent();
    testNPCIntentQueryByArchetype();
    testNPCIntentCooldownPreventsReeval();
    testNPCIntentDockOnFullCargo();
    testNPCIntentGetIntentMissing();
}
