#include "systems/aegis_spawn_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AegisSpawnSystem::AegisSpawnSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AegisSpawnSystem::updateComponent(ecs::Entity& entity,
    components::AegisSpawnState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& sq : comp.squads) {
        switch (sq.phase) {
            case components::AegisSpawnState::SpawnPhase::Dispatching:
                sq.warp_eta -= delta_time;
                if (sq.warp_eta <= 0.0f) {
                    sq.warp_eta = 0.0f;
                    sq.phase = components::AegisSpawnState::SpawnPhase::Warping;
                }
                break;
            case components::AegisSpawnState::SpawnPhase::Warping:
                sq.phase = components::AegisSpawnState::SpawnPhase::Engaged;
                break;
            case components::AegisSpawnState::SpawnPhase::Engaged:
                sq.engagement_time += delta_time;
                if (sq.engagement_time >= sq.max_engagement) {
                    sq.phase = components::AegisSpawnState::SpawnPhase::Withdrawing;
                }
                break;
            case components::AegisSpawnState::SpawnPhase::Withdrawing:
                break;
            default:
                break;
        }
    }

    // Remove withdrawn squads
    comp.squads.erase(
        std::remove_if(comp.squads.begin(), comp.squads.end(),
            [](const components::AegisSpawnState::DispatchedSquad& s) {
                return s.phase == components::AegisSpawnState::SpawnPhase::Withdrawing;
            }),
        comp.squads.end());
}

bool AegisSpawnSystem::initialize(const std::string& entity_id,
    const std::string& system_id, float security_level) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AegisSpawnState>();
    comp->system_id = system_id;
    comp->security_level = security_level;
    entity->addComponent(std::move(comp));
    return true;
}

bool AegisSpawnSystem::reportCriminal(const std::string& entity_id,
    const std::string& criminal_id, int ship_count, float dps_per_ship) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->squads.size()) >= comp->max_squads) return false;

    components::AegisSpawnState::DispatchedSquad sq;
    sq.squad_id = "aegis_" + std::to_string(comp->total_dispatched);
    sq.target_id = criminal_id;
    sq.ship_count = ship_count;
    sq.dps_per_ship = dps_per_ship;
    sq.warp_eta = comp->responseTimeForSecurity();
    sq.phase = components::AegisSpawnState::SpawnPhase::Dispatching;
    comp->squads.push_back(sq);
    comp->total_dispatched++;
    return true;
}

bool AegisSpawnSystem::withdrawSquad(const std::string& entity_id,
    const std::string& squad_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& sq : comp->squads) {
        if (sq.squad_id == squad_id) {
            sq.phase = components::AegisSpawnState::SpawnPhase::Withdrawing;
            return true;
        }
    }
    return false;
}

bool AegisSpawnSystem::setSecurityLevel(const std::string& entity_id, float level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->security_level = level;
    return true;
}

int AegisSpawnSystem::getSquadCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->squads.size()) : 0;
}

int AegisSpawnSystem::getActiveSquadCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& sq : comp->squads) {
        if (sq.phase == components::AegisSpawnState::SpawnPhase::Engaged) count++;
    }
    return count;
}

int AegisSpawnSystem::getTotalDispatched(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_dispatched : 0;
}

int AegisSpawnSystem::getTotalKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_kills : 0;
}

float AegisSpawnSystem::getSecurityLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->security_level : 0.0f;
}

float AegisSpawnSystem::getResponseTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->responseTimeForSecurity() : 0.0f;
}

} // namespace systems
} // namespace atlas
