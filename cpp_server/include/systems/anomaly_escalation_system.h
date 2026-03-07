#ifndef NOVAFORGE_SYSTEMS_ANOMALY_ESCALATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ANOMALY_ESCALATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Combat anomaly escalation system for PvE content progression
 *
 * When a player clears a combat anomaly site, the system evaluates
 * whether an escalation triggers (based on escalation_chance).  If
 * triggered, a harder follow-up site spawns after a configurable delay
 * with increased NPC count, difficulty, and rewards.  Supports up to
 * max_tiers of consecutive escalation.
 */
class AnomalyEscalationSystem : public ecs::SingleComponentSystem<components::AnomalyEscalation> {
public:
    explicit AnomalyEscalationSystem(ecs::World* world);
    ~AnomalyEscalationSystem() override = default;

    std::string getName() const override { return "AnomalyEscalationSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& system_id,
                    const std::string& owner_id);
    bool addTier(const std::string& entity_id, int tier, const std::string& site_type,
                 float difficulty_multiplier, float reward_multiplier, int npc_count);
    bool startSite(const std::string& entity_id);
    bool clearSite(const std::string& entity_id, float roll);
    bool completeEscalation(const std::string& entity_id);
    bool failEscalation(const std::string& entity_id);
    int getTierCount(const std::string& entity_id) const;
    int getCurrentTier(const std::string& entity_id) const;
    std::string getState(const std::string& entity_id) const;
    float getDifficultyMultiplier(const std::string& entity_id) const;
    float getRewardMultiplier(const std::string& entity_id) const;
    int getTotalSitesCleared(const std::string& entity_id) const;
    int getTotalEscalationsTriggered(const std::string& entity_id) const;
    int getTotalEscalationsCompleted(const std::string& entity_id) const;
    int getTotalEscalationsFailed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AnomalyEscalation& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ANOMALY_ESCALATION_SYSTEM_H
