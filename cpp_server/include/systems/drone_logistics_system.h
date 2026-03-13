#ifndef NOVAFORGE_SYSTEMS_DRONE_LOGISTICS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DRONE_LOGISTICS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Automated drone-based item transfer queue
 *
 * Players queue transfer requests between ports.  When fleet-deploy mode
 * is enabled, the system automatically processes one pending transfer per
 * tick (up to max_drones active simultaneously).
 */
class DroneLogisticsSystem
    : public ecs::SingleComponentSystem<components::DroneLogisticsState> {
public:
    explicit DroneLogisticsSystem(ecs::World* world);
    ~DroneLogisticsSystem() override = default;

    std::string getName() const override { return "DroneLogisticsSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Transfer management ---
    bool queue_transfer(const std::string& entity_id,
                        const std::string& request_id,
                        const std::string& source_port,
                        const std::string& dest_port,
                        const std::string& item_type,
                        int amount);
    bool complete_transfer(const std::string& entity_id,
                           const std::string& request_id);
    bool cancel_transfer(const std::string& entity_id,
                         const std::string& request_id);
    bool clear_requests(const std::string& entity_id);

    // --- Configuration ---
    bool set_fleet_deploy_mode(const std::string& entity_id, bool enabled);
    bool set_max_drones(const std::string& entity_id, int count);

    // --- Queries ---
    int   get_request_count(const std::string& entity_id) const;
    int   get_pending_count(const std::string& entity_id) const;
    bool  is_fleet_deploy_mode(const std::string& entity_id) const;
    int   get_total_transfers_completed(const std::string& entity_id) const;
    int   get_max_drones(const std::string& entity_id) const;
    int   get_active_drones(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DroneLogisticsState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DRONE_LOGISTICS_SYSTEM_H
