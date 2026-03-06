#include "systems/mining_laser_cycle_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/exploration_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::MiningLaserCycle::ActiveCycle* findCycle(
    components::MiningLaserCycle* mlc, const std::string& laser_id) {
    for (auto& c : mlc->cycles) {
        if (c.laser_id == laser_id) return &c;
    }
    return nullptr;
}

const components::MiningLaserCycle::ActiveCycle* findCycleConst(
    const components::MiningLaserCycle* mlc, const std::string& laser_id) {
    for (const auto& c : mlc->cycles) {
        if (c.laser_id == laser_id) return &c;
    }
    return nullptr;
}
} // anonymous namespace

MiningLaserCycleSystem::MiningLaserCycleSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void MiningLaserCycleSystem::updateComponent(ecs::Entity& /*entity*/,
    components::MiningLaserCycle& mlc, float delta_time) {
    if (!mlc.active) return;
    mlc.elapsed += delta_time;

    for (auto& cycle : mlc.cycles) {
        if (cycle.completed) continue;

        float progress_increment = delta_time / cycle.cycle_time;
        cycle.progress += progress_increment;

        if (cycle.progress >= 1.0f) {
            cycle.progress = 1.0f;
            cycle.completed = true;

            // Transfer yield to cargo if space available
            float space_left = mlc.cargo_capacity - mlc.cargo_used;
            float actual_yield = std::min(cycle.yield_per_cycle, space_left);
            mlc.cargo_used += actual_yield;
            mlc.total_ore_mined += actual_yield;
            mlc.total_cycles_completed++;
        }
    }

    // Remove completed cycles
    mlc.cycles.erase(
        std::remove_if(mlc.cycles.begin(), mlc.cycles.end(),
            [](const components::MiningLaserCycle::ActiveCycle& c) { return c.completed; }),
        mlc.cycles.end());
}

bool MiningLaserCycleSystem::initializeMining(const std::string& entity_id,
    const std::string& cargo_entity_id, float cargo_capacity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MiningLaserCycle>();
    comp->cargo_entity_id = cargo_entity_id;
    comp->cargo_capacity = cargo_capacity;
    entity->addComponent(std::move(comp));
    return true;
}

bool MiningLaserCycleSystem::startLaser(const std::string& entity_id,
    const std::string& laser_id, const std::string& target_asteroid_id,
    const std::string& ore_type, float cycle_time, float yield_per_cycle) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    if (!mlc) return false;
    if (static_cast<int>(mlc->cycles.size()) >= mlc->max_active_lasers) return false;
    if (findCycle(mlc, laser_id)) return false;  // duplicate
    if (mlc->cargo_used >= mlc->cargo_capacity) return false;  // cargo full

    components::MiningLaserCycle::ActiveCycle cycle;
    cycle.laser_id = laser_id;
    cycle.target_asteroid_id = target_asteroid_id;
    cycle.ore_type = ore_type;
    cycle.cycle_time = cycle_time;
    cycle.yield_per_cycle = yield_per_cycle;
    mlc->cycles.push_back(cycle);
    return true;
}

bool MiningLaserCycleSystem::stopLaser(const std::string& entity_id,
    const std::string& laser_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    if (!mlc) return false;

    auto it = std::remove_if(mlc->cycles.begin(), mlc->cycles.end(),
        [&](const components::MiningLaserCycle::ActiveCycle& c) {
            return c.laser_id == laser_id;
        });
    if (it == mlc->cycles.end()) return false;
    mlc->cycles.erase(it, mlc->cycles.end());
    return true;
}

int MiningLaserCycleSystem::getActiveLaserCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    return mlc ? static_cast<int>(mlc->cycles.size()) : 0;
}

float MiningLaserCycleSystem::getLaserProgress(const std::string& entity_id,
    const std::string& laser_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    if (!mlc) return 0.0f;
    auto* cycle = findCycleConst(mlc, laser_id);
    return cycle ? cycle->progress : 0.0f;
}

int MiningLaserCycleSystem::getTotalCyclesCompleted(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    return mlc ? mlc->total_cycles_completed : 0;
}

float MiningLaserCycleSystem::getTotalOreMined(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    return mlc ? mlc->total_ore_mined : 0.0f;
}

float MiningLaserCycleSystem::getCargoUsed(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    return mlc ? mlc->cargo_used : 0.0f;
}

float MiningLaserCycleSystem::getCargoRemaining(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    return mlc ? (mlc->cargo_capacity - mlc->cargo_used) : 0.0f;
}

bool MiningLaserCycleSystem::isCargoFull(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    return mlc ? (mlc->cargo_used >= mlc->cargo_capacity) : false;
}

} // namespace systems
} // namespace atlas
