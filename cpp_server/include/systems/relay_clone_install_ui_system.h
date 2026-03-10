#ifndef NOVAFORGE_SYSTEMS_RELAY_CLONE_INSTALL_UI_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RELAY_CLONE_INSTALL_UI_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Relay clone installation UI flow system
 *
 * Drives the multi-step "Install Relay Clone" dialog that a player
 * interacts with while docked.  Steps:
 *   Idle → SelectStation → ConfirmCost → Pending → Success / Error
 * Manages the available-station list, search filtering, cost display,
 * and pending-request timeout.
 */
class RelayCloneInstallUiSystem
    : public ecs::SingleComponentSystem<components::RelayCloneInstallUiState> {
public:
    explicit RelayCloneInstallUiSystem(ecs::World* world);
    ~RelayCloneInstallUiSystem() override = default;

    std::string getName() const override { return "RelayCloneInstallUiSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& character_id = "");

    // --- Panel open / close ---
    bool openPanel(const std::string& entity_id);
    bool closePanel(const std::string& entity_id);
    bool isPanelOpen(const std::string& entity_id) const;

    // --- Station list ---
    bool addStation(const std::string& entity_id,
                    const std::string& station_id,
                    const std::string& station_name,
                    const std::string& region,
                    float install_cost);
    bool removeStation(const std::string& entity_id,
                       const std::string& station_id);
    int  getAvailableStationCount(const std::string& entity_id) const;

    // --- Search / filter ---
    bool setSearchFilter(const std::string& entity_id,
                         const std::string& filter);
    std::string getSearchFilter(const std::string& entity_id) const;
    int  getFilteredStationCount(const std::string& entity_id) const;

    // --- UI step transitions ---
    bool selectStation(const std::string& entity_id,
                       const std::string& station_id);
    bool confirmInstall(const std::string& entity_id);
    bool cancelInstall(const std::string& entity_id);
    bool acknowledgeSuccess(const std::string& entity_id,
                            const std::string& clone_id);
    bool acknowledgeError(const std::string& entity_id,
                          const std::string& error_message);

    // --- Installed clones display ---
    int  getInstalledCloneCount(const std::string& entity_id) const;

    // --- Queries ---
    components::RelayCloneInstallUiState::UiStep
        getCurrentStep(const std::string& entity_id) const;
    std::string getSelectedStationId(const std::string& entity_id) const;
    float       getPendingCost(const std::string& entity_id) const;
    std::string getLastError(const std::string& entity_id) const;
    int  getTotalInstalls(const std::string& entity_id) const;
    int  getTotalCancels(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RelayCloneInstallUiState& comp,
                         float delta_time) override;

private:
    void rebuildFilteredList(components::RelayCloneInstallUiState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RELAY_CLONE_INSTALL_UI_SYSTEM_H
