#ifndef NOVAFORGE_SYSTEMS_COMBAT_LOG_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMBAT_LOG_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Combat event recording and engagement analytics system
 *
 * Records combat events for damage type analysis, DPS calculation,
 * and engagement outcome tracking. Provides per-engagement statistics
 * and damage breakdowns for content balance tuning.
 */
class CombatLogSystem : public ecs::SingleComponentSystem<components::CombatLog> {
public:
    explicit CombatLogSystem(ecs::World* world);
    ~CombatLogSystem() override = default;

    std::string getName() const override { return "CombatLogSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool logDamage(const std::string& entity_id, const std::string& attacker,
                   const std::string& defender, int damage_type,
                   float amount, const std::string& weapon, bool hit);
    bool startEngagement(const std::string& entity_id, const std::string& engagement_id);
    bool endEngagement(const std::string& entity_id, const std::string& engagement_id,
                       int outcome);
    bool recordKill(const std::string& entity_id, const std::string& engagement_id);
    bool recordLoss(const std::string& entity_id, const std::string& engagement_id);
    int getEntryCount(const std::string& entity_id) const;
    int getEngagementCount(const std::string& entity_id) const;
    float getTotalDamageDealt(const std::string& entity_id) const;
    float getTotalDamageReceived(const std::string& entity_id) const;
    float getAverageDPS(const std::string& entity_id, const std::string& engagement_id) const;
    int getTotalKills(const std::string& entity_id) const;
    int getTotalLosses(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CombatLog& cl, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMBAT_LOG_SYSTEM_H
