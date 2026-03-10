#include "systems/damage_log_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

DamageLogSystem::DamageLogSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void DamageLogSystem::updateComponent(ecs::Entity& /*entity*/,
                                      components::DamageLog& comp,
                                      float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool DamageLogSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DamageLog>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

bool DamageLogSystem::logOutgoing(const std::string& entity_id,
                                   const std::string& defender_id,
                                   components::DamageLog::DamageType damage_type,
                                   float amount,
                                   const std::string& weapon,
                                   bool hit) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) {
        comp->entries.erase(comp->entries.begin());
    }

    components::DamageLog::DamageEntry entry;
    entry.attacker_id  = entity_id;
    entry.defender_id  = defender_id;
    entry.damage_type  = damage_type;
    entry.amount       = hit ? amount : 0.0f;
    entry.weapon       = weapon;
    entry.hit          = hit;
    entry.timestamp    = comp->elapsed;

    comp->entries.push_back(entry);
    comp->total_shots++;
    if (hit) {
        comp->total_outgoing += amount;
    } else {
        comp->total_misses++;
    }
    return true;
}

bool DamageLogSystem::logIncoming(const std::string& entity_id,
                                   const std::string& attacker_id,
                                   components::DamageLog::DamageType damage_type,
                                   float amount,
                                   const std::string& weapon,
                                   bool hit) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) {
        comp->entries.erase(comp->entries.begin());
    }

    components::DamageLog::DamageEntry entry;
    entry.attacker_id  = attacker_id;
    entry.defender_id  = entity_id;
    entry.damage_type  = damage_type;
    entry.amount       = hit ? amount : 0.0f;
    entry.weapon       = weapon;
    entry.hit          = hit;
    entry.timestamp    = comp->elapsed;

    comp->entries.push_back(entry);
    comp->total_shots++;
    if (hit) {
        comp->total_incoming += amount;
    } else {
        comp->total_misses++;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Management
// ---------------------------------------------------------------------------

bool DamageLogSystem::clearEntries(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int DamageLogSystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->entries.size()) : 0;
}

float DamageLogSystem::getTotalOutgoing(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_outgoing : 0.0f;
}

float DamageLogSystem::getTotalIncoming(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_incoming : 0.0f;
}

int DamageLogSystem::getTotalMisses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_misses : 0;
}

int DamageLogSystem::getTotalShots(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_shots : 0;
}

components::DamageLog::DamageEntry
DamageLogSystem::getMostRecentEntry(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->entries.empty()) return {};
    return comp->entries.back();
}

} // namespace systems
} // namespace atlas
