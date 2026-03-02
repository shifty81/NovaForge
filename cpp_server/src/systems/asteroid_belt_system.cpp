#include "systems/asteroid_belt_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/exploration_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::AsteroidBelt::Asteroid* findAsteroid(
    components::AsteroidBelt* belt, const std::string& asteroid_id) {
    for (auto& a : belt->asteroids) {
        if (a.asteroid_id == asteroid_id) return &a;
    }
    return nullptr;
}

const components::AsteroidBelt::Asteroid* findAsteroidConst(
    const components::AsteroidBelt* belt, const std::string& asteroid_id) {
    for (const auto& a : belt->asteroids) {
        if (a.asteroid_id == asteroid_id) return &a;
    }
    return nullptr;
}
} // anonymous namespace

AsteroidBeltSystem::AsteroidBeltSystem(ecs::World* world)
    : System(world) {
}

void AsteroidBeltSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::AsteroidBelt>();
    for (auto* entity : entities) {
        auto* belt = entity->getComponent<components::AsteroidBelt>();
        if (!belt || !belt->active) continue;

        // Check for depleted asteroids and manage respawn timer
        bool has_depleted = false;
        for (const auto& a : belt->asteroids) {
            if (a.depleted) { has_depleted = true; break; }
        }

        if (has_depleted) {
            belt->respawn_timer += delta_time;
            if (belt->respawn_timer >= belt->respawn_interval) {
                belt->respawn_timer = 0.0f;
                for (auto& a : belt->asteroids) {
                    if (a.depleted) {
                        a.quantity = a.max_quantity;
                        a.depleted = false;
                        belt->total_respawned++;
                    }
                }
            }
        }
    }
}

bool AsteroidBeltSystem::initializeBelt(const std::string& entity_id,
    const std::string& belt_id, const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AsteroidBelt>();
    comp->belt_id = belt_id;
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool AsteroidBeltSystem::addAsteroid(const std::string& entity_id,
    const std::string& asteroid_id, const std::string& ore_type,
    float quantity, float richness) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return false;
    if (static_cast<int>(belt->asteroids.size()) >= belt->max_asteroids) return false;
    if (findAsteroid(belt, asteroid_id)) return false; // duplicate

    components::AsteroidBelt::Asteroid ast;
    ast.asteroid_id = asteroid_id;
    ast.ore_type = ore_type;
    ast.quantity = quantity;
    ast.max_quantity = quantity;
    ast.richness = richness;
    belt->asteroids.push_back(ast);
    return true;
}

bool AsteroidBeltSystem::removeAsteroid(const std::string& entity_id,
    const std::string& asteroid_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return false;

    auto it = std::remove_if(belt->asteroids.begin(), belt->asteroids.end(),
        [&](const components::AsteroidBelt::Asteroid& a) {
            return a.asteroid_id == asteroid_id;
        });
    if (it == belt->asteroids.end()) return false;
    belt->asteroids.erase(it, belt->asteroids.end());
    return true;
}

float AsteroidBeltSystem::mineAsteroid(const std::string& entity_id,
    const std::string& asteroid_id, float amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return 0.0f;

    auto* ast = findAsteroid(belt, asteroid_id);
    if (!ast || ast->depleted) return 0.0f;

    float actual = std::min(amount * ast->richness, ast->quantity);
    ast->quantity -= actual;
    if (ast->quantity <= 0.0f) {
        ast->quantity = 0.0f;
        ast->depleted = true;
        belt->total_mined++;
    }
    return actual;
}

int AsteroidBeltSystem::getAsteroidCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return 0;
    return static_cast<int>(belt->asteroids.size());
}

int AsteroidBeltSystem::getDepletedCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return 0;
    int count = 0;
    for (const auto& a : belt->asteroids) {
        if (a.depleted) count++;
    }
    return count;
}

float AsteroidBeltSystem::getRemainingOre(const std::string& entity_id,
    const std::string& asteroid_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return 0.0f;
    auto* ast = findAsteroidConst(belt, asteroid_id);
    return ast ? ast->quantity : 0.0f;
}

bool AsteroidBeltSystem::isAsteroidDepleted(const std::string& entity_id,
    const std::string& asteroid_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return false;
    auto* ast = findAsteroidConst(belt, asteroid_id);
    return ast ? ast->depleted : false;
}

int AsteroidBeltSystem::getTotalMined(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return 0;
    return belt->total_mined;
}

int AsteroidBeltSystem::getTotalRespawned(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    if (!belt) return 0;
    return belt->total_respawned;
}

} // namespace systems
} // namespace atlas
