#ifndef NOVAFORGE_SYSTEMS_DAMAGE_RESISTANCE_PROFILE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DAMAGE_RESISTANCE_PROFILE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship damage resistance profiles — type-specific mitigation with stacking penalties
 *
 * Every ship can have a resistance profile that reduces incoming damage by
 * type (EM, Thermal, Kinetic, Explosive).  Each resistance is 0.0–0.85
 * (capped at 85%).  Multiple hardener modules stack with diminishing returns
 * using a stacking penalty factor.  Active hardeners cycle each tick,
 * consuming capacitor-equivalent charge.  The system tracks total damage
 * mitigated across all types for analytics.
 */
class DamageResistanceProfileSystem : public ecs::SingleComponentSystem<components::DamageResistanceProfile> {
public:
    explicit DamageResistanceProfileSystem(ecs::World* world);
    ~DamageResistanceProfileSystem() override = default;

    std::string getName() const override { return "DamageResistanceProfileSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool setBaseResistance(const std::string& entity_id, const std::string& damage_type, float value);
    bool addHardener(const std::string& entity_id, const std::string& hardener_id,
                     const std::string& damage_type, float bonus);
    bool removeHardener(const std::string& entity_id, const std::string& hardener_id);
    bool activateHardener(const std::string& entity_id, const std::string& hardener_id);
    bool deactivateHardener(const std::string& entity_id, const std::string& hardener_id);
    float applyResistance(const std::string& entity_id, const std::string& damage_type, float raw_damage);

    float getEffectiveResistance(const std::string& entity_id, const std::string& damage_type) const;
    int getHardenerCount(const std::string& entity_id) const;
    int getActiveHardenerCount(const std::string& entity_id) const;
    float getTotalDamageMitigated(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::DamageResistanceProfile& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DAMAGE_RESISTANCE_PROFILE_SYSTEM_H
