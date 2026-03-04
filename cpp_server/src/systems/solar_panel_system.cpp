#include "systems/solar_panel_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

SolarPanelSystem::SolarPanelSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void SolarPanelSystem::updateComponent(ecs::Entity& /*entity*/, components::SolarPanel& panel, float delta_time) {
    if (panel.is_deployed) {
        // Advance day cycle
        panel.day_cycle_position += panel.day_cycle_speed * delta_time;
        if (panel.day_cycle_position >= 1.0f) {
            panel.day_cycle_position -= 1.0f;
        }

        // Calculate energy output
        float sunlight = panel.getSunlightFactor();
        panel.total_energy_output = panel.panel_count * panel.energy_per_panel
            * panel.panel_efficiency * panel.maintenance_level * sunlight;

        // Degrade efficiency
        panel.panel_efficiency -= panel.degradation_rate * delta_time;
        if (panel.panel_efficiency < 0.0f) {
            panel.panel_efficiency = 0.0f;
        }

        // Store energy
        panel.energy_stored += panel.total_energy_output * delta_time;
        if (panel.energy_stored > panel.max_energy_storage) {
            panel.energy_stored = panel.max_energy_storage;
        }
    } else {
        panel.total_energy_output = 0.0f;
    }
}

bool SolarPanelSystem::initializePanels(const std::string& entity_id,
                                         const std::string& owner_id,
                                         int panel_count) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::SolarPanel>();
    if (existing) return false;

    auto comp = std::make_unique<components::SolarPanel>();
    comp->owner_entity_id = owner_id;
    comp->panel_count = std::max(0, std::min(comp->max_panels, panel_count));
    entity->addComponent(std::move(comp));
    return true;
}

bool SolarPanelSystem::removePanels(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* panel = entity->getComponent<components::SolarPanel>();
    if (!panel) return false;

    entity->removeComponent<components::SolarPanel>();
    return true;
}

bool SolarPanelSystem::deployPanels(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->is_deployed = true;
    return true;
}

bool SolarPanelSystem::retractPanels(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->is_deployed = false;
    panel->total_energy_output = 0.0f;
    return true;
}

bool SolarPanelSystem::addPanel(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    if (panel->panel_count >= panel->max_panels) return false;

    panel->panel_count++;
    return true;
}

bool SolarPanelSystem::removePanel(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    if (panel->panel_count <= 0) return false;

    panel->panel_count--;
    return true;
}

bool SolarPanelSystem::setDayCyclePosition(const std::string& entity_id, float position) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->day_cycle_position = std::max(0.0f, std::min(1.0f, position));
    return true;
}

bool SolarPanelSystem::performMaintenance(const std::string& entity_id, float amount) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->panel_efficiency += amount;
    if (panel->panel_efficiency > 1.0f) {
        panel->panel_efficiency = 1.0f;
    }
    return true;
}

float SolarPanelSystem::getEnergyOutput(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return 0.0f;

    return panel->total_energy_output;
}

float SolarPanelSystem::getEnergyStored(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return 0.0f;

    return panel->energy_stored;
}

int SolarPanelSystem::getPanelCount(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return 0;

    return panel->panel_count;
}

float SolarPanelSystem::getEfficiency(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return 0.0f;

    return panel->panel_efficiency;
}

bool SolarPanelSystem::isDeployed(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    return panel->is_deployed;
}

bool SolarPanelSystem::isDaytime(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    return panel->getSunlightFactor() > 0.0f;
}

} // namespace systems
} // namespace atlas
