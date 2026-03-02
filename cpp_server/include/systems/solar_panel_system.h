#ifndef NOVAFORGE_SYSTEMS_SOLAR_PANEL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SOLAR_PANEL_SYSTEM_H

#include "ecs/system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Solar panel energy generation system (Phase 14)
 *
 * Manages solar energy generation with day/night cycle, panel degradation,
 * maintenance, and energy storage in batteries.
 */
class SolarPanelSystem : public ecs::System {
public:
    explicit SolarPanelSystem(ecs::World* world);
    ~SolarPanelSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SolarPanelSystem"; }

    // Initialization
    bool initializePanels(const std::string& entity_id, const std::string& owner_id, int panel_count);
    bool removePanels(const std::string& entity_id);

    // Panel operations
    bool deployPanels(const std::string& entity_id);
    bool retractPanels(const std::string& entity_id);
    bool addPanel(const std::string& entity_id);
    bool removePanel(const std::string& entity_id);

    // Day cycle
    bool setDayCyclePosition(const std::string& entity_id, float position);

    // Maintenance
    bool performMaintenance(const std::string& entity_id, float amount);

    // Query
    float getEnergyOutput(const std::string& entity_id) const;
    float getEnergyStored(const std::string& entity_id) const;
    int getPanelCount(const std::string& entity_id) const;
    float getEfficiency(const std::string& entity_id) const;
    bool isDeployed(const std::string& entity_id) const;
    bool isDaytime(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SOLAR_PANEL_SYSTEM_H
