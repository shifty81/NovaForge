#ifndef NOVAFORGE_SYSTEMS_WARP_BUBBLE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_BUBBLE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Area-of-effect warp disruption bubble system
 *
 * Manages interdictor-deployed warp disruption bubbles in null-sec
 * space.  Bubbles prevent warping for any ship inside the sphere
 * regardless of warp core strength.  Each bubble has a limited
 * lifetime and catches ships that enter or are within its radius.
 */
class WarpBubbleSystem : public ecs::SingleComponentSystem<components::WarpBubbleState> {
public:
    explicit WarpBubbleSystem(ecs::World* world);
    ~WarpBubbleSystem() override = default;

    std::string getName() const override { return "WarpBubbleSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& system_id = "");
    bool deployBubble(const std::string& entity_id, const std::string& deployer_id,
                      float radius = 20000.0f, float lifetime = 120.0f,
                      float x = 0.0f, float y = 0.0f, float z = 0.0f);
    bool removeBubble(const std::string& entity_id, const std::string& bubble_id);
    bool catchShip(const std::string& entity_id, const std::string& bubble_id);

    int  getBubbleCount(const std::string& entity_id) const;
    int  getActiveBubbleCount(const std::string& entity_id) const;
    int  getTotalDeployed(const std::string& entity_id) const;
    int  getTotalShipsCaught(const std::string& entity_id) const;
    float getBubbleRemaining(const std::string& entity_id,
                             const std::string& bubble_id) const;
    bool isBubbleExpired(const std::string& entity_id,
                         const std::string& bubble_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::WarpBubbleState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_BUBBLE_SYSTEM_H
