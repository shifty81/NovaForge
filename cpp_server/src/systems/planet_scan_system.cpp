#include "systems/planet_scan_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlanetScanSystem::PlanetScanSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlanetScanSystem::updateComponent(ecs::Entity& /*entity*/,
    components::PlanetScanState& comp, float delta_time) {
    if (!comp.active || !comp.scanning) return;
    comp.elapsed += delta_time;
    comp.scan_elapsed += delta_time;

    // Advance scan progress based on probe strength and elapsed time.
    // scan_duration is the time to complete at full (100%) probe strength.
    // progress_per_sec (%) = scan_strength / scan_duration
    float progress_per_sec = comp.scan_strength / comp.scan_duration;
    comp.scan_progress = std::min(100.0f,
        comp.scan_progress + progress_per_sec * delta_time);

    if (comp.scan_progress >= 100.0f) {
        comp.scan_progress = 100.0f;
        comp.scanning = false;
        comp.total_scans_completed++;
        // Generate results if not already populated
        if (comp.results.empty()) {
            // Default resource set based on planet type
            static const std::vector<std::string> barren_resources = {
                "base_metals", "carbon_compounds", "microorganisms"
            };
            static const std::vector<std::string> temperate_resources = {
                "carbon_compounds", "complex_organisms", "microorganisms",
                "aqueous_liquids", "autotrophs"
            };
            const auto* resource_list = &barren_resources;
            if (comp.planet_type == "temperate") {
                resource_list = &temperate_resources;
            }
            for (const auto& res : *resource_list) {
                components::PlanetScanState::ScanResult r;
                r.resource_type = res;
                // Richness scaled by probe strength (0–1)
                r.richness = comp.scan_strength / 100.0f;
                r.confirmed = false;
                comp.results.push_back(r);
            }
        }
    }
}

bool PlanetScanSystem::initialize(const std::string& entity_id,
    const std::string& planet_id, const std::string& planet_type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PlanetScanState>();
    comp->planet_id = planet_id;
    comp->planet_type = planet_type;
    entity->addComponent(std::move(comp));
    return true;
}

bool PlanetScanSystem::launchScan(const std::string& entity_id,
    float probe_strength) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->scanning) return false;  // already scanning
    if (probe_strength <= 0.0f) return false;

    comp->scan_strength = std::min(100.0f, probe_strength);
    comp->scan_progress = 0.0f;
    comp->scan_elapsed = 0.0f;
    comp->scanning = true;
    comp->results.clear();
    comp->probes_launched++;
    return true;
}

bool PlanetScanSystem::cancelScan(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->scanning) return false;
    comp->scanning = false;
    comp->scan_progress = 0.0f;
    comp->scan_elapsed = 0.0f;
    return true;
}

bool PlanetScanSystem::confirmResult(const std::string& entity_id,
    const std::string& resource_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& r : comp->results) {
        if (r.resource_type == resource_type) {
            r.confirmed = true;
            return true;
        }
    }
    return false;
}

bool PlanetScanSystem::isScanning(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scanning : false;
}

float PlanetScanSystem::getScanProgress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_progress : 0.0f;
}

int PlanetScanSystem::getResultCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->results.size()) : 0;
}

int PlanetScanSystem::getProbesLaunched(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->probes_launched : 0;
}

int PlanetScanSystem::getTotalScansCompleted(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scans_completed : 0;
}

} // namespace systems
} // namespace atlas
