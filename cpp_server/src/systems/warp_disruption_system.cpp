#include "systems/warp_disruption_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

using WD = components::WarpDisruption;

WarpDisruptionSystem::WarpDisruptionSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void WarpDisruptionSystem::updateComponent(ecs::Entity& /*entity*/,
    components::WarpDisruption& wd, float delta_time) {
    if (!wd.component_active) return;
    wd.elapsed += delta_time;

    // Update durations and remove expired/inactive disruptors
    for (auto& d : wd.disruptors) {
        d.duration += delta_time;
    }
    wd.disruptors.erase(
        std::remove_if(wd.disruptors.begin(), wd.disruptors.end(),
            [](const WD::Disruptor& d) { return !d.active; }),
        wd.disruptors.end());

    // Recalculate total strength
    int total = 0;
    for (const auto& d : wd.disruptors) {
        total += d.strength;
    }
    wd.total_disruption_strength = total;
    wd.warp_blocked = (total >= wd.warp_core_strength);
}

bool WarpDisruptionSystem::initializeDisruption(const std::string& entity_id,
    int warp_core_strength) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::WarpDisruption>();
    comp->warp_core_strength = warp_core_strength;
    entity->addComponent(std::move(comp));
    return true;
}

bool WarpDisruptionSystem::applyDisruptor(const std::string& entity_id,
    const std::string& source_id, int strength, float range) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    if (!wd) return false;
    if (static_cast<int>(wd->disruptors.size()) >= wd->max_disruptors) return false;

    // Check for duplicate source
    for (const auto& d : wd->disruptors) {
        if (d.source_id == source_id) return false;
    }

    WD::Disruptor dis;
    dis.source_id = source_id;
    dis.strength = strength;
    dis.range = range;
    wd->disruptors.push_back(dis);
    wd->total_disruptions_applied++;

    // Recalculate
    int total = 0;
    for (const auto& d : wd->disruptors) {
        total += d.strength;
    }
    wd->total_disruption_strength = total;
    wd->warp_blocked = (total >= wd->warp_core_strength);
    return true;
}

bool WarpDisruptionSystem::removeDisruptor(const std::string& entity_id,
    const std::string& source_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    if (!wd) return false;

    auto it = std::remove_if(wd->disruptors.begin(), wd->disruptors.end(),
        [&](const WD::Disruptor& d) { return d.source_id == source_id; });
    if (it == wd->disruptors.end()) return false;
    wd->disruptors.erase(it, wd->disruptors.end());

    // Check if warp becomes unblocked → counts as escape
    int total = 0;
    for (const auto& d : wd->disruptors) {
        total += d.strength;
    }
    wd->total_disruption_strength = total;
    bool was_blocked = wd->warp_blocked;
    wd->warp_blocked = (total >= wd->warp_core_strength);
    if (was_blocked && !wd->warp_blocked) {
        wd->total_escapes++;
    }
    return true;
}

int WarpDisruptionSystem::getDisruptorCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    return wd ? static_cast<int>(wd->disruptors.size()) : 0;
}

int WarpDisruptionSystem::getTotalStrength(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    return wd ? wd->total_disruption_strength : 0;
}

bool WarpDisruptionSystem::isWarpBlocked(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    return wd ? wd->warp_blocked : false;
}

int WarpDisruptionSystem::getWarpCoreStrength(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    return wd ? wd->warp_core_strength : 0;
}

int WarpDisruptionSystem::getTotalDisruptionsApplied(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    return wd ? wd->total_disruptions_applied : 0;
}

int WarpDisruptionSystem::getTotalEscapes(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    return wd ? wd->total_escapes : 0;
}

bool WarpDisruptionSystem::clearAllDisruptors(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* wd = entity->getComponent<components::WarpDisruption>();
    if (!wd) return false;
    bool was_blocked = wd->warp_blocked;
    wd->disruptors.clear();
    wd->total_disruption_strength = 0;
    wd->warp_blocked = false;
    if (was_blocked) wd->total_escapes++;
    return true;
}

} // namespace systems
} // namespace atlas
