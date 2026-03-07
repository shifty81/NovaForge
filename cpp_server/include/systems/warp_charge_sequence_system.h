#ifndef NOVAFORGE_SYSTEMS_WARP_CHARGE_SEQUENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_CHARGE_SEQUENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Warp drive charge-up sequence — alignment, mass-based charge time, disruption
 *
 * When a ship initiates warp, the drive enters a charge sequence gated by
 * alignment to the destination.  Charge time scales with ship mass
 * (base_charge_time × mass_factor).  The sequence can be disrupted by
 * incoming warp disruption effects, resetting progress.  After a
 * successful warp, the drive enters a cooldown period before it can be
 * used again.  Tracks total warps completed and disruptions received.
 */
class WarpChargeSequenceSystem : public ecs::SingleComponentSystem<components::WarpChargeState> {
public:
    explicit WarpChargeSequenceSystem(ecs::World* world);
    ~WarpChargeSequenceSystem() override = default;

    std::string getName() const override { return "WarpChargeSequenceSystem"; }

public:
    bool initialize(const std::string& entity_id, float base_charge_time, float mass_factor);
    bool initiateWarp(const std::string& entity_id, const std::string& destination_id);
    bool disruptWarp(const std::string& entity_id);
    bool completeWarp(const std::string& entity_id);
    bool setAligned(const std::string& entity_id, bool aligned);

    float getChargeProgress(const std::string& entity_id) const;
    float getCooldownRemaining(const std::string& entity_id) const;
    std::string getDestination(const std::string& entity_id) const;
    int getTotalWarpsCompleted(const std::string& entity_id) const;
    int getTotalDisruptions(const std::string& entity_id) const;
    bool isCharging(const std::string& entity_id) const;
    bool isOnCooldown(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::WarpChargeState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_CHARGE_SEQUENCE_SYSTEM_H
