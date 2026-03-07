#include "systems/asteroid_mining_laser_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AsteroidMiningLaserSystem::AsteroidMiningLaserSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AsteroidMiningLaserSystem::updateComponent(ecs::Entity& entity,
    components::AsteroidMiningLaser& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& laser : comp.lasers) {
        if (!laser.cycling) continue;

        // Stop cycling when hold is full
        if (comp.ore_hold_current >= comp.ore_hold_capacity) {
            laser.cycling = false;
            continue;
        }

        laser.cycle_progress += delta_time;
        if (laser.cycle_progress >= laser.cycle_time) {
            laser.cycle_progress -= laser.cycle_time;

            // Calculate yield with crystal bonus
            float yield = laser.yield_per_cycle * (1.0f + laser.crystal_bonus);
            double space_left = comp.ore_hold_capacity - comp.ore_hold_current;
            double actual = std::min(static_cast<double>(yield), space_left);

            comp.ore_hold_current += actual;
            comp.total_ore_mined += actual;
            comp.total_cycles_completed++;
        }
    }
}

bool AsteroidMiningLaserSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AsteroidMiningLaser>();
    entity->addComponent(std::move(comp));
    return true;
}

bool AsteroidMiningLaserSystem::addLaser(const std::string& entity_id,
    const std::string& laser_id, float yield_per_cycle, float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->lasers.size()) >= comp->max_lasers) return false;

    // No duplicate laser IDs
    for (const auto& l : comp->lasers) {
        if (l.laser_id == laser_id) return false;
    }

    components::AsteroidMiningLaser::MiningLaser laser;
    laser.laser_id = laser_id;
    laser.yield_per_cycle = yield_per_cycle;
    laser.cycle_time = cycle_time;
    comp->lasers.push_back(laser);
    return true;
}

bool AsteroidMiningLaserSystem::loadCrystal(const std::string& entity_id,
    const std::string& laser_id, const std::string& crystal_id, float bonus) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& l : comp->lasers) {
        if (l.laser_id == laser_id) {
            l.crystal_id = crystal_id;
            l.crystal_bonus = bonus;
            return true;
        }
    }
    return false;
}

bool AsteroidMiningLaserSystem::startCycle(const std::string& entity_id,
    const std::string& laser_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->target_asteroid_id.empty()) return false;
    if (comp->ore_hold_current >= comp->ore_hold_capacity) return false;
    for (auto& l : comp->lasers) {
        if (l.laser_id == laser_id) {
            if (l.cycling) return false;
            l.cycling = true;
            l.cycle_progress = 0.0f;
            return true;
        }
    }
    return false;
}

bool AsteroidMiningLaserSystem::stopCycle(const std::string& entity_id,
    const std::string& laser_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& l : comp->lasers) {
        if (l.laser_id == laser_id) {
            if (!l.cycling) return false;
            l.cycling = false;
            l.cycle_progress = 0.0f;
            return true;
        }
    }
    return false;
}

bool AsteroidMiningLaserSystem::setTarget(const std::string& entity_id,
    const std::string& asteroid_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->target_asteroid_id = asteroid_id;
    return true;
}

int AsteroidMiningLaserSystem::getLaserCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->lasers.size()) : 0;
}

bool AsteroidMiningLaserSystem::isCycling(const std::string& entity_id,
    const std::string& laser_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& l : comp->lasers) {
        if (l.laser_id == laser_id) return l.cycling;
    }
    return false;
}

double AsteroidMiningLaserSystem::getOreHoldCurrent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->ore_hold_current : 0.0;
}

double AsteroidMiningLaserSystem::getTotalOreMined(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ore_mined : 0.0;
}

int AsteroidMiningLaserSystem::getTotalCyclesCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cycles_completed : 0;
}

bool AsteroidMiningLaserSystem::isHoldFull(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->ore_hold_current >= comp->ore_hold_capacity;
}

} // namespace systems
} // namespace atlas
