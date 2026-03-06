#ifndef NOVAFORGE_SYSTEMS_MISSION_REWARD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MISSION_REWARD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/mission_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Transfer rewards to players on mission completion
 *
 * Tracks pending rewards, prevents double-claiming, and records
 * total ISK, items, and standing earned from missions.
 */
class MissionRewardSystem : public ecs::SingleComponentSystem<components::MissionReward> {
public:
    explicit MissionRewardSystem(ecs::World* world);
    ~MissionRewardSystem() override = default;

    std::string getName() const override { return "MissionRewardSystem"; }

    bool addReward(const std::string& entity_id, const std::string& mission_id,
                   double isk_amount, const std::string& faction_id, double standing_change,
                   const std::string& item_id, int item_quantity);
    bool collectReward(const std::string& entity_id, const std::string& mission_id);
    bool isCollected(const std::string& entity_id, const std::string& mission_id) const;
    int getPendingCount(const std::string& entity_id) const;
    int getTotalCollected(const std::string& entity_id) const;
    double getTotalIskEarned(const std::string& entity_id) const;
    double getTotalStandingGained(const std::string& entity_id) const;
    double getRewardIsk(const std::string& entity_id, const std::string& mission_id) const;
    std::string getRewardItemId(const std::string& entity_id, const std::string& mission_id) const;
    int getRewardItemQuantity(const std::string& entity_id, const std::string& mission_id) const;
    bool hasReward(const std::string& entity_id, const std::string& mission_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MissionReward& reward, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MISSION_REWARD_SYSTEM_H
