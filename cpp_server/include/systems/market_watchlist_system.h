#ifndef NOVAFORGE_SYSTEMS_MARKET_WATCHLIST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MARKET_WATCHLIST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Market price watchlist with buy/sell alert thresholds
 *
 * Players can add market items to their watchlist and set optional price
 * alert thresholds.  updatePrice() refreshes the cached price for an entry
 * and checks whether any threshold has been crossed; if so the alert_fired
 * flag is set and total_alerts_fired is incremented.
 *
 * Alerts are one-shot: once fired the flag stays set until
 * acknowledgeAlert() clears it, preventing duplicate notifications.
 * Removing and re-adding the same entry ID resets the alert state.
 *
 * max_entries limits the watchlist size (default 30).
 * Duplicate entry IDs and empty IDs are rejected.
 */
class MarketWatchlistSystem
    : public ecs::SingleComponentSystem<components::MarketWatchlist> {
public:
    explicit MarketWatchlistSystem(ecs::World* world);
    ~MarketWatchlistSystem() override = default;

    std::string getName() const override { return "MarketWatchlistSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Watchlist management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  const std::string& item_name,
                  float current_price = 0.0f);
    bool removeEntry(const std::string& entity_id, const std::string& entry_id);
    bool clearWatchlist(const std::string& entity_id);

    // --- Alert thresholds ---
    bool setBuyThreshold(const std::string& entity_id,
                         const std::string& entry_id,
                         float threshold);
    bool setSellThreshold(const std::string& entity_id,
                          const std::string& entry_id,
                          float threshold);
    bool clearBuyThreshold(const std::string& entity_id, const std::string& entry_id);
    bool clearSellThreshold(const std::string& entity_id, const std::string& entry_id);

    // --- Price updates ---
    bool updatePrice(const std::string& entity_id,
                     const std::string& entry_id,
                     float new_price);

    // --- Alert acknowledgement ---
    bool acknowledgeAlert(const std::string& entity_id,
                          const std::string& entry_id);
    bool acknowledgeAllAlerts(const std::string& entity_id);

    // --- Queries ---
    int   getEntryCount(const std::string& entity_id) const;
    float getCurrentPrice(const std::string& entity_id,
                          const std::string& entry_id) const;
    bool  hasBuyAlert(const std::string& entity_id,
                      const std::string& entry_id) const;
    bool  hasSellAlert(const std::string& entity_id,
                       const std::string& entry_id) const;
    int   getPendingAlertCount(const std::string& entity_id) const;
    int   getTotalAlertsFired(const std::string& entity_id) const;
    bool  hasEntry(const std::string& entity_id,
                   const std::string& entry_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MarketWatchlist& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MARKET_WATCHLIST_SYSTEM_H
