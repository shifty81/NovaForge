#ifndef NOVAFORGE_SYSTEMS_DAMAGE_APPLICATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DAMAGE_APPLICATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Applies queued damage to entity HP pools with resistance calculations
 *
 * Processes pending damage entries each tick, applying EM/Thermal/Kinetic/Explosive
 * damage through shield → armor → hull layers with per-layer resistance modifiers.
 * Damage overflow from a depleted layer carries into the next layer.
 */
class DamageApplicationSystem : public ecs::SingleComponentSystem<components::DamageApplication> {
public:
    explicit DamageApplicationSystem(ecs::World* world);
    ~DamageApplicationSystem() override = default;

    std::string getName() const override { return "DamageApplicationSystem"; }

    bool initializeDamageTracking(const std::string& entity_id);
    bool queueDamage(const std::string& entity_id, const std::string& source_id,
                     float raw_amount, int damage_type, float timestamp);
    int getPendingCount(const std::string& entity_id) const;
    float getTotalApplied(const std::string& entity_id) const;
    float getTotalMitigated(const std::string& entity_id) const;
    int getHitsProcessed(const std::string& entity_id) const;
    bool hasPendingDamage(const std::string& entity_id) const;
    bool clearPending(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::DamageApplication& da, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DAMAGE_APPLICATION_SYSTEM_H
