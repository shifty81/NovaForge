#ifndef NOVAFORGE_SYSTEMS_SENSOR_DAMPENING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SENSOR_DAMPENING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Remote sensor dampening system
 *
 * Manages sensor dampeners applied to an entity.  Each dampener reduces the
 * entity's targeting (lock) range and scan resolution by a configurable
 * fraction.  Multiple dampeners from different sources are applied
 * multiplicatively to the base values.  When all dampeners are removed the
 * effective values return to their base.
 */
class SensorDampeningSystem
    : public ecs::SingleComponentSystem<components::SensorDampeningState> {
public:
    explicit SensorDampeningSystem(ecs::World* world);
    ~SensorDampeningSystem() override = default;

    std::string getName() const override { return "SensorDampeningSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    float base_lock_range = 100.0f,
                    float base_scan_resolution = 400.0f);

    // --- Dampener management ---
    bool applyDampener(const std::string& entity_id,
                       const std::string& source_id,
                       float range_reduction,
                       float scan_res_reduction,
                       float cycle_time = 5.0f);
    bool removeDampener(const std::string& entity_id,
                        const std::string& source_id);
    bool clearDampeners(const std::string& entity_id);

    // --- Base value configuration ---
    bool setBaseLockRange(const std::string& entity_id, float range);
    bool setBaseScanResolution(const std::string& entity_id, float res);

    // --- Queries ---
    int   getDampenerCount(const std::string& entity_id) const;
    float getEffectiveLockRange(const std::string& entity_id) const;
    float getEffectiveScanResolution(const std::string& entity_id) const;
    float getBaseLockRange(const std::string& entity_id) const;
    float getBaseScanResolution(const std::string& entity_id) const;
    int   getTotalDampenersApplied(const std::string& entity_id) const;
    int   getTotalDampenerCycles(const std::string& entity_id) const;
    bool  isDampened(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SensorDampeningState& comp,
                         float delta_time) override;

private:
    void recalcEffectiveValues(components::SensorDampeningState& comp) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SENSOR_DAMPENING_SYSTEM_H
