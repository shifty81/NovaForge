#ifndef NOVAFORGE_SYSTEMS_TRADE_WINDOW_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TRADE_WINDOW_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player-to-player trade window system
 *
 * Models the EVE Online two-step trade confirmation flow.  The owner
 * opens a trade with a partner, each side adds their offers, and both
 * independently confirm.  completeTrade() succeeds only when both sides
 * have confirmed.  Either side can cancel before completion.
 */
class TradeWindowSystem
    : public ecs::SingleComponentSystem<components::TradeWindow> {
public:
    explicit TradeWindowSystem(ecs::World* world);
    ~TradeWindowSystem() override = default;

    std::string getName() const override { return "TradeWindowSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& owner_id);

    // --- Trade control ---
    bool openTrade(const std::string& entity_id,
                   const std::string& partner_id);
    bool cancelTrade(const std::string& entity_id);
    bool confirmTrade(const std::string& entity_id);
    bool setPartnerConfirmed(const std::string& entity_id, bool confirmed);
    bool completeTrade(const std::string& entity_id);

    // --- Offer management ---
    bool addOffer(const std::string& entity_id,
                  const std::string& item_id,
                  const std::string& item_name,
                  int quantity,
                  float unit_value);
    bool removeOffer(const std::string& entity_id,
                     const std::string& item_id);

    // --- Queries ---
    components::TradeWindow::TradeState
        getState(const std::string& entity_id) const;
    bool isTradeOpen(const std::string& entity_id) const;
    int  getOfferCount(const std::string& entity_id) const;
    float getTotalOfferValue(const std::string& entity_id) const;
    int  getTotalTrades(const std::string& entity_id) const;
    std::string getPartnerId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::TradeWindow& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TRADE_WINDOW_SYSTEM_H
