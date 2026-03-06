#ifndef NOVAFORGE_SYSTEMS_FPS_COVER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_COVER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Cover detection and tactical positioning for FPS combat
 *
 * Manages cover points, player cover state transitions,
 * damage reduction while in cover, and destructible cover.
 */
class FPSCoverSystem : public ecs::SingleComponentSystem<components::FPSCover> {
public:
    explicit FPSCoverSystem(ecs::World* world);
    ~FPSCoverSystem() override = default;

    std::string getName() const override { return "FPSCoverSystem"; }

    bool addCoverPoint(const std::string& entity_id, const std::string& point_id,
                       float x, float y, float z, int type, float facing_angle);
    bool removeCoverPoint(const std::string& entity_id, const std::string& point_id);
    bool enterCover(const std::string& entity_id, const std::string& point_id);
    bool leaveCover(const std::string& entity_id);
    bool startPeek(const std::string& entity_id);
    bool stopPeek(const std::string& entity_id);
    int getCoverState(const std::string& entity_id) const;
    float getDamageReduction(const std::string& entity_id) const;
    bool damageCoverPoint(const std::string& entity_id, const std::string& point_id, float amount);
    int getCoverPointCount(const std::string& entity_id) const;
    int getTotalCoversUsed(const std::string& entity_id) const;
    int getTotalCoversDestroyed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FPSCover& cover, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_COVER_SYSTEM_H
