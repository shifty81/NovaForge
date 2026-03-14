#include "systems/survey_scanner_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SurveyScannerSystem::SurveyScannerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — advance scan timer, transition Scanning → Complete
// ---------------------------------------------------------------------------

void SurveyScannerSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SurveyScannerState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.status == components::SurveyScannerState::ScanStatus::Scanning) {
        comp.scan_timer += delta_time;
        if (comp.scan_timer >= comp.scan_duration) {
            comp.scan_timer = comp.scan_duration;
            comp.status = components::SurveyScannerState::ScanStatus::Complete;
            comp.total_scans_completed++;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SurveyScannerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SurveyScannerState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Scanning
// ---------------------------------------------------------------------------

bool SurveyScannerSystem::startScan(const std::string& entity_id,
                                     const std::string& target_belt_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_belt_id.empty()) return false;
    if (comp->status != components::SurveyScannerState::ScanStatus::Idle) return false;

    comp->target_belt_id = target_belt_id;
    comp->scan_timer     = 0.0f;
    comp->status         = components::SurveyScannerState::ScanStatus::Scanning;
    return true;
}

bool SurveyScannerSystem::cancelScan(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->status != components::SurveyScannerState::ScanStatus::Scanning)
        return false;

    comp->status     = components::SurveyScannerState::ScanStatus::Idle;
    comp->scan_timer = 0.0f;
    comp->target_belt_id.clear();
    return true;
}

bool SurveyScannerSystem::addResult(
        const std::string& entity_id,
        const std::string& result_id,
        const std::string& asteroid_id,
        const std::string& ore_type,
        float quantity,
        float estimated_value,
        float distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (result_id.empty()) return false;
    if (asteroid_id.empty()) return false;
    if (ore_type.empty()) return false;
    if (quantity <= 0.0f) return false;
    if (estimated_value < 0.0f) return false;
    if (distance < 0.0f) return false;

    // Duplicate check
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->results.size()) >= comp->max_results) return false;

    components::SurveyScannerState::SurveyResult r;
    r.result_id       = result_id;
    r.asteroid_id     = asteroid_id;
    r.ore_type        = ore_type;
    r.quantity         = quantity;
    r.estimated_value  = estimated_value;
    r.distance         = distance;
    comp->results.push_back(r);

    comp->total_value_scanned += estimated_value;
    return true;
}

bool SurveyScannerSystem::removeResult(const std::string& entity_id,
                                        const std::string& result_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->results.begin(), comp->results.end(),
        [&](const components::SurveyScannerState::SurveyResult& r) {
            return r.result_id == result_id;
        });
    if (it == comp->results.end()) return false;
    comp->results.erase(it);
    return true;
}

bool SurveyScannerSystem::clearResults(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->results.clear();
    // Reset to Idle if scan was complete
    if (comp->status == components::SurveyScannerState::ScanStatus::Complete) {
        comp->status = components::SurveyScannerState::ScanStatus::Idle;
        comp->scan_timer = 0.0f;
        comp->target_belt_id.clear();
    }
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool SurveyScannerSystem::setScanDuration(const std::string& entity_id,
                                           float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (duration <= 0.0f) return false;
    comp->scan_duration = duration;
    return true;
}

bool SurveyScannerSystem::setScanRange(const std::string& entity_id,
                                        float range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range <= 0.0f) return false;
    comp->scan_range = range;
    return true;
}

bool SurveyScannerSystem::setMaxResults(const std::string& entity_id,
                                         int max_results) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_results <= 0) return false;
    comp->max_results = max_results;
    return true;
}

bool SurveyScannerSystem::setScanDeviation(const std::string& entity_id,
                                            float deviation) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (deviation < 0.0f || deviation > 1.0f) return false;
    comp->scan_deviation = deviation;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int SurveyScannerSystem::getResultCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->results.size()) : 0;
}

bool SurveyScannerSystem::hasResult(const std::string& entity_id,
                                     const std::string& result_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return true;
    }
    return false;
}

float SurveyScannerSystem::getScanProgress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    if (comp->scan_duration <= 0.0f) return 0.0f;
    if (comp->status == components::SurveyScannerState::ScanStatus::Complete)
        return 1.0f;
    if (comp->status == components::SurveyScannerState::ScanStatus::Idle)
        return 0.0f;
    return comp->scan_timer / comp->scan_duration;
}

bool SurveyScannerSystem::isScanning(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->status == components::SurveyScannerState::ScanStatus::Scanning;
}

bool SurveyScannerSystem::isScanComplete(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->status == components::SurveyScannerState::ScanStatus::Complete;
}

int SurveyScannerSystem::getTotalScansCompleted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scans_completed : 0;
}

float SurveyScannerSystem::getTotalValueScanned(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_value_scanned : 0.0f;
}

std::string SurveyScannerSystem::getTargetBeltId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->target_belt_id : "";
}

float SurveyScannerSystem::getScanDuration(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_duration : 0.0f;
}

float SurveyScannerSystem::getScanRange(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_range : 0.0f;
}

float SurveyScannerSystem::getResultQuantity(
        const std::string& entity_id,
        const std::string& result_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return r.quantity;
    }
    return 0.0f;
}

std::string SurveyScannerSystem::getResultOreType(
        const std::string& entity_id,
        const std::string& result_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return r.ore_type;
    }
    return "";
}

} // namespace systems
} // namespace atlas
