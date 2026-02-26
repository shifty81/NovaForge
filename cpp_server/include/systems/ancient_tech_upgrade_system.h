#ifndef NOVAFORGE_SYSTEMS_ANCIENT_TECH_UPGRADE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ANCIENT_TECH_UPGRADE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Upgrades repaired ancient tech modules to exceed modern module limits
 *
 * Once ancient tech is repaired, players can invest resources to upgrade it
 * to the "Upgraded" state. Upgraded ancient tech provides stat bonuses that
 * exceed the normal caps of modern modules (rule-breaking).
 */
class AncientTechUpgradeSystem : public ecs::System {
public:
    explicit AncientTechUpgradeSystem(ecs::World* world);
    ~AncientTechUpgradeSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AncientTechUpgradeSystem"; }

    /**
     * @brief Begin upgrading a repaired ancient tech module
     * @return true if upgrade started successfully
     */
    bool startUpgrade(const std::string& entity_id);

    /**
     * @brief Get upgrade progress (0.0 to 1.0)
     */
    float getUpgradeProgress(const std::string& entity_id) const;

    /**
     * @brief Check if a module has rule-breaking bonuses active
     */
    bool hasRuleBreakingBonuses(const std::string& entity_id) const;

    /**
     * @brief Get the stat multiplier for an upgraded ancient module
     * Modern modules cap at 1.0, ancient upgraded can go up to power_multiplier (1.5+)
     */
    float getStatMultiplier(const std::string& entity_id) const;

    /**
     * @brief Get the number of modules currently being upgraded
     */
    int getUpgradingCount() const;

    /**
     * @brief Get the number of fully upgraded modules
     */
    int getUpgradedCount() const;

    /**
     * @brief Get the bonus description for an upgraded module
     */
    std::string getBonusDescription(const std::string& entity_id) const;

    /**
     * @brief Cancel an in-progress upgrade
     * @return true if cancellation succeeded
     */
    bool cancelUpgrade(const std::string& entity_id);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ANCIENT_TECH_UPGRADE_SYSTEM_H
