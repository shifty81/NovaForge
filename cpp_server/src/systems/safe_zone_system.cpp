#include "systems/safe_zone_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"

namespace atlas {
namespace systems {

namespace {

using SZ = components::SafeZone;

const char* stateToString(SZ::ZoneState s) {
    switch (s) {
        case SZ::ZoneState::Disabled:   return "Disabled";
        case SZ::ZoneState::Active:     return "Active";
        case SZ::ZoneState::Reinforced: return "Reinforced";
    }
    return "Unknown";
}

} // anonymous namespace

SafeZoneSystem::SafeZoneSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SafeZoneSystem::updateComponent(ecs::Entity& entity,
    components::SafeZone& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool SafeZoneSystem::initialize(const std::string& entity_id,
    const std::string& zone_id, const std::string& station_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SafeZone>();
    comp->zone_id = zone_id;
    comp->station_id = station_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SafeZoneSystem::setRadius(const std::string& entity_id, float radius) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (radius <= 0.0f) return false;
    sz->radius = radius;
    return true;
}

bool SafeZoneSystem::enableZone(const std::string& entity_id) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (sz->state == SZ::ZoneState::Active) return false;
    sz->state = SZ::ZoneState::Active;
    sz->weapons_disabled = true;
    return true;
}

bool SafeZoneSystem::disableZone(const std::string& entity_id) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (sz->state == SZ::ZoneState::Disabled) return false;
    sz->state = SZ::ZoneState::Disabled;
    sz->weapons_disabled = false;
    return true;
}

bool SafeZoneSystem::reinforceZone(const std::string& entity_id) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (sz->state == SZ::ZoneState::Disabled) return false;
    sz->state = SZ::ZoneState::Reinforced;
    sz->weapons_disabled = true;
    return true;
}

bool SafeZoneSystem::entityEnter(const std::string& entity_id) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (sz->state == SZ::ZoneState::Disabled) return false;
    if (sz->entities_inside >= sz->max_entities) return false;
    sz->entities_inside++;
    sz->total_entries++;
    return true;
}

bool SafeZoneSystem::entityExit(const std::string& entity_id) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (sz->entities_inside <= 0) return false;
    sz->entities_inside--;
    sz->total_exits++;
    return true;
}

bool SafeZoneSystem::blockWeapon(const std::string& entity_id) {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return false;
    if (!sz->weapons_disabled) return false;
    sz->total_weapons_blocked++;
    return true;
}

std::string SafeZoneSystem::getZoneState(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return "Unknown";
    return stateToString(sz->state);
}

float SafeZoneSystem::getRadius(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    return sz ? sz->radius : 0.0f;
}

int SafeZoneSystem::getEntitiesInside(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    return sz ? sz->entities_inside : 0;
}

bool SafeZoneSystem::areWeaponsDisabled(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    return sz ? sz->weapons_disabled : false;
}

float SafeZoneSystem::getTetherBonus(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    if (!sz) return 0.0f;
    if (sz->state == SZ::ZoneState::Disabled) return 0.0f;
    return sz->tether_speed_bonus;
}

int SafeZoneSystem::getTotalEntries(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    return sz ? sz->total_entries : 0;
}

int SafeZoneSystem::getTotalExits(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    return sz ? sz->total_exits : 0;
}

int SafeZoneSystem::getTotalWeaponsBlocked(const std::string& entity_id) const {
    auto* sz = getComponentFor(entity_id);
    return sz ? sz->total_weapons_blocked : 0;
}

} // namespace systems
} // namespace atlas
