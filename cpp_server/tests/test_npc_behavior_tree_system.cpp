// Tests for: NPC Behavior Tree System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/npc_behavior_tree_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== NPC Behavior Tree System Tests ====================

static void testNPCBehaviorTreeDefaults() {
    std::cout << "\n=== NPC Behavior Tree: Defaults ===" << std::endl;
    ecs::World world;

    auto* npc = world.createEntity("bt_npc");
    auto* behavior = addComp<components::NPCBehaviorState>(npc);

    assertTrue(behavior->phases.empty(), "Phases initially empty");
    assertTrue(behavior->current_phase_index == 0, "Phase index starts at 0");
    assertTrue(behavior->tree_complete == false, "Tree not complete initially");
    assertTrue(behavior->phase_duration == 10.0f, "Default phase duration 10s");
}

static void testNPCBehaviorTreeTraderPhases() {
    std::cout << "\n=== NPC Behavior Tree: Trader Trade Phases ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Trader, I::Trade);
    assertTrue(phases.size() == 5, "Trader trade has 5 phases");
    assertTrue(phases[0] == "CheckMarketPrices", "First phase is CheckMarketPrices");
    assertTrue(phases[4] == "EvaluateProfit", "Last phase is EvaluateProfit");
}

static void testNPCBehaviorTreeMinerPhases() {
    std::cout << "\n=== NPC Behavior Tree: Miner Mine Phases ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Miner, I::Mine);
    assertTrue(phases.size() == 5, "Miner mine has 5 phases");
    assertTrue(phases[0] == "FindDeposit", "First phase is FindDeposit");
    assertTrue(phases[2] == "ActivateLasers", "Third phase is ActivateLasers");
}

static void testNPCBehaviorTreePiratePhases() {
    std::cout << "\n=== NPC Behavior Tree: Pirate Hunt Phases ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Pirate, I::Hunt);
    assertTrue(phases.size() == 5, "Pirate hunt has 5 phases");
    assertTrue(phases[0] == "ScanForPrey", "First phase is ScanForPrey");
    assertTrue(phases[3] == "Engage", "Fourth phase is Engage");
}

static void testNPCBehaviorTreePhaseAdvancement() {
    std::cout << "\n=== NPC Behavior Tree: Phase Advancement ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_advance");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Trade;
    intent->archetype = components::SimNPCIntent::Archetype::Trader;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 5.0f;

    // First tick builds the tree
    btSys.update(0.0f);
    assertTrue(behavior->phases.size() == 5, "Phases populated from intent");
    assertTrue(btSys.getCurrentPhase("bt_advance") == "CheckMarketPrices",
               "Starts at first phase");

    // Advance time past phase_duration
    btSys.update(6.0f);
    assertTrue(behavior->current_phase_index == 1, "Advanced to phase 1");
    assertTrue(btSys.getCurrentPhase("bt_advance") == "BuyGoods",
               "Now in BuyGoods phase");
}

static void testNPCBehaviorTreeCompletion() {
    std::cout << "\n=== NPC Behavior Tree: Completion ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_complete");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Dock;
    intent->archetype = components::SimNPCIntent::Archetype::Trader;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 1.0f;

    // Build tree
    btSys.update(0.0f);
    int numPhases = static_cast<int>(behavior->phases.size());
    assertTrue(numPhases == 3, "Dock has 3 phases");

    // Advance through all phases
    for (int i = 0; i < numPhases; i++) {
        btSys.update(2.0f);
    }

    assertTrue(behavior->tree_complete, "Tree marked complete");
    assertTrue(intent->intent_complete, "Intent marked complete");
    assertTrue(btSys.isTreeComplete("bt_complete"), "Query confirms complete");
}

static void testNPCBehaviorTreeIntentChange() {
    std::cout << "\n=== NPC Behavior Tree: Intent Change Rebuilds Tree ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_change");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Trade;
    intent->archetype = components::SimNPCIntent::Archetype::Trader;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 1.0f;

    btSys.update(0.0f);
    assertTrue(behavior->phases.size() == 5, "Trade has 5 phases");

    // Change intent
    intent->current_intent = components::SimNPCIntent::Intent::Flee;
    btSys.update(0.0f);
    assertTrue(behavior->current_phase_index == 0, "Phase reset to 0");
    assertTrue(behavior->phases[0] == "SelectSafespot", "Flee starts at SelectSafespot");
    assertTrue(!behavior->tree_complete, "Tree no longer complete");
}

static void testNPCBehaviorTreeReset() {
    std::cout << "\n=== NPC Behavior Tree: Reset ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_reset");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Patrol;
    intent->archetype = components::SimNPCIntent::Archetype::Patrol;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 1.0f;

    btSys.update(0.0f);
    btSys.update(2.0f);
    assertTrue(behavior->current_phase_index > 0, "Advanced past phase 0");

    btSys.resetTree("bt_reset");
    assertTrue(behavior->current_phase_index == 0, "Reset to phase 0");
    assertTrue(behavior->phase_elapsed == 0.0f, "Elapsed reset to 0");
    assertTrue(!behavior->tree_complete, "Tree not complete after reset");
}

static void testNPCBehaviorTreeGenericPhases() {
    std::cout << "\n=== NPC Behavior Tree: Generic Phase Fallback ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    // Trader exploring uses generic explore phases
    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Trader, I::Explore);
    assertTrue(phases.size() == 3, "Generic explore has 3 phases");
    assertTrue(phases[0] == "SelectDestination", "Generic explore starts with SelectDestination");
}

static void testNPCBehaviorTreeMissingEntity() {
    std::cout << "\n=== NPC Behavior Tree: Missing Entity Queries ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    assertTrue(btSys.getCurrentPhase("nobody") == "", "Missing entity returns empty phase");
    assertTrue(btSys.isTreeComplete("nobody"), "Missing entity returns true for complete");
    assertTrue(btSys.getPhaseElapsed("nobody") == 0.0f, "Missing entity returns 0 elapsed");
}


void run_npc_behavior_tree_system_tests() {
    testNPCBehaviorTreeDefaults();
    testNPCBehaviorTreeTraderPhases();
    testNPCBehaviorTreeMinerPhases();
    testNPCBehaviorTreePiratePhases();
    testNPCBehaviorTreePhaseAdvancement();
    testNPCBehaviorTreeCompletion();
    testNPCBehaviorTreeIntentChange();
    testNPCBehaviorTreeReset();
    testNPCBehaviorTreeGenericPhases();
    testNPCBehaviorTreeMissingEntity();
}
