#ifndef NOVAFORGE_SYSTEMS_CAPACITOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPACITOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/core_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles capacitor recharge for all entities
 * 
 * Implements Astralis-style capacitor recharge.
 * Capacitor is the energy resource used for module activation (weapons, EWAR, etc.).
 */
class CapacitorSystem : public ecs::SingleComponentSystem<components::Capacitor> {
public:
    explicit CapacitorSystem(ecs::World* world);
    ~CapacitorSystem() override = default;
    
    std::string getName() const override { return "CapacitorSystem"; }
    
    /**
     * @brief Consume capacitor from an entity
     * @param entity_id Entity to drain capacitor from
     * @param amount Amount of capacitor to consume (GJ)
     * @return true if enough capacitor was available and consumed
     */
    bool consumeCapacitor(const std::string& entity_id, float amount);
    
    /**
     * @brief Get current capacitor percentage for an entity
     * @param entity_id Entity to query
     * @return Capacitor percentage (0.0 - 1.0), or -1.0 if entity not found
     */
    float getCapacitorPercentage(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::Capacitor& cap, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPACITOR_SYSTEM_H
