#include "systems/mining_belt_populator_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using MBP = components::MiningBeltPopulator;
using AsteroidEntry = components::MiningBeltPopulator::AsteroidEntry;

AsteroidEntry* findAsteroid(MBP* mbp, const std::string& asteroid_id) {
    for (auto& a : mbp->asteroids) {
        if (a.asteroid_id == asteroid_id) return &a;
    }
    return nullptr;
}

const AsteroidEntry* findAsteroidConst(const MBP* mbp, const std::string& asteroid_id) {
    for (const auto& a : mbp->asteroids) {
        if (a.asteroid_id == asteroid_id) return &a;
    }
    return nullptr;
}

} // anonymous namespace

MiningBeltPopulatorSystem::MiningBeltPopulatorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MiningBeltPopulatorSystem::updateComponent(ecs::Entity& entity,
    components::MiningBeltPopulator& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Check for newly depleted asteroids
    for (auto& a : comp.asteroids) {
        if (!a.depleted && a.remaining_quantity <= 0.0f) {
            a.depleted = true;
            a.remaining_quantity = 0.0f;
            comp.total_mined++;
        }
    }

    // Respawn timer
    comp.respawn_timer += delta_time;
    if (comp.respawn_timer >= comp.respawn_interval) {
        comp.respawn_timer -= comp.respawn_interval;
        for (auto& a : comp.asteroids) {
            if (a.depleted) {
                a.remaining_quantity = a.initial_quantity;
                a.depleted = false;
                comp.total_respawned++;
            }
        }
    }
}

bool MiningBeltPopulatorSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MiningBeltPopulator>();
    entity->addComponent(std::move(comp));
    return true;
}

bool MiningBeltPopulatorSystem::addAsteroid(const std::string& entity_id,
    const std::string& asteroid_id, const std::string& ore_type,
    float quantity, float richness) {
    auto* mbp = getComponentFor(entity_id);
    if (!mbp) return false;
    if (static_cast<int>(mbp->asteroids.size()) >= mbp->max_asteroids) return false;
    if (findAsteroid(mbp, asteroid_id)) return false; // duplicate

    AsteroidEntry a;
    a.asteroid_id = asteroid_id;
    a.ore_type = ore_type;
    a.initial_quantity = quantity;
    a.remaining_quantity = quantity;
    a.richness = richness;
    mbp->asteroids.push_back(a);
    return true;
}

bool MiningBeltPopulatorSystem::removeAsteroid(const std::string& entity_id,
    const std::string& asteroid_id) {
    auto* mbp = getComponentFor(entity_id);
    if (!mbp) return false;
    auto it = std::find_if(mbp->asteroids.begin(), mbp->asteroids.end(),
        [&](const AsteroidEntry& a) { return a.asteroid_id == asteroid_id; });
    if (it == mbp->asteroids.end()) return false;
    mbp->asteroids.erase(it);
    return true;
}

bool MiningBeltPopulatorSystem::extractOre(const std::string& entity_id,
    const std::string& asteroid_id, float amount) {
    auto* mbp = getComponentFor(entity_id);
    if (!mbp) return false;

    auto* asteroid = findAsteroid(mbp, asteroid_id);
    if (!asteroid || asteroid->depleted) return false;

    float extracted = std::min(amount, asteroid->remaining_quantity);
    asteroid->remaining_quantity -= extracted;
    mbp->total_ore_extracted += extracted;
    return true;
}

int MiningBeltPopulatorSystem::getAsteroidCount(const std::string& entity_id) const {
    auto* mbp = getComponentFor(entity_id);
    return mbp ? static_cast<int>(mbp->asteroids.size()) : 0;
}

int MiningBeltPopulatorSystem::getDepletedCount(const std::string& entity_id) const {
    auto* mbp = getComponentFor(entity_id);
    if (!mbp) return 0;
    int count = 0;
    for (const auto& a : mbp->asteroids) {
        if (a.depleted) count++;
    }
    return count;
}

float MiningBeltPopulatorSystem::getRemainingOre(const std::string& entity_id,
    const std::string& asteroid_id) const {
    auto* mbp = getComponentFor(entity_id);
    if (!mbp) return 0.0f;
    const auto* asteroid = findAsteroidConst(mbp, asteroid_id);
    return asteroid ? asteroid->remaining_quantity : 0.0f;
}

float MiningBeltPopulatorSystem::getTotalOreExtracted(const std::string& entity_id) const {
    auto* mbp = getComponentFor(entity_id);
    return mbp ? mbp->total_ore_extracted : 0.0f;
}

int MiningBeltPopulatorSystem::getTotalMined(const std::string& entity_id) const {
    auto* mbp = getComponentFor(entity_id);
    return mbp ? mbp->total_mined : 0;
}

int MiningBeltPopulatorSystem::getTotalRespawned(const std::string& entity_id) const {
    auto* mbp = getComponentFor(entity_id);
    return mbp ? mbp->total_respawned : 0;
}

bool MiningBeltPopulatorSystem::isAsteroidDepleted(const std::string& entity_id,
    const std::string& asteroid_id) const {
    auto* mbp = getComponentFor(entity_id);
    if (!mbp) return false;
    const auto* asteroid = findAsteroidConst(mbp, asteroid_id);
    return asteroid ? asteroid->depleted : false;
}

} // namespace systems
} // namespace atlas
