#ifndef NOVAFORGE_SYSTEMS_REGIONAL_MARKET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_REGIONAL_MARKET_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Regional market price tracking system
 *
 * Tracks average prices and trade volumes across market regions.
 * Each region maintains rolling price history driven by local
 * supply/demand.  Trade-hub regions have lower price multipliers
 * while remote regions cost more due to hauling logistics.
 */
class RegionalMarketSystem : public ecs::SingleComponentSystem<components::RegionalMarketState> {
public:
    explicit RegionalMarketSystem(ecs::World* world);
    ~RegionalMarketSystem() override = default;

    std::string getName() const override { return "RegionalMarketSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    bool addRegionPrice(const std::string& entity_id,
                        const std::string& region_id,
                        const std::string& item_type,
                        float average_price, float supply, float demand,
                        float hub_multiplier = 1.0f);
    bool recordTrade(const std::string& entity_id,
                     const std::string& region_id,
                     const std::string& item_type,
                     int volume, float price);
    bool setVolatilityFactor(const std::string& entity_id, float factor);

    int  getTrackedItemCount(const std::string& entity_id) const;
    int  getTotalUpdates(const std::string& entity_id) const;
    float getAveragePrice(const std::string& entity_id,
                          const std::string& region_id,
                          const std::string& item_type) const;
    float getHubMultiplier(const std::string& entity_id,
                           const std::string& region_id,
                           const std::string& item_type) const;
    int  getTradeVolume(const std::string& entity_id,
                        const std::string& region_id,
                        const std::string& item_type) const;
    float getVolatilityFactor(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RegionalMarketState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_REGIONAL_MARKET_SYSTEM_H
