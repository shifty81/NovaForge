#include "systems/sleeper_cache_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SleeperCacheSystem::SleeperCacheSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SleeperCacheSystem::updateComponent(ecs::Entity& entity,
    components::SleeperCacheState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Count down site timer
    if (!comp.expired) {
        comp.time_remaining -= delta_time;
        if (comp.time_remaining <= 0.0f) {
            comp.time_remaining = 0.0f;
            comp.expired = true;
        }
    }
}

bool SleeperCacheSystem::initialize(const std::string& entity_id,
    const std::string& site_id, components::SleeperCacheState::CacheTier tier) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SleeperCacheState>();
    comp->site_id = site_id;
    comp->tier = tier;
    // Set tier-based defaults
    if (tier == components::SleeperCacheState::CacheTier::Standard) {
        comp->time_limit = 900.0f;
        comp->time_remaining = 900.0f;
        comp->max_rooms = 5;
    } else if (tier == components::SleeperCacheState::CacheTier::Superior) {
        comp->time_limit = 1200.0f;
        comp->time_remaining = 1200.0f;
        comp->max_rooms = 7;
    }
    entity->addComponent(std::move(comp));
    return true;
}

bool SleeperCacheSystem::addRoom(const std::string& entity_id,
    const std::string& room_id, int sentry_count, float sentry_dps) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->rooms.size()) >= comp->max_rooms) return false;

    components::SleeperCacheState::CacheRoom room;
    room.room_id = room_id;
    room.sentry_count = sentry_count;
    room.sentry_dps = sentry_dps;
    comp->rooms.push_back(room);
    return true;
}

bool SleeperCacheSystem::addContainer(const std::string& entity_id,
    const std::string& room_id, const std::string& container_id,
    float difficulty, float loot_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& room : comp->rooms) {
        if (room.room_id == room_id) {
            components::SleeperCacheState::Container c;
            c.container_id = container_id;
            c.hack_difficulty = difficulty;
            c.loot_value = loot_value;
            room.containers.push_back(c);
            return true;
        }
    }
    return false;
}

bool SleeperCacheSystem::openRoom(const std::string& entity_id,
    const std::string& room_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& room : comp->rooms) {
        if (room.room_id == room_id &&
            room.status == components::SleeperCacheState::RoomStatus::Locked) {
            room.status = components::SleeperCacheState::RoomStatus::Open;
            return true;
        }
    }
    return false;
}

bool SleeperCacheSystem::hackContainer(const std::string& entity_id,
    const std::string& room_id, const std::string& container_id,
    float hack_amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->expired) return false;
    for (auto& room : comp->rooms) {
        if (room.room_id == room_id &&
            room.status == components::SleeperCacheState::RoomStatus::Open) {
            for (auto& c : room.containers) {
                if (c.container_id == container_id && !c.hacked && !c.exploded) {
                    c.hack_progress += hack_amount;
                    if (c.hack_progress >= c.hack_difficulty) {
                        c.hacked = true;
                        comp->containers_hacked++;
                        comp->total_loot_value += c.loot_value;
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

bool SleeperCacheSystem::failContainer(const std::string& entity_id,
    const std::string& room_id, const std::string& container_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& room : comp->rooms) {
        if (room.room_id == room_id) {
            for (auto& c : room.containers) {
                if (c.container_id == container_id && !c.hacked && !c.exploded) {
                    c.exploded = true;
                    comp->containers_failed++;
                    return true;
                }
            }
        }
    }
    return false;
}

bool SleeperCacheSystem::destroySentries(const std::string& entity_id,
    const std::string& room_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& room : comp->rooms) {
        if (room.room_id == room_id && room.sentries_alive) {
            room.sentries_alive = false;
            return true;
        }
    }
    return false;
}

int SleeperCacheSystem::getRoomCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->rooms.size()) : 0;
}

int SleeperCacheSystem::getContainersHacked(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->containers_hacked : 0;
}

int SleeperCacheSystem::getContainersFailed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->containers_failed : 0;
}

float SleeperCacheSystem::getTotalLootValue(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_loot_value : 0.0f;
}

float SleeperCacheSystem::getTimeRemaining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->time_remaining : 0.0f;
}

bool SleeperCacheSystem::isExpired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->expired : true;
}

bool SleeperCacheSystem::isRoomOpen(const std::string& entity_id,
    const std::string& room_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& room : comp->rooms) {
        if (room.room_id == room_id) {
            return room.status == components::SleeperCacheState::RoomStatus::Open;
        }
    }
    return false;
}

bool SleeperCacheSystem::isContainerHacked(const std::string& entity_id,
    const std::string& room_id, const std::string& container_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& room : comp->rooms) {
        if (room.room_id == room_id) {
            for (const auto& c : room.containers) {
                if (c.container_id == container_id) return c.hacked;
            }
        }
    }
    return false;
}

components::SleeperCacheState::CacheTier SleeperCacheSystem::getCacheTier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->tier : components::SleeperCacheState::CacheTier::Limited;
}

} // namespace systems
} // namespace atlas
