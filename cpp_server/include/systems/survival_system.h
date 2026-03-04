#ifndef NOVAFORGE_SYSTEMS_SURVIVAL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SURVIVAL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <tuple>

namespace atlas {
namespace systems {

class SurvivalSystem : public ecs::SingleComponentSystem<components::SurvivalNeeds> {
public:
    explicit SurvivalSystem(ecs::World* world);
    ~SurvivalSystem() override = default;

    std::string getName() const override { return "SurvivalSystem"; }

    float refillOxygen(const std::string& entity_id, float amount);
    float feed(const std::string& entity_id, float amount);
    float rest(const std::string& entity_id, float amount);
    bool isAlive(const std::string& entity_id) const;
    std::tuple<float, float, float> getNeeds(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SurvivalNeeds& needs, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SURVIVAL_SYSTEM_H
