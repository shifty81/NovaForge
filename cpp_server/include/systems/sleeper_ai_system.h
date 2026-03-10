#ifndef NOVAFORGE_SYSTEMS_SLEEPER_AI_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SLEEPER_AI_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Sleeper NPC artificial intelligence system
 *
 * Manages advanced Sleeper drone behaviour in wormhole space.
 * Sleepers coordinate target selection, apply remote repairs,
 * and escalate by spawning reinforcement waves when total
 * damage taken exceeds configurable thresholds.
 */
class SleeperAISystem : public ecs::SingleComponentSystem<components::SleeperAIState> {
public:
    explicit SleeperAISystem(ecs::World* world);
    ~SleeperAISystem() override = default;

    std::string getName() const override { return "SleeperAISystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& site_id = "");
    bool addUnit(const std::string& entity_id, const std::string& unit_id,
                 components::SleeperAIState::SleeperRole role =
                     components::SleeperAIState::SleeperRole::Sentry,
                 float hp = 1000.0f, float dps = 150.0f,
                 float remote_rep = 0.0f);
    bool removeUnit(const std::string& entity_id, const std::string& unit_id);
    bool applyDamage(const std::string& entity_id, const std::string& unit_id,
                     float amount);
    bool setAlertLevel(const std::string& entity_id,
                       components::SleeperAIState::AlertLevel level);

    int  getUnitCount(const std::string& entity_id) const;
    int  getAliveCount(const std::string& entity_id) const;
    int  getEscalationWave(const std::string& entity_id) const;
    float getDamageTaken(const std::string& entity_id) const;
    int  getTotalKills(const std::string& entity_id) const;
    int  getTotalLosses(const std::string& entity_id) const;
    components::SleeperAIState::AlertLevel getAlertLevel(
        const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SleeperAIState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SLEEPER_AI_SYSTEM_H
