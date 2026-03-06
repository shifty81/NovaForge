#ifndef NOVAFORGE_SYSTEMS_WARP_DISRUPTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_DISRUPTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages warp disruption/scramble effects on entities
 *
 * Tracks active warp disruptors/scramblers applied to each entity.
 * When total disruption strength exceeds warp core strength, warp is blocked.
 * Supports disruptor (1 point) and scrambler (2 points) modules.
 */
class WarpDisruptionSystem : public ecs::SingleComponentSystem<components::WarpDisruption> {
public:
    explicit WarpDisruptionSystem(ecs::World* world);
    ~WarpDisruptionSystem() override = default;

    std::string getName() const override { return "WarpDisruptionSystem"; }

    bool initializeDisruption(const std::string& entity_id, int warp_core_strength);
    bool applyDisruptor(const std::string& entity_id, const std::string& source_id,
                        int strength, float range);
    bool removeDisruptor(const std::string& entity_id, const std::string& source_id);
    int getDisruptorCount(const std::string& entity_id) const;
    int getTotalStrength(const std::string& entity_id) const;
    bool isWarpBlocked(const std::string& entity_id) const;
    int getWarpCoreStrength(const std::string& entity_id) const;
    int getTotalDisruptionsApplied(const std::string& entity_id) const;
    int getTotalEscapes(const std::string& entity_id) const;
    bool clearAllDisruptors(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::WarpDisruption& wd, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_DISRUPTION_SYSTEM_H
