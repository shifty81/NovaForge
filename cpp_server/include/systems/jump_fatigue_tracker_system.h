#ifndef NOVAFORGE_SYSTEMS_JUMP_FATIGUE_TRACKER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_JUMP_FATIGUE_TRACKER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Jump fatigue tracking for capital ship jumps
 *
 * Manages jump fatigue timers that accumulate from rapid capital
 * ship jumping. Tracks blue (short) and orange (long) fatigue
 * timers, decay rates, and jump restriction states.
 */
class JumpFatigueTrackerSystem : public ecs::SingleComponentSystem<components::JumpFatigueTrackerState> {
public:
    explicit JumpFatigueTrackerSystem(ecs::World* world);
    ~JumpFatigueTrackerSystem() override = default;

    std::string getName() const override { return "JumpFatigueTrackerSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id);

    // Operations
    bool recordJump(const std::string& entity_id, float distance);
    bool setFatigueMultiplier(const std::string& entity_id, float multiplier);
    bool setDecayRate(const std::string& entity_id, float rate);
    bool resetTimers(const std::string& entity_id);

    // Queries
    float getBlueTimer(const std::string& entity_id) const;
    float getOrangeTimer(const std::string& entity_id) const;
    bool isJumpRestricted(const std::string& entity_id) const;
    float getFatigueMultiplier(const std::string& entity_id) const;
    float getDecayRate(const std::string& entity_id) const;
    int getTotalJumps(const std::string& entity_id) const;
    int getTotalFatiguePenalties(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::JumpFatigueTrackerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_JUMP_FATIGUE_TRACKER_SYSTEM_H
