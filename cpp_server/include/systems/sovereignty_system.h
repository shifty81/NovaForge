#ifndef NOVAFORGE_SYSTEMS_SOVEREIGNTY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SOVEREIGNTY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class SovereigntySystem : public ecs::SingleComponentSystem<components::Sovereignty> {
public:
    explicit SovereigntySystem(ecs::World* world);
    ~SovereigntySystem() override = default;

    std::string getName() const override { return "SovereigntySystem"; }

    bool claimSovereignty(const std::string& system_entity_id,
                          const std::string& owner_id,
                          const std::string& system_name);

    bool relinquishSovereignty(const std::string& system_entity_id,
                               const std::string& requester_id);

    bool contestSovereignty(const std::string& system_entity_id);

    bool updateIndices(const std::string& system_entity_id,
                       float military_delta,
                       float industrial_delta);

    float getControlLevel(const std::string& system_entity_id);

    std::string getOwner(const std::string& system_entity_id);

    bool upgradeInfrastructure(const std::string& system_entity_id,
                               const std::string& requester_id);

protected:
    void updateComponent(ecs::Entity& entity, components::Sovereignty& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SOVEREIGNTY_SYSTEM_H
