#include "systems/relay_clone_install_ui_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

RelayCloneInstallUiSystem::RelayCloneInstallUiSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void RelayCloneInstallUiSystem::updateComponent(
    ecs::Entity& /*entity*/,
    components::RelayCloneInstallUiState& comp,
    float delta_time) {
    if (!comp.active) return;

    if (comp.current_step ==
            components::RelayCloneInstallUiState::UiStep::Pending) {
        comp.pending_elapsed += delta_time;
        if (comp.pending_elapsed >= comp.pending_timeout) {
            comp.current_step        = components::RelayCloneInstallUiState::UiStep::Error;
            comp.last_error_message  = "Request timed out";
            comp.pending_elapsed     = 0.0f;
        }
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void RelayCloneInstallUiSystem::rebuildFilteredList(
    components::RelayCloneInstallUiState& comp) {
    comp.filtered_stations.clear();
    const std::string& f = comp.station_search_filter;
    for (const auto& s : comp.available_stations) {
        if (!s.available) continue;
        if (f.empty()) {
            comp.filtered_stations.push_back(s);
            continue;
        }
        // Case-insensitive substring match on name or region
        auto contains = [&](const std::string& haystack) {
            std::string h = haystack, needle = f;
            std::transform(h.begin(), h.end(), h.begin(), ::tolower);
            std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
            return h.find(needle) != std::string::npos;
        };
        if (contains(s.station_name) || contains(s.region)) {
            comp.filtered_stations.push_back(s);
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool RelayCloneInstallUiSystem::initialize(const std::string& entity_id,
                                            const std::string& character_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RelayCloneInstallUiState>();
    comp->character_id = character_id.empty() ? entity_id : character_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Panel open / close
// ---------------------------------------------------------------------------

bool RelayCloneInstallUiSystem::openPanel(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->panel_open    = true;
    comp->current_step  = components::RelayCloneInstallUiState::UiStep::SelectStation;
    comp->selected_station_id.clear();
    comp->pending_cost  = 0.0f;
    comp->last_error_message.clear();
    comp->station_search_filter.clear();
    rebuildFilteredList(*comp);
    return true;
}

bool RelayCloneInstallUiSystem::closePanel(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->panel_open   = false;
    comp->current_step = components::RelayCloneInstallUiState::UiStep::Idle;
    return true;
}

bool RelayCloneInstallUiSystem::isPanelOpen(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->panel_open : false;
}

// ---------------------------------------------------------------------------
// Station list
// ---------------------------------------------------------------------------

bool RelayCloneInstallUiSystem::addStation(const std::string& entity_id,
                                            const std::string& station_id,
                                            const std::string& station_name,
                                            const std::string& region,
                                            float install_cost) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || station_id.empty()) return false;
    if (static_cast<int>(comp->available_stations.size()) >=
            comp->max_stations) return false;

    // No duplicates
    for (const auto& s : comp->available_stations) {
        if (s.station_id == station_id) return false;
    }

    components::RelayCloneInstallUiState::StationEntry e;
    e.station_id   = station_id;
    e.station_name = station_name;
    e.region       = region;
    e.install_cost = install_cost;
    e.available    = true;
    comp->available_stations.push_back(e);
    rebuildFilteredList(*comp);
    return true;
}

bool RelayCloneInstallUiSystem::removeStation(const std::string& entity_id,
                                               const std::string& station_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto& stations = comp->available_stations;
    auto it = std::find_if(stations.begin(), stations.end(),
        [&](const components::RelayCloneInstallUiState::StationEntry& s) {
            return s.station_id == station_id;
        });
    if (it == stations.end()) return false;
    stations.erase(it);
    rebuildFilteredList(*comp);
    return true;
}

int RelayCloneInstallUiSystem::getAvailableStationCount(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->available_stations.size()) : 0;
}

// ---------------------------------------------------------------------------
// Search / filter
// ---------------------------------------------------------------------------

bool RelayCloneInstallUiSystem::setSearchFilter(const std::string& entity_id,
                                                 const std::string& filter) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->station_search_filter = filter;
    rebuildFilteredList(*comp);
    return true;
}

std::string RelayCloneInstallUiSystem::getSearchFilter(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->station_search_filter : "";
}

int RelayCloneInstallUiSystem::getFilteredStationCount(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->filtered_stations.size()) : 0;
}

// ---------------------------------------------------------------------------
// UI step transitions
// ---------------------------------------------------------------------------

bool RelayCloneInstallUiSystem::selectStation(const std::string& entity_id,
                                               const std::string& station_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->current_step !=
            components::RelayCloneInstallUiState::UiStep::SelectStation)
        return false;

    // Find station and populate cost
    for (const auto& s : comp->available_stations) {
        if (s.station_id == station_id && s.available) {
            comp->selected_station_id = station_id;
            comp->pending_cost        = s.install_cost;
            comp->current_step =
                components::RelayCloneInstallUiState::UiStep::ConfirmCost;
            return true;
        }
    }
    return false;
}

