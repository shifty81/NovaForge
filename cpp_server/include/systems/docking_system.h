#ifndef NOVAFORGE_SYSTEMS_DOCKING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DOCKING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class DockingSystem : public ecs::SingleComponentSystem<components::DockingPort> {
public:
    explicit DockingSystem(ecs::World* world);
    ~DockingSystem() override = default;

    std::string getName() const override { return "DockingSystem"; }

    bool dock(const std::string& port_entity_id, const std::string& ship_entity_id);
    std::string undock(const std::string& port_entity_id);
    bool extendDockingRing(const std::string& entity_id);
    bool retractDockingRing(const std::string& entity_id);
    bool isOccupied(const std::string& entity_id) const;
    std::string getDockedEntity(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::DockingPort& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DOCKING_SYSTEM_H
