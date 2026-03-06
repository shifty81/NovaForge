#include "systems/entity_lifecycle_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/core_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using LC = components::EntityLifecycle;
using Evt = components::EntityLifecycle::LifecycleEvent;
using EvtType = components::EntityLifecycle::EventType;
using DeathCause = components::EntityLifecycle::DeathCause;

void addEvent(LC* lc, EvtType type, const std::string& cause, const std::string& etype) {
    // Evict oldest if at capacity
    if (static_cast<int>(lc->events.size()) >= lc->max_events) {
        lc->events.erase(lc->events.begin());
    }
    Evt e;
    e.event_type = type;
    e.timestamp = lc->elapsed;
    e.cause = cause;
    e.entity_type = etype;
    lc->events.push_back(e);
}

} // anonymous namespace

EntityLifecycleSystem::EntityLifecycleSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void EntityLifecycleSystem::updateComponent(ecs::Entity& entity,
    components::EntityLifecycle& lc, float delta_time) {
    if (!lc.active) return;

    lc.elapsed += delta_time;
    if (lc.alive) {
        lc.lifetime += delta_time;
    }
}

bool EntityLifecycleSystem::initialize(const std::string& entity_id,
    const std::string& entity_type, const std::string& spawn_source) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::EntityLifecycle>();
    comp->entity_type = entity_type;
    comp->spawn_source = spawn_source;
    entity->addComponent(std::move(comp));
    return true;
}

bool EntityLifecycleSystem::recordSpawn(const std::string& entity_id,
    const std::string& entity_type) {
    auto* lc = getComponentFor(entity_id);
    if (!lc) return false;
    addEvent(lc, EvtType::Spawned, "spawn", entity_type);
    lc->total_spawned++;
    return true;
}

bool EntityLifecycleSystem::recordDestroy(const std::string& entity_id,
    int death_cause, const std::string& cause_detail) {
    auto* lc = getComponentFor(entity_id);
    if (!lc) return false;
    if (death_cause < 0 || death_cause > 5) return false;
    addEvent(lc, EvtType::Destroyed, cause_detail, lc->entity_type);
    lc->total_destroyed++;
    lc->death_cause = static_cast<DeathCause>(death_cause);
    lc->alive = false;
    return true;
}

bool EntityLifecycleSystem::recordStateChange(const std::string& entity_id,
    const std::string& description) {
    auto* lc = getComponentFor(entity_id);
    if (!lc) return false;
    addEvent(lc, EvtType::StateChange, description, lc->entity_type);
    lc->total_state_changes++;
    return true;
}

bool EntityLifecycleSystem::markDead(const std::string& entity_id, int death_cause) {
    auto* lc = getComponentFor(entity_id);
    if (!lc) return false;
    if (death_cause < 0 || death_cause > 5) return false;
    lc->alive = false;
    lc->death_cause = static_cast<DeathCause>(death_cause);
    return true;
}

int EntityLifecycleSystem::getEventCount(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? static_cast<int>(lc->events.size()) : 0;
}

float EntityLifecycleSystem::getLifetime(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->lifetime : 0.0f;
}

int EntityLifecycleSystem::getDeathCause(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? static_cast<int>(lc->death_cause) : 0;
}

int EntityLifecycleSystem::getTotalSpawned(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->total_spawned : 0;
}

int EntityLifecycleSystem::getTotalDestroyed(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->total_destroyed : 0;
}

int EntityLifecycleSystem::getTotalStateChanges(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->total_state_changes : 0;
}

bool EntityLifecycleSystem::isAlive(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->alive : false;
}

std::string EntityLifecycleSystem::getEntityType(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->entity_type : "";
}

std::string EntityLifecycleSystem::getSpawnSource(const std::string& entity_id) const {
    auto* lc = getComponentFor(entity_id);
    return lc ? lc->spawn_source : "";
}

} // namespace systems
} // namespace atlas
