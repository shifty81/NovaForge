#ifndef NOVAFORGE_SYSTEMS_RIG_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RIG_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class RigSystem : public ecs::SingleComponentSystem<components::RigLoadout> {
public:
    explicit RigSystem(ecs::World* world);
    ~RigSystem() override = default;

    std::string getName() const override { return "RigSystem"; }

    bool installModule(const std::string& entity_id, const std::string& module_entity_id);
    bool removeModule(const std::string& entity_id, const std::string& module_entity_id);
    int getRackSize(const std::string& entity_id) const;
    int getInstalledCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::RigLoadout& loadout, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RIG_SYSTEM_H
