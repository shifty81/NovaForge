#ifndef NOVAFORGE_SYSTEMS_MARKET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MARKET_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class MarketSystem : public ecs::SingleComponentSystem<components::MarketHub> {
public:
    explicit MarketSystem(ecs::World* world);
    ~MarketSystem() override = default;

    std::string getName() const override { return "MarketSystem"; }

    /**
     * @brief Place a sell order at a station
     * @return order_id or empty string on failure
     */
    std::string placeSellOrder(const std::string& station_id,
                                const std::string& seller_id,
                                const std::string& item_id,
                                const std::string& item_name,
                                int quantity,
                                double price_per_unit);

    /**
     * @brief Place a buy order at a station
     * @return order_id or empty string on failure
     */
    std::string placeBuyOrder(const std::string& station_id,
                               const std::string& buyer_id,
                               const std::string& item_id,
                               const std::string& item_name,
                               int quantity,
                               double price_per_unit);

    /**
     * @brief Buy directly from the lowest-priced sell order
     * @return quantity actually bought
     */
    int buyFromMarket(const std::string& station_id,
                      const std::string& buyer_id,
                      const std::string& item_id,
                      int quantity);

    /**
     * @brief Get lowest sell price for an item at a station
     * @return lowest price, or -1 if none available
     */
    double getLowestSellPrice(const std::string& station_id,
                               const std::string& item_id);

    /**
     * @brief Get highest buy price for an item at a station
     * @return highest price, or -1 if none available
     */
    double getHighestBuyPrice(const std::string& station_id,
                                const std::string& item_id);

    /**
     * @brief Get number of active orders at a station
     */
    int getOrderCount(const std::string& station_id);

    /**
     * @brief Seed a station's market with NPC sell orders for common minerals
     *
     * Creates permanent sell orders for Stellium, Vanthium, Cydrium, and
     * Nocxidium at baseline prices so players can always buy basic materials.
     *
     * @param station_id Entity id of the station with a MarketHub component
     * @return number of NPC orders created
     */
    int seedNPCOrders(const std::string& station_id);

protected:
    void updateComponent(ecs::Entity& entity, components::MarketHub& hub, float delta_time) override;

private:
    int order_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MARKET_SYSTEM_H
