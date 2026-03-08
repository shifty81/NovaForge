#ifndef NOVAFORGE_SYSTEMS_COLONY_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COLONY_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages planetary colonies: buildings, production, storage, exports
 *
 * Handles the lifecycle of planetary colonies including building placement,
 * power management, production cycles, goods storage, and export to market.
 * Integrates with the economy system for trade and pricing.
 */
class ColonyManagementSystem : public ecs::SingleComponentSystem<components::ColonyManagementState> {
public:
    explicit ColonyManagementSystem(ecs::World* world);
    ~ColonyManagementSystem() override = default;

    std::string getName() const override { return "ColonyManagementSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& colony_name,
                    const std::string& planet_id);
    bool addBuilding(const std::string& entity_id, const std::string& building_id,
                     const std::string& building_type, float production_rate, float power_usage);
    bool removeBuilding(const std::string& entity_id, const std::string& building_id);
    bool toggleBuilding(const std::string& entity_id, const std::string& building_id);
    bool addGoods(const std::string& entity_id, const std::string& good_type,
                  float quantity, float max_quantity);
    bool exportGoods(const std::string& entity_id, const std::string& good_type,
                     float quantity, float unit_price);
    bool setPowerCapacity(const std::string& entity_id, float capacity);
    int getBuildingCount(const std::string& entity_id) const;
    int getOnlineBuildingCount(const std::string& entity_id) const;
    float getPowerUsed(const std::string& entity_id) const;
    float getRemainingPower(const std::string& entity_id) const;
    float getGoodsQuantity(const std::string& entity_id, const std::string& good_type) const;
    float getTotalExports(const std::string& entity_id) const;
    float getTotalExportValue(const std::string& entity_id) const;
    int getTotalProductionCycles(const std::string& entity_id) const;
    std::string getColonyName(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ColonyManagementState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COLONY_MANAGEMENT_SYSTEM_H
