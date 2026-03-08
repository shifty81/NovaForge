#include "systems/asteroid_scanner_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

AsteroidScannerSystem::AsteroidScannerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AsteroidScannerSystem::updateComponent(ecs::Entity& entity,
    components::AsteroidScannerState& state, float delta_time) {
    if (!state.active) return;

    if (state.scanning && !state.scan_complete) {
        float increment = delta_time / state.scan_duration;
        state.scan_progress = std::min(1.0f, state.scan_progress + increment);
        if (state.scan_progress >= 1.0f) {
            state.scanning = false;
            state.scan_complete = true;
            state.total_scans_completed++;
            float total = 0.0f;
            for (const auto& r : state.readings) total += r.estimated_value;
            state.total_value_scanned += total;
        }
    }

    state.elapsed += delta_time;
}

bool AsteroidScannerSystem::initialize(const std::string& entity_id,
    const std::string& target_asteroid) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (target_asteroid.empty()) return false;
    auto comp = std::make_unique<components::AsteroidScannerState>();
    comp->target_asteroid_id = target_asteroid;
    entity->addComponent(std::move(comp));
    return true;
}

bool AsteroidScannerSystem::startScan(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state || state->scanning || state->scan_complete) return false;
    state->scanning = true;
    state->scan_progress = 0.0f;
    return true;
}

bool AsteroidScannerSystem::cancelScan(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->scanning) return false;
    state->scanning = false;
    state->scan_progress = 0.0f;
    return true;
}

bool AsteroidScannerSystem::addReading(const std::string& entity_id,
    const std::string& ore_type, float concentration, float value) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (ore_type.empty()) return false;
    if (static_cast<int>(state->readings.size()) >= state->max_readings) return false;
    concentration = std::max(0.0f, std::min(1.0f, concentration));
    value = std::max(0.0f, value);
    state->readings.push_back({ore_type, concentration, value});
    return true;
}

bool AsteroidScannerSystem::removeReading(const std::string& entity_id,
    const std::string& ore_type) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->readings.begin(), state->readings.end(),
        [&](const components::AsteroidScannerState::OreReading& r) {
            return r.ore_type == ore_type;
        });
    if (it == state->readings.end()) return false;
    state->readings.erase(it);
    return true;
}

bool AsteroidScannerSystem::setScanDuration(const std::string& entity_id, float duration) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->scan_duration = std::max(1.0f, std::min(60.0f, duration));
    return true;
}

bool AsteroidScannerSystem::setScanResolution(const std::string& entity_id, float resolution) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->scan_resolution = std::max(0.1f, std::min(5.0f, resolution));
    return true;
}

float AsteroidScannerSystem::getScanProgress(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->scan_progress : 0.0f;
}

bool AsteroidScannerSystem::isScanning(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->isScanning() : false;
}

bool AsteroidScannerSystem::isScanComplete(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->scan_complete : false;
}

int AsteroidScannerSystem::getReadingCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->readings.size()) : 0;
}

int AsteroidScannerSystem::getTotalScans(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_scans_completed : 0;
}

float AsteroidScannerSystem::getTotalValueScanned(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_value_scanned : 0.0f;
}

std::string AsteroidScannerSystem::getTargetAsteroid(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->target_asteroid_id : "";
}

} // namespace systems
} // namespace atlas
