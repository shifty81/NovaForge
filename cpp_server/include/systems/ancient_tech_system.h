#ifndef EVE_SYSTEMS_ANCIENT_TECH_SYSTEM_H
#define EVE_SYSTEMS_ANCIENT_TECH_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class AncientTechSystem : public ecs::System {
public:
    explicit AncientTechSystem(ecs::World* world);
    ~AncientTechSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AncientTechSystem"; }

    bool startRepair(const std::string& entity_id);
    std::string reverseEngineer(const std::string& entity_id);
    components::AncientTechModule::TechState getState(const std::string& entity_id) const;
    bool isUsable(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_ANCIENT_TECH_SYSTEM_H
