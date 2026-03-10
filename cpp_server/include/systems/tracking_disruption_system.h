#ifndef NOVAFORGE_SYSTEMS_TRACKING_DISRUPTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TRACKING_DISRUPTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Turret tracking disruption and missile guidance disruption system
 *
 * Tracking disruptors reduce turret tracking speed and optimal range.
 * Guidance disruptors increase missile explosion radius and reduce explosion
 * velocity.  Both module types are tracked per source; effective values are
 * recomputed whenever the disruptor list changes.  Reduction fractions
 * accumulate additively and are clamped so effective values never go below
 * a small positive floor.
 */
class TrackingDisruptionSystem
    : public ecs::SingleComponentSystem<components::TrackingDisruptionState> {
public:
    explicit TrackingDisruptionSystem(ecs::World* world);
    ~TrackingDisruptionSystem() override = default;

    std::string getName() const override { return "TrackingDisruptionSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    float base_tracking_speed = 1.0f,
                    float base_optimal_range  = 50.0f,
                    float base_explosion_radius   = 40.0f,
                    float base_explosion_velocity = 200.0f);

    // --- Tracking disruptor management ---
    bool applyTrackingDisruptor(const std::string& entity_id,
                                const std::string& source_id,
                                float tracking_speed_reduction,
                                float optimal_range_reduction,
                                float cycle_time = 5.0f);
    bool removeTrackingDisruptor(const std::string& entity_id,
                                 const std::string& source_id);
    bool clearTrackingDisruptors(const std::string& entity_id);

    // --- Guidance disruptor management ---
    bool applyGuidanceDisruptor(const std::string& entity_id,
                                const std::string& source_id,
                                float explosion_radius_increase,
                                float explosion_velocity_reduction,
                                float cycle_time = 5.0f);
    bool removeGuidanceDisruptor(const std::string& entity_id,
                                 const std::string& source_id);
    bool clearGuidanceDisruptors(const std::string& entity_id);

    // --- Queries (tracking) ---
    float getEffectiveTrackingSpeed(const std::string& entity_id) const;
    float getEffectiveOptimalRange(const std::string& entity_id) const;
    int   getTrackingDisruptorCount(const std::string& entity_id) const;

    // --- Queries (guidance) ---
    float getEffectiveExplosionRadius(const std::string& entity_id) const;
    float getEffectiveExplosionVelocity(const std::string& entity_id) const;
    int   getGuidanceDisruptorCount(const std::string& entity_id) const;

    // --- Aggregate queries ---
    int   getTotalTrackingDisruptorsApplied(const std::string& entity_id) const;
    int   getTotalGuidanceDisruptorsApplied(const std::string& entity_id) const;
    bool  isDisrupted(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::TrackingDisruptionState& comp,
                         float delta_time) override;

private:
    void recalcEffectiveValues(components::TrackingDisruptionState& comp) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TRACKING_DISRUPTION_SYSTEM_H
