#include "systems/fleet_squad_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FleetSquadSystem::FleetSquadSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void FleetSquadSystem::updateComponent(ecs::Entity& /*entity*/, components::FleetSquad& sq, float /*delta_time*/) {
    // Recalculate cohesion
    if (sq.is_active && !sq.member_ids.empty()) {
        sq.cohesion = 1.0f;
    } else {
        sq.cohesion = 0.0f;
    }

    // Recalculate effectiveness: 1.0 + 0.05 * (member_count - 1), clamped to [0.0, 2.0]
    int count = static_cast<int>(sq.member_ids.size());
    if (count == 0) {
        sq.effectiveness = 0.0f;
    } else {
        float eff = 1.0f + 0.05f * static_cast<float>(count - 1);
        sq.effectiveness = std::clamp(eff, 0.0f, 2.0f);
    }
}

bool FleetSquadSystem::createSquad(const std::string& entity_id, const std::string& squad_id,
                                    const std::string& leader_id, components::FleetSquad::SquadRole role) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::FleetSquad>();
    if (existing) return false;

    auto comp = std::make_unique<components::FleetSquad>();
    comp->squad_id = squad_id;
    comp->squad_leader_id = leader_id;
    comp->role = role;
    comp->member_ids.push_back(leader_id);
    comp->is_active = true;
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetSquadSystem::dissolveSquad(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    entity->removeComponent<components::FleetSquad>();
    return true;
}

bool FleetSquadSystem::addMember(const std::string& entity_id, const std::string& member_id) {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    if (static_cast<int>(sq->member_ids.size()) >= sq->max_members) return false;

    // Check for duplicate
    for (const auto& m : sq->member_ids) {
        if (m == member_id) return false;
    }

    sq->member_ids.push_back(member_id);
    return true;
}

bool FleetSquadSystem::removeMember(const std::string& entity_id, const std::string& member_id) {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    auto it = std::find(sq->member_ids.begin(), sq->member_ids.end(), member_id);
    if (it == sq->member_ids.end()) return false;

    sq->member_ids.erase(it);

    // If leader was removed, promote first remaining member
    if (member_id == sq->squad_leader_id && !sq->member_ids.empty()) {
        sq->squad_leader_id = sq->member_ids.front();
    }

    return true;
}

bool FleetSquadSystem::setFormation(const std::string& entity_id, components::FleetSquad::SquadFormation formation) {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    sq->formation = formation;
    return true;
}

bool FleetSquadSystem::setRole(const std::string& entity_id, components::FleetSquad::SquadRole role) {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    sq->role = role;
    return true;
}

bool FleetSquadSystem::setActive(const std::string& entity_id, bool active) {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    sq->is_active = active;
    return true;
}

int FleetSquadSystem::getMemberCount(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return 0;

    return static_cast<int>(sq->member_ids.size());
}

std::string FleetSquadSystem::getLeaderId(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return "";

    return sq->squad_leader_id;
}

std::string FleetSquadSystem::getRole(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return "";

    return components::FleetSquad::roleToString(sq->role);
}

std::string FleetSquadSystem::getFormation(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return "";

    return components::FleetSquad::formationToString(sq->formation);
}

float FleetSquadSystem::getCohesion(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return 0.0f;

    return sq->cohesion;
}

float FleetSquadSystem::getEffectiveness(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return 0.0f;

    return sq->effectiveness;
}

bool FleetSquadSystem::isSquadActive(const std::string& entity_id) const {
    auto* sq = getComponentFor(entity_id);
    if (!sq) return false;

    return sq->is_active;
}

} // namespace systems
} // namespace atlas
