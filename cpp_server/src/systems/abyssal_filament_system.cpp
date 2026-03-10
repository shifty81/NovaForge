#include "systems/abyssal_filament_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AbyssalFilamentSystem::AbyssalFilamentSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AbyssalFilamentSystem::updateComponent(ecs::Entity& /*entity*/,
    components::AbyssalFilamentState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Tick the current pocket's countdown timer
    if (comp.current_pocket < static_cast<int>(comp.pockets.size())) {
        auto& pocket = comp.pockets[comp.current_pocket];
        if (!pocket.completed && !pocket.failed) {
            pocket.time_remaining -= delta_time;
            if (pocket.time_remaining <= 0.0f) {
                pocket.time_remaining = 0.0f;
                pocket.failed = true;
                comp.pockets_failed++;
                comp.active = false;
            }
        }
    }
}

bool AbyssalFilamentSystem::initialize(const std::string& entity_id,
    const std::string& pilot_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AbyssalFilamentState>();
    comp->pilot_id = pilot_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool AbyssalFilamentSystem::activateFilament(const std::string& entity_id,
    components::AbyssalFilamentState::FilamentType type,
    components::AbyssalFilamentState::Tier tier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->active) return false;   // already in a run

    // Build the three pocket entries for this run
    comp->pockets.clear();
    float base_time = 1200.0f;  // 20 minutes per pocket
    for (int i = 0; i < comp->max_pockets; i++) {
        components::AbyssalFilamentState::PocketEntry p;
        p.pocket_id = "pocket_" + std::to_string(i + 1);
        p.type = type;
        p.tier = tier;
        p.time_limit = base_time;
        p.time_remaining = base_time;
        comp->pockets.push_back(p);
    }
    comp->current_pocket = 0;
    comp->filaments_consumed++;
    comp->active = true;
    return true;
}

bool AbyssalFilamentSystem::completePocket(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->active) return false;
    if (comp->current_pocket >= static_cast<int>(comp->pockets.size())) return false;

    auto& pocket = comp->pockets[comp->current_pocket];
    if (pocket.completed || pocket.failed) return false;

    pocket.completed = true;
    comp->pockets_completed++;
    comp->current_pocket++;

    // Check if all pockets are done
    if (comp->current_pocket >= comp->max_pockets) {
        comp->active = false;
    }
    return true;
}

bool AbyssalFilamentSystem::cancelRun(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->active) return false;
    comp->active = false;
    return true;
}

int AbyssalFilamentSystem::getCurrentPocket(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_pocket : 0;
}

int AbyssalFilamentSystem::getPocketsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pockets_completed : 0;
}

int AbyssalFilamentSystem::getPocketsFailed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pockets_failed : 0;
}

int AbyssalFilamentSystem::getFilamentsConsumed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->filaments_consumed : 0;
}

bool AbyssalFilamentSystem::isActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active : false;
}

bool AbyssalFilamentSystem::isRunComplete(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active) return false;
    return comp->pockets_completed == comp->max_pockets;
}

float AbyssalFilamentSystem::getPocketTimeRemaining(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->current_pocket >= static_cast<int>(comp->pockets.size()))
        return 0.0f;
    return comp->pockets[comp->current_pocket].time_remaining;
}

} // namespace systems
} // namespace atlas
