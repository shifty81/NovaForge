#ifndef NOVAFORGE_SYSTEMS_INFORMATION_PROPAGATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INFORMATION_PROPAGATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Propagates rumors about player actions between star systems
 *
 * Rumors decay over time and propagate to neighboring systems with
 * reduced belief strength. Confirmed rumors reinforce belief.
 */
class InformationPropagationSystem : public ecs::SingleComponentSystem<components::InformationPropagation> {
public:
    explicit InformationPropagationSystem(ecs::World* world);
    ~InformationPropagationSystem() override = default;

    std::string getName() const override { return "InformationPropagationSystem"; }

    // --- API ---
    void reportPlayerAction(const std::string& system_id, const std::string& player_id,
                            const std::string& action_type);
    std::vector<components::InformationPropagation::Rumor> getRumors(const std::string& system_id) const;
    std::vector<components::InformationPropagation::Rumor> getRumorsAboutPlayer(
        const std::string& system_id, const std::string& player_id) const;
    int getRumorCount(const std::string& system_id) const;
    float getPlayerNotoriety(const std::string& system_id, const std::string& player_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::InformationPropagation& info, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INFORMATION_PROPAGATION_SYSTEM_H
