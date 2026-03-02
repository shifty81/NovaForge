#include "systems/cargo_scan_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CargoScanSystem::CargoScanSystem(ecs::World* world)
    : System(world) {
}

void CargoScanSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::CargoScanState>();
    for (auto* entity : entities) {
        auto* scan = entity->getComponent<components::CargoScanState>();
        if (!scan) continue;

        if (scan->phase != components::CargoScanState::ScanPhase::Scanning) continue;

        scan->scan_timer += delta_time;
        if (scan->scan_timer < scan->scan_time) continue;

        // Scan complete — resolve
        scan->total_scans++;

        auto* target = world_->getEntity(scan->target_entity_id);
        int found = 0;
        if (target) {
            auto* target_scan = target->getComponent<components::CargoScanState>();
            if (target_scan && !target_scan->detected_types.empty() && scan->detection_chance > 0.0f) {
                found = static_cast<int>(target_scan->detected_types.size());
            }
        }

        scan->contraband_found = found;
        scan->total_contraband_detected += found;
        scan->total_fines_issued += found * static_cast<double>(scan->fine_per_contraband);
        scan->phase = components::CargoScanState::ScanPhase::Complete;
    }
}

bool CargoScanSystem::initiateScan(const std::string& scanner_id, const std::string& target_id) {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return false;

    if (scan->phase != components::CargoScanState::ScanPhase::Idle) return false;

    auto* target = world_->getEntity(target_id);
    if (!target) return false;

    scan->phase = components::CargoScanState::ScanPhase::Scanning;
    scan->target_entity_id = target_id;
    scan->scan_timer = 0.0f;
    scan->contraband_found = 0;
    return true;
}

bool CargoScanSystem::cancelScan(const std::string& scanner_id) {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return false;

    if (scan->phase != components::CargoScanState::ScanPhase::Scanning) return false;

    scan->phase = components::CargoScanState::ScanPhase::Idle;
    scan->scan_timer = 0.0f;
    scan->target_entity_id.clear();
    return true;
}

bool CargoScanSystem::setDetectionChance(const std::string& scanner_id, float chance) {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return false;

    scan->detection_chance = std::max(0.0f, std::min(1.0f, chance));
    return true;
}

bool CargoScanSystem::markAsCustomsScanner(const std::string& scanner_id, bool is_customs) {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return false;

    scan->is_customs_scanner = is_customs;
    return true;
}

bool CargoScanSystem::plantContraband(const std::string& entity_id, components::CargoScanState::ContrabandType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) {
        entity->addComponent<components::CargoScanState>(std::make_unique<components::CargoScanState>());
        scan = entity->getComponent<components::CargoScanState>();
        if (!scan) return false;
    }

    scan->detected_types.push_back(type);
    return true;
}

bool CargoScanSystem::removeContraband(const std::string& entity_id, components::CargoScanState::ContrabandType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return false;

    auto it = std::find(scan->detected_types.begin(), scan->detected_types.end(), type);
    if (it == scan->detected_types.end()) return false;

    scan->detected_types.erase(it);
    return true;
}

std::string CargoScanSystem::getPhase(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return "unknown";

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return "unknown";

    return components::CargoScanState::phaseToString(scan->phase);
}

float CargoScanSystem::getScanProgress(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return 0.0f;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return 0.0f;

    if (scan->scan_time <= 0.0f) return 0.0f;
    return std::max(0.0f, std::min(1.0f, scan->scan_timer / scan->scan_time));
}

int CargoScanSystem::getContrabandFound(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return 0;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return 0;

    return scan->contraband_found;
}

int CargoScanSystem::getTotalScans(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return 0;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return 0;

    return scan->total_scans;
}

int CargoScanSystem::getTotalContrabandDetected(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return 0;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return 0;

    return scan->total_contraband_detected;
}

double CargoScanSystem::getTotalFinesIssued(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return 0.0;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return 0.0;

    return scan->total_fines_issued;
}

bool CargoScanSystem::isCustomsScanner(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return false;

    return scan->is_customs_scanner;
}

std::vector<std::string> CargoScanSystem::getDetectedTypes(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return {};

    auto* scan = entity->getComponent<components::CargoScanState>();
    if (!scan) return {};

    std::vector<std::string> result;
    result.reserve(scan->detected_types.size());
    for (const auto& t : scan->detected_types) {
        result.push_back(components::CargoScanState::contrabandToString(t));
    }
    return result;
}

} // namespace systems
} // namespace atlas
