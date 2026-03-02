#ifndef NOVAFORGE_SYSTEMS_PVP_TOGGLE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PVP_TOGGLE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief PvP flagging system with safety zone enforcement
 *
 * Manages PvP toggle state, engagement timers, safety levels, kill tracking,
 * bounty accumulation, and security status impact from PvP kills.
 */
class PvPToggleSystem : public ecs::System {
public:
    explicit PvPToggleSystem(ecs::World* world);
    ~PvPToggleSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "PvPToggleSystem"; }

    bool createPvPState(const std::string& entity_id);
    bool enablePvP(const std::string& entity_id);
    bool disablePvP(const std::string& entity_id);
    bool setSafetyLevel(const std::string& entity_id, const std::string& level);
    bool canEngage(const std::string& attacker_id, const std::string& defender_id) const;
    bool recordEngagement(const std::string& attacker_id, const std::string& defender_id);
    bool recordKill(const std::string& entity_id);
    int getKillCount(const std::string& entity_id) const;
    float getSecurityStatus(const std::string& entity_id) const;
    float getAggressionTimer(const std::string& entity_id) const;
    bool isPvPEnabled(const std::string& entity_id) const;
    std::string getSafetyLevel(const std::string& entity_id) const;
    float getBounty(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PVP_TOGGLE_SYSTEM_H
