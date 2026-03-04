#ifndef NOVAFORGE_SYSTEMS_FLEET_MORALE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_MORALE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetMoraleSystem : public ecs::SingleComponentSystem<components::FleetMorale> {
public:
    explicit FleetMoraleSystem(ecs::World* world);
    ~FleetMoraleSystem() override = default;

    std::string getName() const override { return "FleetMoraleSystem"; }

    // Record combat outcomes
    void recordWin(const std::string& entity_id);
    void recordLoss(const std::string& entity_id);
    void recordShipLost(const std::string& entity_id);
    void recordSavedByPlayer(const std::string& entity_id);
    void recordPlayerSaved(const std::string& entity_id);
    void recordMissionTogether(const std::string& entity_id);

    // Queries
    float getMoraleScore(const std::string& entity_id) const;
    std::string getMoraleState(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetMorale& morale, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_MORALE_SYSTEM_H
