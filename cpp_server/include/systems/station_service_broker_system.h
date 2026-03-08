#ifndef NOVAFORGE_SYSTEMS_STATION_SERVICE_BROKER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STATION_SERVICE_BROKER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages station service availability and pricing
 *
 * Handles docking service registration, dynamic pricing based on
 * demand and standings, and tracks service revenue. Integrates with
 * the economy and reputation systems to provide contextual costs
 * for repair, refit, refuel, cloning, and market access.
 */
class StationServiceBrokerSystem : public ecs::SingleComponentSystem<components::StationServiceState> {
public:
    explicit StationServiceBrokerSystem(ecs::World* world);
    ~StationServiceBrokerSystem() override = default;

    std::string getName() const override { return "StationServiceBrokerSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addService(const std::string& entity_id, const std::string& service_id, float base_cost);
    bool removeService(const std::string& entity_id, const std::string& service_id);
    bool setServiceAvailable(const std::string& entity_id, const std::string& service_id, bool available);
    bool setDemandModifier(const std::string& entity_id, const std::string& service_id, float modifier);
    bool setTaxRate(const std::string& entity_id, float rate);
    bool setStandingDiscount(const std::string& entity_id, float discount);
    bool processTransaction(const std::string& entity_id, const std::string& service_id);
    float getServiceCost(const std::string& entity_id, const std::string& service_id) const;
    bool isServiceAvailable(const std::string& entity_id, const std::string& service_id) const;
    float getTaxRate(const std::string& entity_id) const;
    float getStandingDiscount(const std::string& entity_id) const;
    float getTotalRevenue(const std::string& entity_id) const;
    int getTotalTransactions(const std::string& entity_id) const;
    int getServiceCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::StationServiceState& sss, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STATION_SERVICE_BROKER_SYSTEM_H
