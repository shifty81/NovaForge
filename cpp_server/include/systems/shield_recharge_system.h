#ifndef NOVAFORGE_SYSTEMS_SHIELD_RECHARGE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIELD_RECHARGE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/core_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles shield recharge for all entities
 * 
 * Implements Astralis-style passive shield recharge.
 * Shields regenerate over time based on shield_recharge_rate.
 */
class ShieldRechargeSystem : public ecs::SingleComponentSystem<components::Health> {
public:
    explicit ShieldRechargeSystem(ecs::World* world);
    ~ShieldRechargeSystem() override = default;
    
    std::string getName() const override { return "ShieldRechargeSystem"; }
    
    /**
     * @brief Get current shield percentage for an entity
     * @param entity_id Entity to query
     * @return Shield percentage (0.0 - 1.0), or -1.0 if entity not found
     */
    float getShieldPercentage(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::Health& health, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIELD_RECHARGE_SYSTEM_H
