#ifndef NOVAFORGE_SYSTEMS_JUMP_GATE_ACTIVATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_JUMP_GATE_ACTIVATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles jump gate charge-up, activation, and cooldown
 *
 * Manages the lifecycle of jump gate activations: charge-up phase,
 * ship transit, and cooldown period. Tracks fuel costs, jump counts,
 * and queue management for multiple ships waiting to use a gate.
 * Integrates with navigation and economy systems.
 */
class JumpGateActivationSystem : public ecs::SingleComponentSystem<components::JumpGateState> {
public:
    explicit JumpGateActivationSystem(ecs::World* world);
    ~JumpGateActivationSystem() override = default;

    std::string getName() const override { return "JumpGateActivationSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& gate_id,
                    const std::string& dest_system, const std::string& dest_gate);
    bool startCharge(const std::string& entity_id);
    bool cancelCharge(const std::string& entity_id);
    bool queueShip(const std::string& entity_id);
    bool dequeueShip(const std::string& entity_id);
    bool setChargeTime(const std::string& entity_id, float time);
    bool setCooldownTime(const std::string& entity_id, float time);
    bool setFuelCost(const std::string& entity_id, float cost);
    float getChargeProgress(const std::string& entity_id) const;
    float getRemainingCooldown(const std::string& entity_id) const;
    float getFuelCost(const std::string& entity_id) const;
    bool isReady(const std::string& entity_id) const;
    bool isCharging(const std::string& entity_id) const;
    bool isOnCooldown(const std::string& entity_id) const;
    int getTotalJumps(const std::string& entity_id) const;
    int getCurrentQueue(const std::string& entity_id) const;
    std::string getDestinationSystem(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::JumpGateState& jgs, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_JUMP_GATE_ACTIVATION_SYSTEM_H
