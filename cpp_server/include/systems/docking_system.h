#ifndef EVE_SYSTEMS_DOCKING_SYSTEM_H
#define EVE_SYSTEMS_DOCKING_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class DockingSystem : public ecs::System {
public:
    explicit DockingSystem(ecs::World* world);
    ~DockingSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "DockingSystem"; }

    bool dock(const std::string& port_entity_id, const std::string& ship_entity_id);
    std::string undock(const std::string& port_entity_id);
    bool extendDockingRing(const std::string& entity_id);
    bool retractDockingRing(const std::string& entity_id);
    bool isOccupied(const std::string& entity_id) const;
    std::string getDockedEntity(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_DOCKING_SYSTEM_H
