#include "systems/player_progression_tracking_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using PPT = components::PlayerProgressionTracking;
using Milestone = components::PlayerProgressionTracking::Milestone;

Milestone* findMilestone(PPT* ppt, const std::string& milestone_id) {
    for (auto& m : ppt->milestones) {
        if (m.milestone_id == milestone_id) return &m;
    }
    return nullptr;
}

const Milestone* findMilestoneConst(const PPT* ppt, const std::string& milestone_id) {
    for (const auto& m : ppt->milestones) {
        if (m.milestone_id == milestone_id) return &m;
    }
    return nullptr;
}

} // anonymous namespace

PlayerProgressionTrackingSystem::PlayerProgressionTrackingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlayerProgressionTrackingSystem::updateComponent(ecs::Entity& entity,
    components::PlayerProgressionTracking& ppt, float delta_time) {
    if (!ppt.active) return;
    ppt.play_time += delta_time;
}

bool PlayerProgressionTrackingSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PlayerProgressionTracking>();
    entity->addComponent(std::move(comp));
    return true;
}

bool PlayerProgressionTrackingSystem::recordMilestone(const std::string& entity_id,
    const std::string& milestone_id, const std::string& description) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt) return false;
    if (static_cast<int>(ppt->milestones.size()) >= ppt->max_milestones) return false;
    if (findMilestone(ppt, milestone_id)) return false;

    Milestone m;
    m.milestone_id = milestone_id;
    m.description = description;
    m.timestamp = ppt->play_time;
    ppt->milestones.push_back(m);
    return true;
}

bool PlayerProgressionTrackingSystem::addIscEarned(const std::string& entity_id, float amount) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt || amount <= 0.0f) return false;
    ppt->isc_earned += amount;
    return true;
}

bool PlayerProgressionTrackingSystem::addIscSpent(const std::string& entity_id, float amount) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt || amount <= 0.0f) return false;
    ppt->isc_spent += amount;
    return true;
}

bool PlayerProgressionTrackingSystem::recordKill(const std::string& entity_id) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt) return false;
    ppt->kills++;
    return true;
}

bool PlayerProgressionTrackingSystem::recordDeath(const std::string& entity_id) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt) return false;
    ppt->deaths++;
    return true;
}

bool PlayerProgressionTrackingSystem::recordDock(const std::string& entity_id) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt) return false;
    ppt->docks++;
    return true;
}

bool PlayerProgressionTrackingSystem::recordJump(const std::string& entity_id) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt) return false;
    ppt->jumps++;
    return true;
}

bool PlayerProgressionTrackingSystem::addMiningYield(const std::string& entity_id, float ore_units) {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt || ore_units <= 0.0f) return false;
    ppt->mining_yield += ore_units;
    return true;
}

int PlayerProgressionTrackingSystem::getMilestoneCount(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? static_cast<int>(ppt->milestones.size()) : 0;
}

bool PlayerProgressionTrackingSystem::hasMilestone(const std::string& entity_id,
    const std::string& milestone_id) const {
    auto* ppt = getComponentFor(entity_id);
    if (!ppt) return false;
    return findMilestoneConst(ppt, milestone_id) != nullptr;
}

float PlayerProgressionTrackingSystem::getIscEarned(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->isc_earned : 0.0f;
}

float PlayerProgressionTrackingSystem::getIscSpent(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->isc_spent : 0.0f;
}

int PlayerProgressionTrackingSystem::getKills(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->kills : 0;
}

int PlayerProgressionTrackingSystem::getDeaths(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->deaths : 0;
}

int PlayerProgressionTrackingSystem::getDocks(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->docks : 0;
}

int PlayerProgressionTrackingSystem::getJumps(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->jumps : 0;
}

float PlayerProgressionTrackingSystem::getMiningYield(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->mining_yield : 0.0f;
}

float PlayerProgressionTrackingSystem::getPlayTime(const std::string& entity_id) const {
    auto* ppt = getComponentFor(entity_id);
    return ppt ? ppt->play_time : 0.0f;
}

} // namespace systems
} // namespace atlas
