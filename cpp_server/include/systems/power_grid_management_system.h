#ifndef NOVAFORGE_SYSTEMS_POWER_GRID_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_POWER_GRID_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship power grid budget management
 *
 * Tracks total powergrid output and per-module power draw.  Modules
 * can be onlined if sufficient power is available.  If the total draw
 * exceeds output (e.g. after a reactor downgrade) the system
 * auto-offlines the lowest-priority module each tick until balanced.
 */
class PowerGridManagementSystem : public ecs::SingleComponentSystem<components::PowerGridState> {
public:
    explicit PowerGridManagementSystem(ecs::World* world);
    ~PowerGridManagementSystem() override = default;

    std::string getName() const override { return "PowerGridManagementSystem"; }

public:
    bool initialize(const std::string& entity_id, float total_output);
    bool addModule(const std::string& entity_id, const std::string& module_id,
                   float power_draw, int priority);
    bool onlineModule(const std::string& entity_id, const std::string& module_id);
    bool offlineModule(const std::string& entity_id, const std::string& module_id);
    bool removeModule(const std::string& entity_id, const std::string& module_id);
    bool setTotalOutput(const std::string& entity_id, float output);

    int getModuleCount(const std::string& entity_id) const;
    int getOnlineCount(const std::string& entity_id) const;
    float getTotalDraw(const std::string& entity_id) const;
    float getTotalOutput(const std::string& entity_id) const;
    float getAvailablePower(const std::string& entity_id) const;
    bool isOverloaded(const std::string& entity_id) const;
    int getTotalOverloads(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PowerGridState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_POWER_GRID_MANAGEMENT_SYSTEM_H
