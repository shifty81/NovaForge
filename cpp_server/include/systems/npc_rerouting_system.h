#ifndef NOVAFORGE_SYSTEMS_NPC_REROUTING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_REROUTING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Reroutes NPCs away from dangerous star systems
 */
class NPCReroutingSystem : public ecs::SingleComponentSystem<components::NPCRouteState> {
public:
    explicit NPCReroutingSystem(ecs::World* world);
    ~NPCReroutingSystem() override = default;

    std::string getName() const override { return "NPCReroutingSystem"; }

    // --- Query API ---
    bool isRerouting(const std::string& entity_id) const;
    void setRoute(const std::string& entity_id, const std::vector<std::string>& route);
    std::vector<std::string> getRoute(const std::string& entity_id) const;

    // --- Configuration ---
    float reroute_interval = 60.0f;

protected:
    void updateComponent(ecs::Entity& entity, components::NPCRouteState& route, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_REROUTING_SYSTEM_H
