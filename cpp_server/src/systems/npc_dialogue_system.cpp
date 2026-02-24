#include "systems/npc_dialogue_system.h"
#include "systems/legend_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

NPCDialogueSystem::NPCDialogueSystem(ecs::World* world)
    : System(world) {
}

void NPCDialogueSystem::update(float /*delta_time*/) {
    // Dialogue generation is event-driven via generateDialogue
}

void NPCDialogueSystem::observeLegend(const std::string& npc_entity_id,
                                       const std::string& player_id,
                                       const std::string& event_type,
                                       float timestamp) {
    auto* entity = world_->getEntity(npc_entity_id);
    if (!entity) return;
    auto* dialogue = entity->getComponent<components::NPCDialogue>();
    if (!dialogue) return;
    dialogue->observeLegend(player_id, event_type, timestamp);
}

std::string NPCDialogueSystem::generateDialogue(const std::string& npc_entity_id,
                                                 const std::string& player_entity_id) {
    auto* npc_entity = world_->getEntity(npc_entity_id);
    if (!npc_entity) return "";
    auto* dialogue = npc_entity->getComponent<components::NPCDialogue>();
    if (!dialogue) return "";

    // Look up player legend
    auto* player_entity = world_->getEntity(player_entity_id);
    if (!player_entity) return "";
    auto* legend = player_entity->getComponent<components::PlayerLegend>();
    if (!legend || legend->legend_score == 0) {
        std::string line = "You're new around here, aren't you?";
        dialogue->addLine(line);
        return line;
    }

    std::string title = LegendSystem::computeTitle(legend->legend_score);
    std::string line;
    if (title == "Mythic") {
        line = "You are the " + title + " — tales of your deeds echo across the stars.";
    } else if (title == "Legendary") {
        line = "A " + title + " walks among us! I've heard of your exploits in distant systems.";
    } else if (title == "Famous") {
        line = "Aren't you that " + title + " pilot? Word travels fast in this sector.";
    } else if (title == "Notable") {
        line = "I've heard your name mentioned a few times. Keep it up, pilot.";
    } else {
        line = "You're new around here, aren't you?";
    }
    dialogue->addLine(line);
    return line;
}

int NPCDialogueSystem::getDialogueCount(const std::string& npc_entity_id) const {
    auto* entity = world_->getEntity(npc_entity_id);
    if (!entity) return 0;
    auto* dialogue = entity->getComponent<components::NPCDialogue>();
    if (!dialogue) return 0;
    return dialogue->getLineCount();
}

std::vector<std::string> NPCDialogueSystem::getDialogueLines(
    const std::string& npc_entity_id) const {
    auto* entity = world_->getEntity(npc_entity_id);
    if (!entity) return {};
    auto* dialogue = entity->getComponent<components::NPCDialogue>();
    if (!dialogue) return {};
    return dialogue->generated_lines;
}

} // namespace systems
} // namespace atlas
