// Tests for: Propaganda System Tests
#include "test_log.h"
#include "components/narrative_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/propaganda_system.h"

using namespace atlas;

// ==================== Propaganda System Tests ====================

static void testPropagandaDefaults() {
    std::cout << "\n=== Propaganda Defaults ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    auto myths = propSys.getMythsAbout("player1");
    assertTrue(myths.empty(), "No myths about unknown entity");
    
    int count = propSys.getActiveMythCount("player1");
    assertTrue(count == 0, "Active myth count is 0");
}

static void testPropagandaGenerateMyth() {
    std::cout << "\n=== Propaganda Generate Myth ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("hero_pilot", "Solari", "heroic", "Battle of Jita");
    assertTrue(!mythId.empty(), "Myth ID returned");
    
    float cred = propSys.getMythCredibility(mythId);
    assertTrue(approxEqual(cred, 1.0f), "New myth has full credibility");
    
    auto myths = propSys.getMythsAbout("hero_pilot");
    assertTrue(myths.size() == 1, "One myth about hero_pilot");
    assertTrue(myths[0].type == components::PropagandaNetwork::MythType::Heroic, "Myth type is heroic");
}

static void testPropagandaDebunk() {
    std::cout << "\n=== Propaganda Debunk ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("villain_npc", "Veyren", "villainous");
    float cred = propSys.debunkMyth(mythId, 0.7f);
    assertTrue(approxEqual(cred, 0.3f), "Credibility reduced by evidence strength");
    
    cred = propSys.debunkMyth(mythId, 0.5f);
    assertTrue(approxEqual(cred, 0.0f), "Credibility cannot go below 0");
    
    auto myths = propSys.getMythsAbout("villain_npc", true);
    assertTrue(myths[0].debunked, "Myth marked as debunked");
}

static void testPropagandaSpread() {
    std::cout << "\n=== Propaganda Spread ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("mystery_trader", "Keldari", "mysterious");
    
    auto myths = propSys.getMythsAbout("mystery_trader");
    int initialSpread = myths[0].spread_count;
    
    propSys.spreadMyth(mythId, "system_alpha", 0.1f);
    
    myths = propSys.getMythsAbout("mystery_trader");
    assertTrue(myths[0].spread_count == initialSpread + 1, "Spread count increased");
}

static void testPropagandaNPCBelief() {
    std::cout << "\n=== Propaganda NPC Belief ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("legend_pilot", "Aurelian", "exaggerated");
    assertTrue(propSys.npcBelievesMyth("npc_1", mythId), "NPCs believe high credibility myths");
    
    propSys.debunkMyth(mythId, 0.8f);
    assertTrue(!propSys.npcBelievesMyth("npc_1", mythId), "NPCs don't believe low credibility myths");
}

static void testPropagandaMythTypeName() {
    std::cout << "\n=== Propaganda Myth Type Name ===" << std::endl;
    assertTrue(systems::PropagandaSystem::getMythTypeName(0) == "Heroic", "Type 0 is Heroic");
    assertTrue(systems::PropagandaSystem::getMythTypeName(1) == "Villainous", "Type 1 is Villainous");
    assertTrue(systems::PropagandaSystem::getMythTypeName(2) == "Mysterious", "Type 2 is Mysterious");
    assertTrue(systems::PropagandaSystem::getMythTypeName(3) == "Exaggerated", "Type 3 is Exaggerated");
    assertTrue(systems::PropagandaSystem::getMythTypeName(4) == "Fabricated", "Type 4 is Fabricated");
}

static void testPropagandaDecay() {
    std::cout << "\n=== Propaganda Decay ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("fading_legend", "Solari", "heroic");
    float initialCred = propSys.getMythCredibility(mythId);
    
    // Update should decay credibility slightly
    propSys.update(100.0f);
    float newCred = propSys.getMythCredibility(mythId);
    assertTrue(newCred < initialCred, "Credibility decays over time");
}


void run_propaganda_system_tests() {
    testPropagandaDefaults();
    testPropagandaGenerateMyth();
    testPropagandaDebunk();
    testPropagandaSpread();
    testPropagandaNPCBelief();
    testPropagandaMythTypeName();
    testPropagandaDecay();
}
