#ifndef NOVAFORGE_SYSTEMS_SUPPLY_DEMAND_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SUPPLY_DEMAND_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class SupplyDemandSystem : public ecs::System {
public:
    explicit SupplyDemandSystem(ecs::World* world);
    ~SupplyDemandSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SupplyDemandSystem"; }

    // --- API ---
    float getPrice(const std::string& system_id, const std::string& commodity_id) const;
    float getSupply(const std::string& system_id, const std::string& commodity_id) const;
    float getDemand(const std::string& system_id, const std::string& commodity_id) const;
    void addSupply(const std::string& system_id, const std::string& commodity_id, float amount);
    void addDemand(const std::string& system_id, const std::string& commodity_id, float amount);
    void setNPCActivityModifier(const std::string& system_id, float modifier);
};

} // namespace systems
} // namespace atlas

#endif
