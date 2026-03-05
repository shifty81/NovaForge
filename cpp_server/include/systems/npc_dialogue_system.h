#ifndef NOVAFORGE_SYSTEMS_NPC_DIALOGUE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_DIALOGUE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class NPCDialogueSystem : public ecs::SingleComponentSystem<components::NPCDialogue> {
public:
    explicit NPCDialogueSystem(ecs::World* world);
    ~NPCDialogueSystem() override = default;

    std::string getName() const override { return "NPCDialogueSystem"; }

    // Record that an NPC observed a legend event for a player
    void observeLegend(const std::string& npc_entity_id,
                       const std::string& player_id,
                       const std::string& event_type,
                       float timestamp);

    // Generate a dialogue line for an NPC referencing a player's legend
    std::string generateDialogue(const std::string& npc_entity_id,
                                 const std::string& player_entity_id);

    // Get number of generated dialogue lines for an NPC
    int getDialogueCount(const std::string& npc_entity_id) const;

    // Get all generated dialogue lines for an NPC
    std::vector<std::string> getDialogueLines(const std::string& npc_entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NPCDialogue& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_DIALOGUE_SYSTEM_H
