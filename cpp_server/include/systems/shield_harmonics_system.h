#ifndef NOVAFORGE_SYSTEMS_SHIELD_HARMONICS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIELD_HARMONICS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Shield frequency tuning and resonance-based damage resistance
 *
 * Manages shield harmonics — players can tune their shield frequency
 * toward a specific damage type to increase resistance at the cost of
 * vulnerability to other types.  Tracks resonance strength (how well
 * the shield is currently tuned) and per-type resistance profiles.
 * Essential for the combat depth in the vertical slice.
 */
class ShieldHarmonicsSystem : public ecs::SingleComponentSystem<components::ShieldHarmonicsState> {
public:
    explicit ShieldHarmonicsSystem(ecs::World* world);
    ~ShieldHarmonicsSystem() override = default;

    std::string getName() const override { return "ShieldHarmonicsSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, float initial_frequency);

    // Harmonic profiles
    bool addProfile(const std::string& entity_id, const std::string& damage_type,
                    float base_resistance);
    bool removeProfile(const std::string& entity_id, const std::string& damage_type);

    // Tuning
    bool tuneFrequency(const std::string& entity_id, float target_frequency);
    bool setTuningSpeed(const std::string& entity_id, float speed);
    bool setMaxBonus(const std::string& entity_id, float bonus);

    // Queries
    float getFrequency(const std::string& entity_id) const;
    float getResonanceStrength(const std::string& entity_id) const;
    float getEffectiveResistance(const std::string& entity_id,
                                  const std::string& damage_type) const;
    int getProfileCount(const std::string& entity_id) const;
    float getTuningSpeed(const std::string& entity_id) const;
    float getMaxBonus(const std::string& entity_id) const;
    int getTotalRetunings(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShieldHarmonicsState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIELD_HARMONICS_SYSTEM_H
