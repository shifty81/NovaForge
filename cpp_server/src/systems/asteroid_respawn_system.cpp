#include "systems/asteroid_respawn_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using AR = components::AsteroidRespawn;

const char* stateToString(AR::RespawnState s) {
    switch (s) {
        case AR::RespawnState::Active:       return "Active";
        case AR::RespawnState::Depleted:     return "Depleted";
        case AR::RespawnState::Regenerating: return "Regenerating";
        case AR::RespawnState::Full:         return "Full";
    }
    return "Unknown";
}

AR::RespawnState evaluateState(const AR& comp) {
    if (comp.total_asteroids >= comp.max_asteroids) return AR::RespawnState::Full;
    if (comp.total_asteroids == 0) return AR::RespawnState::Depleted;
    if (comp.depleted_count > 0) return AR::RespawnState::Regenerating;
    return AR::RespawnState::Active;
}

} // anonymous namespace

AsteroidRespawnSystem::AsteroidRespawnSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AsteroidRespawnSystem::updateComponent(ecs::Entity& entity,
    components::AsteroidRespawn& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Update depletion percentage
    if (comp.max_asteroids > 0) {
        comp.depletion_pct = 1.0f - static_cast<float>(comp.total_asteroids) /
                             static_cast<float>(comp.max_asteroids);
    }

    // Handle regeneration delay
    if (comp.depleted_count > 0 && comp.delay_timer > 0.0f) {
        comp.delay_timer -= delta_time;
        if (comp.delay_timer > 0.0f) {
            comp.state = evaluateState(comp);
            return;  // still waiting
        }
        comp.delay_timer = 0.0f;
    }

    // Respawn asteroids gradually
    if (comp.depleted_count > 0 && comp.total_asteroids < comp.max_asteroids) {
        comp.respawn_accumulator += comp.respawn_rate * delta_time;
        while (comp.respawn_accumulator >= 1.0f && comp.depleted_count > 0) {
            comp.respawn_accumulator -= 1.0f;
            comp.total_asteroids = std::min(comp.total_asteroids + 1, comp.max_asteroids);
            comp.depleted_count = std::max(0, comp.depleted_count - 1);
            comp.total_respawned++;
        }
    }

    comp.state = evaluateState(comp);
}

bool AsteroidRespawnSystem::initialize(const std::string& entity_id,
    const std::string& belt_id, const std::string& system_id, int max_asteroids) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AsteroidRespawn>();
    comp->belt_id = belt_id;
    comp->system_id = system_id;
    comp->max_asteroids = max_asteroids;
    comp->total_asteroids = max_asteroids;
    entity->addComponent(std::move(comp));
    return true;
}

bool AsteroidRespawnSystem::deplete(const std::string& entity_id, int count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (count <= 0) return false;
    int actual = std::min(count, comp->total_asteroids);
    comp->total_asteroids -= actual;
    comp->depleted_count += actual;
    comp->total_depleted += actual;
    // Reset delay timer when new depletion occurs
    comp->delay_timer = comp->regeneration_delay;
    return true;
}

bool AsteroidRespawnSystem::setRespawnRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->respawn_rate = rate;
    return true;
}

bool AsteroidRespawnSystem::setRegenerationDelay(const std::string& entity_id,
    float delay_seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->regeneration_delay = delay_seconds;
    return true;
}

std::string AsteroidRespawnSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return stateToString(comp->state);
}

int AsteroidRespawnSystem::getTotalAsteroids(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_asteroids : 0;
}

int AsteroidRespawnSystem::getMaxAsteroids(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_asteroids : 0;
}

int AsteroidRespawnSystem::getDepletedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->depleted_count : 0;
}

float AsteroidRespawnSystem::getDepletionPct(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->depletion_pct : 0.0f;
}

int AsteroidRespawnSystem::getTotalRespawned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_respawned : 0;
}

int AsteroidRespawnSystem::getTotalDepleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_depleted : 0;
}

} // namespace systems
} // namespace atlas