bool RelayCloneInstallUiSystem::confirmInstall(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->current_step !=
            components::RelayCloneInstallUiState::UiStep::ConfirmCost)
        return false;

    comp->current_step    = components::RelayCloneInstallUiState::UiStep::Pending;
    comp->pending_elapsed = 0.0f;
    return true;
}

bool RelayCloneInstallUiSystem::cancelInstall(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using Step = components::RelayCloneInstallUiState::UiStep;
    if (comp->current_step == Step::Idle || comp->current_step == Step::Pending)
        return false;

    comp->total_cancels++;
    comp->current_step        = Step::SelectStation;
    comp->selected_station_id.clear();
    comp->pending_cost        = 0.0f;
    comp->last_error_message.clear();
    return true;
}

bool RelayCloneInstallUiSystem::acknowledgeSuccess(
    const std::string& entity_id, const std::string& clone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->current_step !=
            components::RelayCloneInstallUiState::UiStep::Pending)
        return false;

    // Record the newly installed clone
    components::RelayCloneInstallUiState::InstalledCloneEntry entry;
    entry.clone_id = clone_id;
    for (const auto& s : comp->available_stations) {
        if (s.station_id == comp->selected_station_id) {
            entry.station_id   = s.station_id;
            entry.station_name = s.station_name;
            break;
        }
    }
    comp->installed_clones.push_back(entry);
    comp->total_installs++;
    comp->current_step        = components::RelayCloneInstallUiState::UiStep::Success;
    comp->pending_elapsed     = 0.0f;
    return true;
}

bool RelayCloneInstallUiSystem::acknowledgeError(
    const std::string& entity_id, const std::string& error_message) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->current_step !=
            components::RelayCloneInstallUiState::UiStep::Pending)
        return false;

    comp->last_error_message = error_message;
    comp->current_step       = components::RelayCloneInstallUiState::UiStep::Error;
    comp->pending_elapsed    = 0.0f;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int RelayCloneInstallUiSystem::getInstalledCloneCount(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->installed_clones.size()) : 0;
}

components::RelayCloneInstallUiState::UiStep
RelayCloneInstallUiSystem::getCurrentStep(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_step
                : components::RelayCloneInstallUiState::UiStep::Idle;
}

std::string RelayCloneInstallUiSystem::getSelectedStationId(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->selected_station_id : "";
}

float RelayCloneInstallUiSystem::getPendingCost(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pending_cost : 0.0f;
}

std::string RelayCloneInstallUiSystem::getLastError(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->last_error_message : "";
}

int RelayCloneInstallUiSystem::getTotalInstalls(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_installs : 0;
}

int RelayCloneInstallUiSystem::getTotalCancels(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cancels : 0;
}

} // namespace systems
} // namespace atlas
