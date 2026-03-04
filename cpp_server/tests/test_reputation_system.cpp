// Tests for: Reputation System Tests, AI Reputation Targeting Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/ai_system.h"
#include "systems/reputation_system.h"

using namespace atlas;

// ==================== Reputation System Tests ====================

static void testReputationInstallRelationships() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    float disp = sys.getFactionDisposition("Solari", "Veyren");
    assertTrue(disp < 0.0f, "Solari-Veyren are rivals (negative disposition)");

    float ally = sys.getFactionDisposition("Solari", "Aurelian");
    assertTrue(ally > 0.0f, "Solari-Aurelian are friendly (positive disposition)");
}

static void testReputationPirateHostile() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    float disp = sys.getFactionDisposition("Solari", "Serpentis");
    assertTrue(approxEqual(disp, -1.0f, 0.01f), "Player factions hostile to pirates");
}

static void testReputationModifyStanding() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    sys.modifyFactionStanding("player1", "Solari", 2.0f);
    float standing = sys.getEffectiveStanding("player1", "Solari");
    assertTrue(approxEqual(standing, 2.0f, 0.01f), "Direct standing applied");
}

static void testReputationDerivedEffects() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    // Gaining standing with Solari should affect allies/enemies
    sys.modifyFactionStanding("player1", "Solari", 4.0f);

    // Aurelian is friendly (0.3) → derived = 4.0 * 0.3 * 0.5 = 0.6
    float aurelian = sys.getEffectiveStanding("player1", "Aurelian");
    assertTrue(aurelian > 0.0f, "Derived positive standing with ally");

    // Veyren is rival (-0.5) → derived = 4.0 * -0.5 * 0.5 = -1.0
    float veyren = sys.getEffectiveStanding("player1", "Veyren");
    assertTrue(veyren < 0.0f, "Derived negative standing with rival");
}

static void testReputationAgentAccess() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    assertTrue(!sys.hasAgentAccess("player1", "Solari", 1.0f),
               "No access with 0 standing");

    sys.modifyFactionStanding("player1", "Solari", 3.0f);
    assertTrue(sys.hasAgentAccess("player1", "Solari", 1.0f),
               "Access with sufficient standing");
    assertTrue(!sys.hasAgentAccess("player1", "Solari", 5.0f),
               "No access when standing insufficient");
}

static void testReputationStandingClamped() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    sys.modifyFactionStanding("player1", "Solari", 15.0f);
    float standing = sys.getEffectiveStanding("player1", "Solari");
    assertTrue(standing <= 10.0f, "Standing clamped to max 10");

    sys.modifyFactionStanding("player1", "Veyren", -15.0f);
    float neg = sys.getEffectiveStanding("player1", "Veyren");
    assertTrue(neg >= -10.0f, "Standing clamped to min -10");
}


// ==================== AI Reputation Targeting Tests ====================

static void testAISkipsFriendlyTargets() {
    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create an NPC with faction
    auto* npc = world.createEntity("npc1");
    addComp<components::AI>(npc)->behavior = components::AI::Behavior::Aggressive;
    addComp<components::Position>(npc);
    addComp<components::Velocity>(npc);
    auto* npcFaction = addComp<components::Faction>(npc);
    npcFaction->faction_name = "Solari";

    // Create a player with positive standing toward Solari
    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 100.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Solari"] = 5.0f;

    ecs::Entity* target = aiSys.selectTarget(npc);
    assertTrue(target == nullptr, "AI does not target player with positive faction standing");
}

static void testAITargetsHostileEntities() {
    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->awareness_range = 100000.0f;
    addComp<components::Position>(npc);
    addComp<components::Velocity>(npc);
    auto* npcFaction = addComp<components::Faction>(npc);
    npcFaction->faction_name = "Serpentis";

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 100.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Serpentis"] = -5.0f;

    ecs::Entity* target = aiSys.selectTarget(npc);
    assertTrue(target != nullptr, "AI targets player with negative faction standing");
}

static void testAITargetsHostileNPCFaction() {
    ecs::World world;
    systems::AISystem aiSys(&world);
    constexpr float kTestAwarenessRange = 100000.0f;
    constexpr float kTestTargetDistance = 100.0f;

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->awareness_range = kTestAwarenessRange;
    addComp<components::Position>(npc);
    addComp<components::Velocity>(npc);
    auto* npcFaction = addComp<components::Faction>(npc);
    npcFaction->faction_name = "Solari";
    npcFaction->standings["Veyren"] = -5.0f;

    auto* hostileNpc = world.createEntity("npc2");
    addComp<components::AI>(hostileNpc);
    auto* hostilePos = addComp<components::Position>(hostileNpc);
    hostilePos->x = kTestTargetDistance;
    auto* hostileFaction = addComp<components::Faction>(hostileNpc);
    hostileFaction->faction_name = "Veyren";

    ecs::Entity* target = aiSys.selectTarget(npc);
    assertTrue(target == hostileNpc, "AI targets hostile NPC faction when no player target exists");
}


void run_reputation_system_tests() {
    testReputationInstallRelationships();
    testReputationPirateHostile();
    testReputationModifyStanding();
    testReputationDerivedEffects();
    testReputationAgentAccess();
    testReputationStandingClamped();
    testAISkipsFriendlyTargets();
    testAITargetsHostileEntities();
    testAITargetsHostileNPCFaction();
}
