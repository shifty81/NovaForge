#include "systems/aggro_table_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AggroTableSystem::AggroTableSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool AggroTableSystem::initializeAggroTable(const std::string& entity_id,
                                             float decay_rate,
                                             float decay_delay) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::AggroTable>()) return false;

    auto at = std::make_unique<components::AggroTable>();
    at->decay_rate = decay_rate;
    at->decay_delay = decay_delay;
    entity->addComponent(std::move(at));
    return true;
}

bool AggroTableSystem::recordThreat(const std::string& entity_id,
                                     const std::string& attacker_id,
                                     float amount) {
    auto* at = getComponentFor(entity_id);
    if (!at || !at->active || amount <= 0.0f || attacker_id.empty()) return false;

    // Find existing entry or create new one
    for (auto& e : at->entries) {
        if (e.attacker_id == attacker_id) {
            e.threat += amount;
            e.last_hit_time = at->elapsed;
            at->total_threat_events++;
            at->total_threat_accumulated += amount;
            return true;
        }
    }

    // New entry
    if (static_cast<int>(at->entries.size()) >= at->max_entries) return false;

    components::AggroTable::AggroEntry entry;
    entry.attacker_id = attacker_id;
    entry.threat = amount;
    entry.last_hit_time = at->elapsed;
    at->entries.push_back(entry);
    at->total_threat_events++;
    at->total_threat_accumulated += amount;
    return true;
}

float AggroTableSystem::getThreat(const std::string& entity_id,
                                   const std::string& attacker_id) const {
    auto* at = getComponentFor(entity_id);
    if (!at) return 0.0f;
    for (const auto& e : at->entries) {
        if (e.attacker_id == attacker_id) return e.threat;
    }
    return 0.0f;
}

std::string AggroTableSystem::getTopThreat(const std::string& entity_id) const {
    auto* at = getComponentFor(entity_id);
    if (!at || at->entries.empty()) return "";

    const components::AggroTable::AggroEntry* top = nullptr;
    for (const auto& e : at->entries) {
        if (!top || e.threat > top->threat) {
            top = &e;
        }
    }
    return top ? top->attacker_id : "";
}

int AggroTableSystem::getEntryCount(const std::string& entity_id) const {
    auto* at = getComponentFor(entity_id);
    return at ? static_cast<int>(at->entries.size()) : 0;
}

int AggroTableSystem::getTotalThreatEvents(const std::string& entity_id) const {
    auto* at = getComponentFor(entity_id);
    return at ? at->total_threat_events : 0;
}

float AggroTableSystem::getTotalThreatAccumulated(const std::string& entity_id) const {
    auto* at = getComponentFor(entity_id);
    return at ? at->total_threat_accumulated : 0.0f;
}

bool AggroTableSystem::clearTable(const std::string& entity_id) {
    auto* at = getComponentFor(entity_id);
    if (!at) return false;
    at->entries.clear();
    return true;
}

void AggroTableSystem::updateComponent(ecs::Entity& /*entity*/,
                                        components::AggroTable& at,
                                        float delta_time) {
    if (!at.active) return;
    at.elapsed += delta_time;

    // Decay threat for entries where enough time has passed since last hit
    for (auto it = at.entries.begin(); it != at.entries.end(); ) {
        float time_since_hit = at.elapsed - it->last_hit_time;
        if (time_since_hit > at.decay_delay) {
            it->threat -= at.decay_rate * delta_time;
            if (it->threat <= 0.0f) {
                it = at.entries.erase(it);
                continue;
            }
        }
        ++it;
    }
}

} // namespace systems
} // namespace atlas
