#ifndef NOVAFORGE_SYSTEMS_JUMP_GATE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_JUMP_GATE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Jump gate network for inter-system travel
 *
 * Manages jump gate entities that connect star systems. Handles gate
 * activation sequences with progress tracking, fuel cost validation,
 * cooldown enforcement, security level checks, and jump statistics.
 */
class JumpGateSystem : public ecs::System {
public:
    explicit JumpGateSystem(ecs::World* world);
    ~JumpGateSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "JumpGateSystem"; }

    bool initializeGateNetwork(const std::string& entity_id, const std::string& system_id);
    bool addGate(const std::string& entity_id, const std::string& gate_id,
                 const std::string& dest_system, const std::string& dest_gate_id,
                 float fuel_cost, float security_level);
    bool removeGate(const std::string& entity_id, const std::string& gate_id);
    bool activateGate(const std::string& entity_id, const std::string& gate_id);
    bool completeJump(const std::string& entity_id, const std::string& gate_id);
    bool setGateOnline(const std::string& entity_id, const std::string& gate_id, bool online);
    float getActivationProgress(const std::string& entity_id, const std::string& gate_id) const;
    float getCooldownRemaining(const std::string& entity_id, const std::string& gate_id) const;
    int getGateCount(const std::string& entity_id) const;
    int getOnlineGateCount(const std::string& entity_id) const;
    int getTotalJumps(const std::string& entity_id) const;
    bool isGateReady(const std::string& entity_id, const std::string& gate_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_JUMP_GATE_SYSTEM_H
