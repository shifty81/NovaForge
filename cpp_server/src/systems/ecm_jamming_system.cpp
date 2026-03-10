#include "systems/ecm_jamming_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <random>

namespace atlas {
namespace systems {

EcmJammingSystem::EcmJammingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void EcmJammingSystem::updateComponent(ecs::Entity& /*entity*/,
                                        components::EcmJammingState& comp,
                                        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Deterministic but non-trivially predictable RNG seeded per-entity
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    bool any_jamming = false;
    for (auto& jammer : comp.jammers) {
        jammer.cycle_elapsed += delta_time;
        if (jammer.cycle_elapsed >= jammer.cycle_time) {
            jammer.cycle_elapsed -= jammer.cycle_time;
            comp.total_jam_attempts++;

            // Jam probability: jam_strength / sensor_strength
            float probability = (comp.sensor_strength > 0.0f)
                ? jammer.jam_strength / comp.sensor_strength
                : 1.0f;
            bool success = (dist(rng) < probability);
            jammer.currently_jamming = success;
            if (success) {
                comp.total_jams_applied++;
                if (!comp.is_jammed) {
                    comp.total_lock_breaks++;
                }
            }
        }
        if (jammer.currently_jamming) any_jamming = true;
    }
    comp.is_jammed = any_jamming;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool EcmJammingSystem::initialize(const std::string& entity_id,
                                   float sensor_strength) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::EcmJammingState>();
    comp->sensor_strength = sensor_strength;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Jammer management
// ---------------------------------------------------------------------------

bool EcmJammingSystem::applyJammer(const std::string& entity_id,
                                    const std::string& source_id,
                                    float jam_strength,
                                    float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (source_id.empty() || jam_strength <= 0.0f || cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->jammers.size()) >= comp->max_jammers) return false;

    for (const auto& j : comp->jammers) {
        if (j.source_id == source_id) return false; // duplicate
    }

    components::EcmJammingState::Jammer jammer;
    jammer.source_id   = source_id;
    jammer.jam_strength = jam_strength;
    jammer.cycle_time  = cycle_time;
    jammer.cycle_elapsed = 0.0f;
    jammer.currently_jamming = false;
    comp->jammers.push_back(jammer);
    return true;
}

bool EcmJammingSystem::removeJammer(const std::string& entity_id,
                                     const std::string& source_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->jammers.begin(), comp->jammers.end(),
        [&](const components::EcmJammingState::Jammer& j) {
            return j.source_id == source_id;
        });
    if (it == comp->jammers.end()) return false;
    comp->jammers.erase(it);

    // Recheck jam state
    bool any_jamming = false;
    for (const auto& j : comp->jammers) {
        if (j.currently_jamming) { any_jamming = true; break; }
    }
    comp->is_jammed = any_jamming;
    return true;
}

bool EcmJammingSystem::clearJammers(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->jammers.clear();
    comp->is_jammed = false;
    return true;
}

// ---------------------------------------------------------------------------
// Sensor strength
// ---------------------------------------------------------------------------

bool EcmJammingSystem::setSensorStrength(const std::string& entity_id,
                                          float strength) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (strength <= 0.0f) return false;
    comp->sensor_strength = strength;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool EcmJammingSystem::isJammed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->is_jammed : false;
}

int EcmJammingSystem::getJammerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->jammers.size()) : 0;
}

float EcmJammingSystem::getSensorStrength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->sensor_strength : 0.0f;
}

float EcmJammingSystem::getTotalJamStrength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& j : comp->jammers) total += j.jam_strength;
    return total;
}

int EcmJammingSystem::getTotalJamsApplied(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jams_applied : 0;
}

int EcmJammingSystem::getTotalJamAttempts(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jam_attempts : 0;
}

int EcmJammingSystem::getTotalLockBreaks(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_lock_breaks : 0;
}

} // namespace systems
} // namespace atlas
