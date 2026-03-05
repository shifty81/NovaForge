#ifndef NOVAFORGE_SYSTEMS_INTEREST_MANAGEMENT_PRIORITY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INTEREST_MANAGEMENT_PRIORITY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Enhanced interest management with priority-tier update rates
 *
 * Provides priority-based update frequency control for large player counts,
 * assigning bandwidth weights and update intervals based on distance tiers.
 */
class InterestManagementPrioritySystem : public ecs::SingleComponentSystem<components::InterestPriority> {
public:
    explicit InterestManagementPrioritySystem(ecs::World* world);
    ~InterestManagementPrioritySystem() override = default;

    std::string getName() const override { return "InterestManagementPrioritySystem"; }

    bool createPriority(const std::string& entity_id);
    bool setClientId(const std::string& entity_id, int client_id);
    bool setPriorityTier(const std::string& entity_id, int tier);
    bool setDistance(const std::string& entity_id, float distance);
    bool needsUpdate(const std::string& entity_id) const;
    int getPriorityTier(const std::string& entity_id) const;
    float getBandwidthWeight(const std::string& entity_id) const;
    int getTotalUpdates(const std::string& entity_id) const;
    float getEstimatedBandwidth(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::InterestPriority& ip, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INTEREST_MANAGEMENT_PRIORITY_SYSTEM_H
