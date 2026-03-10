#include "systems/player_spawn_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

PlayerSpawnSystem::PlayerSpawnSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void PlayerSpawnSystem::updateComponent(ecs::Entity& /*entity*/,
                                        components::PlayerSpawn& comp,
                                        float delta_time) {
    if (!comp.active) return;
    if (comp.state != components::PlayerSpawn::SpawnState::Respawning) return;

    comp.respawn_timer -= delta_time;
    if (comp.respawn_timer <= 0.0f) {
        comp.respawn_timer = 0.0f;
        comp.state = components::PlayerSpawn::SpawnState::Spawned;
        comp.spawn_count++;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool PlayerSpawnSystem::initialize(const std::string& entity_id,
                                   const std::string& spawn_location) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || spawn_location.empty()) return false;
    auto comp = std::make_unique<components::PlayerSpawn>();
    comp->spawn_location = spawn_location;
    entity->addComponent(std::move(comp));
    return true;
}

bool PlayerSpawnSystem::spawnPlayer(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state == components::PlayerSpawn::SpawnState::Spawned) return false;
    comp->state = components::PlayerSpawn::SpawnState::Spawned;
    comp->spawn_count++;
    return true;
}

bool PlayerSpawnSystem::killPlayer(const std::string& entity_id,
                                   const std::string& death_location) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::PlayerSpawn::SpawnState::Spawned) return false;
    comp->state = components::PlayerSpawn::SpawnState::Dead;
    comp->death_location = death_location;
    comp->death_count++;
    return true;
}

bool PlayerSpawnSystem::beginRespawn(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::PlayerSpawn::SpawnState::Dead) return false;
    if (comp->max_respawn_attempts >= 0 &&
        comp->respawn_attempts >= comp->max_respawn_attempts) {
        return false;
    }
    comp->state = components::PlayerSpawn::SpawnState::Respawning;
    comp->respawn_timer = comp->respawn_cooldown;
    comp->respawn_attempts++;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

components::PlayerSpawn::SpawnState
PlayerSpawnSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->state : components::PlayerSpawn::SpawnState::Dead;
}

bool PlayerSpawnSystem::isSpawned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp && comp->state == components::PlayerSpawn::SpawnState::Spawned;
}

int PlayerSpawnSystem::getSpawnCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->spawn_count : 0;
}

int PlayerSpawnSystem::getDeathCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->death_count : 0;
}

float PlayerSpawnSystem::getRespawnTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->respawn_timer : 0.0f;
}

std::string PlayerSpawnSystem::getSpawnLocation(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->spawn_location : std::string();
}

std::string PlayerSpawnSystem::getDeathLocation(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->death_location : std::string();
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool PlayerSpawnSystem::setSpawnLocation(const std::string& entity_id,
                                         const std::string& location) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || location.empty()) return false;
    comp->spawn_location = location;
    return true;
}

bool PlayerSpawnSystem::setRespawnCooldown(const std::string& entity_id,
                                           float cooldown) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || cooldown < 0.0f) return false;
    comp->respawn_cooldown = cooldown;
    return true;
}

} // namespace systems
} // namespace atlas
