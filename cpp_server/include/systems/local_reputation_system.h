#ifndef NOVAFORGE_SYSTEMS_LOCAL_REPUTATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LOCAL_REPUTATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Per-system player reputation tracking with decay
 */
class LocalReputationSystem : public ecs::SingleComponentSystem<components::LocalReputation> {
public:
    explicit LocalReputationSystem(ecs::World* world);
    ~LocalReputationSystem() override = default;

    std::string getName() const override { return "LocalReputationSystem"; }

    // --- API ---
    void modifyReputation(const std::string& system_id, const std::string& player_id, float amount);
    float getReputation(const std::string& system_id, const std::string& player_id) const;
    std::string getStanding(const std::string& system_id, const std::string& player_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::LocalReputation& rep, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LOCAL_REPUTATION_SYSTEM_H
