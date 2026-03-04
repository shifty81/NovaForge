#ifndef NOVAFORGE_SYSTEMS_ANCIENT_TECH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ANCIENT_TECH_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class AncientTechSystem : public ecs::SingleComponentSystem<components::AncientTechModule> {
public:
    explicit AncientTechSystem(ecs::World* world);
    ~AncientTechSystem() override = default;

    std::string getName() const override { return "AncientTechSystem"; }

    bool startRepair(const std::string& entity_id);
    std::string reverseEngineer(const std::string& entity_id);
    components::AncientTechModule::TechState getState(const std::string& entity_id) const;
    bool isUsable(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AncientTechModule& tech, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ANCIENT_TECH_SYSTEM_H
