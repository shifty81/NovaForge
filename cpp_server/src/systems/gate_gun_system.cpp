#include "systems/gate_gun_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/combat_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

GateGunSystem::GateGunSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void GateGunSystem::updateComponent(ecs::Entity& /*entity*/,
    components::GateGunState& gun, float delta_time) {
    if (!gun.active || !gun.online) return;
    if (gun.targets.empty()) {
        gun.cycle_progress = 0.0f;
        return;
    }

    gun.elapsed += delta_time;

    // Advance fire cycle
    gun.cycle_progress += delta_time;
    if (gun.cycle_progress >= gun.cycle_time) {
        gun.cycle_progress -= gun.cycle_time;
        gun.total_shots_fired++;

        // Apply damage to first (highest-threat) target
        if (!gun.targets.empty()) {
            gun.total_damage_dealt += gun.damage_per_cycle;
        }
    }

    // Tick engagement timers
    for (auto& t : gun.targets) {
        t.time_engaged += delta_time;
    }
}

bool GateGunSystem::initialize(const std::string& entity_id, const std::string& gate_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::GateGunState>();
    comp->gate_id = gate_id.empty() ? entity_id : gate_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool GateGunSystem::addTarget(const std::string& entity_id, const std::string& target_id,
    float threat_level, bool is_criminal) {
    auto* gun = getComponentFor(entity_id);
    if (!gun || !gun->online) return false;
    if (static_cast<int>(gun->targets.size()) >= gun->max_targets) return false;

    // Don't add duplicates
    for (const auto& t : gun->targets) {
        if (t.entity_id == target_id) return false;
    }

    components::GateGunState::Target tgt;
    tgt.entity_id = target_id;
    tgt.threat_level = (std::max)(0.0f, (std::min)(10.0f, threat_level));
    tgt.is_criminal = is_criminal;
    gun->targets.push_back(tgt);
    return true;
}

bool GateGunSystem::removeTarget(const std::string& entity_id, const std::string& target_id) {
    auto* gun = getComponentFor(entity_id);
    if (!gun) return false;
    auto it = std::find_if(gun->targets.begin(), gun->targets.end(),
        [&](const components::GateGunState::Target& t) { return t.entity_id == target_id; });
    if (it == gun->targets.end()) return false;
    gun->targets.erase(it);
    return true;
}

int GateGunSystem::getTargetCount(const std::string& entity_id) const {
    auto* gun = getComponentFor(entity_id);
    return gun ? static_cast<int>(gun->targets.size()) : 0;
}

int GateGunSystem::getTotalShotsFired(const std::string& entity_id) const {
    auto* gun = getComponentFor(entity_id);
    return gun ? gun->total_shots_fired : 0;
}

float GateGunSystem::getTotalDamageDealt(const std::string& entity_id) const {
    auto* gun = getComponentFor(entity_id);
    return gun ? gun->total_damage_dealt : 0.0f;
}

int GateGunSystem::getTotalKills(const std::string& entity_id) const {
    auto* gun = getComponentFor(entity_id);
    return gun ? gun->total_kills : 0;
}

bool GateGunSystem::setOnline(const std::string& entity_id, bool online) {
    auto* gun = getComponentFor(entity_id);
    if (!gun) return false;
    gun->online = online;
    if (!online) {
        gun->targets.clear();
        gun->cycle_progress = 0.0f;
    }
    return true;
}

bool GateGunSystem::isOnline(const std::string& entity_id) const {
    auto* gun = getComponentFor(entity_id);
    return gun ? gun->online : false;
}

float GateGunSystem::getDamageAtRange(const std::string& entity_id, float range) const {
    auto* gun = getComponentFor(entity_id);
    if (!gun) return 0.0f;
    if (range <= gun->optimal_range) return gun->damage_per_cycle;
    float beyond = range - gun->optimal_range;
    if (beyond >= gun->falloff_range || gun->falloff_range <= 0.0f) return 0.0f;
    float fraction = 1.0f - (beyond / gun->falloff_range);
    return gun->damage_per_cycle * fraction;
}

} // namespace systems
} // namespace atlas
