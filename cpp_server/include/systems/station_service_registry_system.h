#ifndef NOVAFORGE_SYSTEMS_STATION_SERVICE_REGISTRY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STATION_SERVICE_REGISTRY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Station service registry system
 *
 * Manages station service availability, pricing, cooldowns, and usage
 * tracking for docked players.  Ties together the docking, trading,
 * and repair loops in the vertical slice.
 */
class StationServiceRegistrySystem : public ecs::SingleComponentSystem<components::StationServiceRegistry> {
public:
    explicit StationServiceRegistrySystem(ecs::World* world);
    ~StationServiceRegistrySystem() override = default;

    std::string getName() const override { return "StationServiceRegistrySystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool registerService(const std::string& entity_id, const std::string& service_id,
                         const std::string& name,
                         components::StationServiceRegistry::ServiceCategory category,
                         float cost, float cooldown);
    bool removeService(const std::string& entity_id, const std::string& service_id);
    bool useService(const std::string& entity_id, const std::string& service_id);
    bool setAvailability(const std::string& entity_id, const std::string& service_id, bool available);
    int getServiceCount(const std::string& entity_id) const;
    int getAvailableCount(const std::string& entity_id) const;
    float getServiceCost(const std::string& entity_id, const std::string& service_id) const;
    int getTimesUsed(const std::string& entity_id, const std::string& service_id) const;
    bool isServiceAvailable(const std::string& entity_id, const std::string& service_id) const;
    bool isOnCooldown(const std::string& entity_id, const std::string& service_id) const;
    int getTotalUses(const std::string& entity_id) const;
    float getTotalRevenue(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::StationServiceRegistry& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STATION_SERVICE_REGISTRY_SYSTEM_H
