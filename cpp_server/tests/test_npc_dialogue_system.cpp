// Tests for: NPC Dialogue System tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/narrative_components.h"
#include "ecs/system.h"
#include "systems/npc_dialogue_system.h"

using namespace atlas;

// ==================== NPC Dialogue System tests ====================

static void testNPCDialogueUnknownPlayer() {
    std::cout << "\n=== NPC Dialogue Unknown Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 0;

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for unknown player");
    assertTrue(line.find("new") != std::string::npos, "Line mentions being new");
    assertTrue(dialogueSys.getDialogueCount("npc1") == 1, "One line generated");
}

static void testNPCDialogueFamousPlayer() {
    std::cout << "\n=== NPC Dialogue Famous Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 60; // Famous

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for famous player");
    assertTrue(line.find("Famous") != std::string::npos, "Line mentions Famous title");
}

static void testNPCDialogueLegendaryPlayer() {
    std::cout << "\n=== NPC Dialogue Legendary Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 150; // Legendary

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for legendary player");
    assertTrue(line.find("Legendary") != std::string::npos, "Line mentions Legendary title");
}

static void testNPCDialogueMythicPlayer() {
    std::cout << "\n=== NPC Dialogue Mythic Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 600; // Mythic

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for mythic player");
    assertTrue(line.find("Mythic") != std::string::npos, "Line mentions Mythic title");
}

static void testNPCDialogueObserveLegend() {
    std::cout << "\n=== NPC Dialogue Observe Legend ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    dialogueSys.observeLegend("npc1", "player1", "titan_kill", 100.0f);
    auto* npc = npcEnt->getComponent<components::NPCDialogue>();
    assertTrue(npc->getObservedCount() == 1, "NPC observed one legend event");
}

static void testNPCDialogueMissingComponents() {
    std::cout << "\n=== NPC Dialogue Missing Components ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    // No entity created — should return empty string safely
    std::string line = dialogueSys.generateDialogue("no_npc", "no_player");
    assertTrue(line.empty(), "No crash with missing entities");
    assertTrue(dialogueSys.getDialogueCount("no_npc") == 0, "Count is 0 for missing entity");
}


void run_npc_dialogue_system_tests() {
    testNPCDialogueUnknownPlayer();
    testNPCDialogueFamousPlayer();
    testNPCDialogueLegendaryPlayer();
    testNPCDialogueMythicPlayer();
    testNPCDialogueObserveLegend();
    testNPCDialogueMissingComponents();
}
