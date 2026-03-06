#ifndef NOVAFORGE_SYSTEMS_FPS_COMPANION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_COMPANION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief VIP/companion NPC follow mechanics for rescue objectives
 *
 * Manages companion state, follow behavior, morale under fire,
 * injury/healing, and rescue completion.
 */
class FPSCompanionSystem : public ecs::SingleComponentSystem<components::FPSCompanion> {
public:
    explicit FPSCompanionSystem(ecs::World* world);
    ~FPSCompanionSystem() override = default;

    std::string getName() const override { return "FPSCompanionSystem"; }

    bool startFollowing(const std::string& entity_id, const std::string& player_id);
    bool stopFollowing(const std::string& entity_id);
    bool commandHide(const std::string& entity_id);
    bool applyDamage(const std::string& entity_id, float amount);
    bool heal(const std::string& entity_id, float amount);
    bool rescue(const std::string& entity_id);
    int getState(const std::string& entity_id) const;
    float getHealth(const std::string& entity_id) const;
    float getMorale(const std::string& entity_id) const;
    bool isDead(const std::string& entity_id) const;
    bool isRescued(const std::string& entity_id) const;
    float getDistanceFollowed(const std::string& entity_id) const;
    bool setPosition(const std::string& entity_id, float x, float y, float z);

protected:
    void updateComponent(ecs::Entity& entity, components::FPSCompanion& companion, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_COMPANION_SYSTEM_H
