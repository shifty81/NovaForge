#ifndef EVE_SYSTEMS_MARKET_ORDER_SYSTEM_H
#define EVE_SYSTEMS_MARKET_ORDER_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class MarketOrderSystem : public ecs::System {
public:
    explicit MarketOrderSystem(ecs::World* world);
    ~MarketOrderSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MarketOrderSystem"; }

    void placeOrder(const std::string& entity_id,
                    components::MarketOrder::OrderType order_type,
                    const std::string& item, int qty, float price,
                    const std::string& region, const std::string& station,
                    const std::string& owner);
    bool cancelOrder(const std::string& entity_id);
    int fillOrder(const std::string& entity_id, int fill_qty);
    std::vector<std::string> getOrdersForRegion(const std::string& region_id) const;
    std::string dispatchAIFleet(const std::string& order_entity_id,
                                components::AIFleetDispatch::DispatchType dispatch_type,
                                const std::string& target_system, int fleet_size);
    std::vector<std::string> getActiveDispatches() const;
    bool isOrderExpired(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MARKET_ORDER_SYSTEM_H
