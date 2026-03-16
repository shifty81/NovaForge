#include "systems/access_list_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AccessListSystem::AccessListSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — count down TTL, remove expired entries
// ---------------------------------------------------------------------------

void AccessListSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::AccessListState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Count down TTL for entries that have one
    auto it = comp.entries.begin();
    while (it != comp.entries.end()) {
        if (it->ttl > 0.0f) {
            it->ttl -= delta_time;
            if (it->ttl <= 0.0f) {
                it = comp.entries.erase(it);
                comp.total_entries_expired++;
                continue;
            }
        }
        ++it;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool AccessListSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AccessListState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Entry management
// ---------------------------------------------------------------------------

bool AccessListSystem::addEntry(
        const std::string& entity_id,
        const std::string& entry_id,
        const std::string& member_id,
        components::AccessListState::Permission permission,
        float ttl) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (member_id.empty()) return false;
    if (ttl < 0.0f) return false;

    // Duplicate check
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries)
        return false;

    components::AccessListState::AclEntry entry;
    entry.entry_id   = entry_id;
    entry.member_id  = member_id;
    entry.permission = permission;
    entry.ttl        = ttl;
    comp->entries.push_back(entry);
    comp->total_entries_added++;
    return true;
}

bool AccessListSystem::removeEntry(const std::string& entity_id,
                                    const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->entries.begin(), comp->entries.end(),
        [&](const components::AccessListState::AclEntry& e) {
            return e.entry_id == entry_id;
        });
    if (it == comp->entries.end()) return false;
    comp->entries.erase(it);
    return true;
}

bool AccessListSystem::clearEntries(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Operations
// ---------------------------------------------------------------------------

bool AccessListSystem::checkAccess(const std::string& entity_id,
                                    const std::string& member_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_access_checks++;

    // Owner always has access
    if (!comp->owner_id.empty() && member_id == comp->owner_id) return true;

    // Check explicit entries — last match wins (first found)
    for (const auto& e : comp->entries) {
        if (e.member_id == member_id) {
            return e.permission == components::AccessListState::Permission::Allow;
        }
    }

    // Fall back to default policy
    return comp->default_policy == components::AccessListState::Permission::Allow;
}

bool AccessListSystem::setPermission(
        const std::string& entity_id,
        const std::string& entry_id,
        components::AccessListState::Permission permission) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.entry_id == entry_id) {
            e.permission = permission;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool AccessListSystem::setOwnerId(const std::string& entity_id,
                                   const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->owner_id = owner_id;
    return true;
}

bool AccessListSystem::setStructureId(const std::string& entity_id,
                                       const std::string& structure_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->structure_id = structure_id;
    return true;
}

bool AccessListSystem::setDefaultPolicy(
        const std::string& entity_id,
        components::AccessListState::Permission policy) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->default_policy = policy;
    return true;
}

bool AccessListSystem::setMaxEntries(const std::string& entity_id,
                                      int max_entries) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_entries <= 0) return false;
    comp->max_entries = max_entries;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int AccessListSystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->entries.size()) : 0;
}

bool AccessListSystem::hasEntry(const std::string& entity_id,
                                 const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

bool AccessListSystem::hasMember(const std::string& entity_id,
                                  const std::string& member_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.member_id == member_id) return true;
    }
    return false;
}

std::string AccessListSystem::getOwnerId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->owner_id : "";
}

std::string AccessListSystem::getStructureId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->structure_id : "";
}

int AccessListSystem::getTotalEntriesAdded(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_entries_added : 0;
}

int AccessListSystem::getTotalEntriesExpired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_entries_expired : 0;
}

int AccessListSystem::getTotalAccessChecks(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_access_checks : 0;
}

float AccessListSystem::getEntryTtl(const std::string& entity_id,
                                     const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.ttl;
    }
    return 0.0f;
}

bool AccessListSystem::isDefaultAllow(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->default_policy == components::AccessListState::Permission::Allow;
}

} // namespace systems
} // namespace atlas
