#ifndef EVE_SYSTEMS_FLEET_CARGO_SYSTEM_H
#define EVE_SYSTEMS_FLEET_CARGO_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <cstdint>

namespace atlas {
namespace systems {

class FleetCargoSystem : public ecs::System {
public:
    explicit FleetCargoSystem(ecs::World* world);
    ~FleetCargoSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetCargoSystem"; }

    void addContributor(const std::string& pool_entity_id, const std::string& ship_entity_id);
    void removeContributor(const std::string& pool_entity_id, const std::string& ship_entity_id);
    uint64_t getTotalCapacity(const std::string& pool_entity_id) const;
    uint64_t getUsedCapacity(const std::string& pool_entity_id) const;
    void recalculate(const std::string& pool_entity_id);

    // Phase 11: Ship loss = cargo loss
    /**
     * @brief Handle a ship being destroyed — removes its contribution and
     *        recalculates the fleet cargo pool immediately.
     * @return The cargo capacity that was lost
     */
    uint64_t handleShipLoss(const std::string& pool_entity_id, const std::string& ship_entity_id);

    // Phase 11: Capacity scaling with modifiers
    /**
     * @brief Calculate scaled capacity using logistics efficiency, captain skill,
     *        and morale modifier: Σ(shipCargo × logistics × skill × morale)
     * @param logistics_efficiency  Logistics wing efficiency multiplier (1.0 = baseline)
     * @param captain_skill         Captain's logistics skill multiplier (1.0 = baseline)
     * @param morale_modifier       Fleet morale-based modifier (1.0 = Steady, 0.75 = Doubtful)
     * @return Scaled total capacity
     */
    uint64_t getScaledCapacity(const std::string& pool_entity_id,
                                float logistics_efficiency,
                                float captain_skill,
                                float morale_modifier) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_CARGO_SYSTEM_H
