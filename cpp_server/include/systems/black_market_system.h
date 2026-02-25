#ifndef NOVAFORGE_SYSTEMS_BLACK_MARKET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BLACK_MARKET_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class BlackMarketSystem : public ecs::System {
public:
    explicit BlackMarketSystem(ecs::World* world);
    ~BlackMarketSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "BlackMarketSystem"; }

    // --- API ---
    void addListing(const std::string& market_id, const std::string& item_id,
                    const std::string& seller_id, float price, int quantity,
                    bool contraband, float risk);
    bool purchaseItem(const std::string& market_id, const std::string& item_id,
                      const std::string& buyer_id);
    int getListingCount(const std::string& market_id) const;
    float getDetectionChance(const std::string& market_id) const;
    std::vector<std::string> getAvailableItems(const std::string& market_id) const;
    void setSecurityLevel(const std::string& market_id, float security);
};

} // namespace systems
} // namespace atlas

#endif
