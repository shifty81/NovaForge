#include "systems/imperfect_information_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ImperfectInformationSystem::ImperfectInformationSystem(ecs::World* world)
    : System(world) {
}

void ImperfectInformationSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::EntityIntel>();
    for (auto* entity : entities) {
        auto* intel = entity->getComponent<components::EntityIntel>();
        if (!intel) continue;

        for (auto& entry : intel->entries) {
            entry.age += delta_time;

            // Decay confidence over time
            entry.confidence -= entry.decay_rate * delta_time;
            entry.confidence = std::max(0.0f, entry.confidence);

            // Mark as ghost if below threshold
            if (entry.confidence < intel->ghost_threshold && !entry.is_ghost) {
                entry.is_ghost = true;
            }
        }

        // Remove entries with zero confidence that are ghosts
        intel->entries.erase(
            std::remove_if(intel->entries.begin(), intel->entries.end(),
                [](const components::EntityIntel::IntelEntry& e) {
                    return e.confidence <= 0.0f;
                }),
            intel->entries.end()
        );
    }
}

bool ImperfectInformationSystem::recordIntel(const std::string& observer_id,
                                              const std::string& target_id,
                                              float scan_quality,
                                              float distance) {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return false;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return false;

    // Compute confidence: scan_quality * sensor_strength, reduced by distance
    float distance_factor = 1.0f / (1.0f + distance * 0.001f);
    float confidence = std::clamp(scan_quality * intel->sensor_strength * distance_factor, 0.0f, 1.0f);

    // Update existing or add new
    auto* existing = intel->getEntry(target_id);
    if (existing) {
        // Refresh: take the better confidence
        existing->confidence = std::max(existing->confidence, confidence);
        existing->scan_quality = scan_quality;
        existing->age = 0.0f;
        existing->distance_at_scan = distance;
        existing->is_ghost = false;
    } else {
        if (static_cast<int>(intel->entries.size()) >= intel->max_entries) {
            // Evict oldest entry
            auto oldest = std::max_element(intel->entries.begin(), intel->entries.end(),
                [](const components::EntityIntel::IntelEntry& a, const components::EntityIntel::IntelEntry& b) {
                    return a.age < b.age;
                });
            if (oldest != intel->entries.end()) {
                intel->entries.erase(oldest);
            }
        }

        components::EntityIntel::IntelEntry entry;
        entry.target_id = target_id;
        entry.confidence = confidence;
        entry.scan_quality = scan_quality;
        entry.age = 0.0f;
        entry.distance_at_scan = distance;
        entry.is_ghost = false;
        intel->entries.push_back(entry);
    }

    intel->total_scans++;
    return true;
}

bool ImperfectInformationSystem::clearIntel(const std::string& observer_id,
                                             const std::string& target_id) {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return false;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return false;

    auto it = std::remove_if(intel->entries.begin(), intel->entries.end(),
        [&target_id](const components::EntityIntel::IntelEntry& e) {
            return e.target_id == target_id;
        });
    if (it == intel->entries.end()) return false;
    intel->entries.erase(it, intel->entries.end());
    return true;
}

bool ImperfectInformationSystem::clearAllIntel(const std::string& observer_id) {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return false;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return false;

    intel->entries.clear();
    return true;
}

bool ImperfectInformationSystem::setSensorStrength(const std::string& observer_id, float strength) {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return false;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return false;

    intel->sensor_strength = std::max(0.0f, strength);
    return true;
}

float ImperfectInformationSystem::getConfidence(const std::string& observer_id,
                                                 const std::string& target_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return 0.0f;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return 0.0f;

    auto* entry = intel->getEntry(target_id);
    return entry ? entry->confidence : 0.0f;
}

bool ImperfectInformationSystem::hasIntel(const std::string& observer_id,
                                           const std::string& target_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return false;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return false;

    return intel->getEntry(target_id) != nullptr;
}

float ImperfectInformationSystem::getIntelAge(const std::string& observer_id,
                                               const std::string& target_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return 0.0f;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return 0.0f;

    auto* entry = intel->getEntry(target_id);
    return entry ? entry->age : 0.0f;
}

bool ImperfectInformationSystem::isGhost(const std::string& observer_id,
                                          const std::string& target_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return false;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return false;

    auto* entry = intel->getEntry(target_id);
    return entry ? entry->is_ghost : false;
}

int ImperfectInformationSystem::getIntelCount(const std::string& observer_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return 0;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return 0;

    return static_cast<int>(intel->entries.size());
}

int ImperfectInformationSystem::getTotalScans(const std::string& observer_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return 0;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return 0;

    return intel->total_scans;
}

float ImperfectInformationSystem::getSensorStrength(const std::string& observer_id) const {
    auto* entity = world_->getEntity(observer_id);
    if (!entity) return 0.0f;

    auto* intel = entity->getComponent<components::EntityIntel>();
    if (!intel) return 0.0f;

    return intel->sensor_strength;
}

} // namespace systems
} // namespace atlas
