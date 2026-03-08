#include "systems/signature_analysis_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SignatureAnalysisSystem::SignatureAnalysisSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SignatureAnalysisSystem::updateComponent(ecs::Entity& /*entity*/,
    components::SignatureAnalysisState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& reading : comp.readings) {
        if (reading.identified) continue;
        reading.scan_resolution += comp.analysis_speed * comp.scan_power * delta_time;
    }
}

bool SignatureAnalysisSystem::initialize(const std::string& entity_id, float scan_power) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (scan_power <= 0.0f) return false;

    auto comp = std::make_unique<components::SignatureAnalysisState>();
    comp->scan_power = scan_power;
    entity->addComponent(std::move(comp));
    return true;
}

bool SignatureAnalysisSystem::addReading(const std::string& entity_id,
    const std::string& sig_id, const std::string& sig_type, float signal_strength) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (sig_id.empty()) return false;
    if (sig_type.empty()) return false;
    if (signal_strength <= 0.0f) return false;

    for (const auto& r : comp->readings) {
        if (r.sig_id == sig_id) return false;
    }
    if (static_cast<int>(comp->readings.size()) >= comp->max_readings) return false;

    components::SignatureAnalysisState::SignatureReading reading;
    reading.sig_id = sig_id;
    reading.sig_type = sig_type;
    reading.signal_strength = signal_strength;
    comp->readings.push_back(reading);
    return true;
}

bool SignatureAnalysisSystem::removeReading(const std::string& entity_id,
    const std::string& sig_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->readings.begin(), comp->readings.end(),
        [&sig_id](const components::SignatureAnalysisState::SignatureReading& r) {
            return r.sig_id == sig_id;
        });
    if (it == comp->readings.end()) return false;
    comp->readings.erase(it);
    return true;
}

bool SignatureAnalysisSystem::analyzeSignature(const std::string& entity_id,
    const std::string& sig_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& r : comp->readings) {
        if (r.sig_id == sig_id) {
            if (r.identified) return false;
            comp->total_scans++;
            if (r.scan_resolution >= r.signal_strength) {
                r.identified = true;
                comp->total_identified++;
                return true;
            }
            return false;
        }
    }
    return false;
}

bool SignatureAnalysisSystem::setScanPower(const std::string& entity_id, float power) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (power <= 0.0f) return false;
    comp->scan_power = power;
    return true;
}

bool SignatureAnalysisSystem::setAnalysisSpeed(const std::string& entity_id, float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (speed <= 0.0f) return false;
    comp->analysis_speed = speed;
    return true;
}

int SignatureAnalysisSystem::getReadingCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->readings.size()) : 0;
}

int SignatureAnalysisSystem::getIdentifiedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& r : comp->readings) {
        if (r.identified) count++;
    }
    return count;
}

float SignatureAnalysisSystem::getSignalStrength(const std::string& entity_id,
    const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->readings) {
        if (r.sig_id == sig_id) return r.signal_strength;
    }
    return 0.0f;
}

bool SignatureAnalysisSystem::isIdentified(const std::string& entity_id,
    const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& r : comp->readings) {
        if (r.sig_id == sig_id) return r.identified;
    }
    return false;
}

float SignatureAnalysisSystem::getScanPower(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_power : 0.0f;
}

int SignatureAnalysisSystem::getTotalScans(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scans : 0;
}

int SignatureAnalysisSystem::getTotalIdentified(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_identified : 0;
}

} // namespace systems
} // namespace atlas
