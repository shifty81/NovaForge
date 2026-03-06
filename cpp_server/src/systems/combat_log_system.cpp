#include "systems/combat_log_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/combat_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using CL = components::CombatLog;
using Entry = components::CombatLog::CombatEntry;
using Engagement = components::CombatLog::EngagementSummary;

Engagement* findEngagement(CL* cl, const std::string& engagement_id) {
    for (auto& e : cl->engagements) {
        if (e.engagement_id == engagement_id) return &e;
    }
    return nullptr;
}

const Engagement* findEngagementConst(const CL* cl, const std::string& engagement_id) {
    for (const auto& e : cl->engagements) {
        if (e.engagement_id == engagement_id) return &e;
    }
    return nullptr;
}

} // anonymous namespace

CombatLogSystem::CombatLogSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CombatLogSystem::updateComponent(ecs::Entity& entity,
    components::CombatLog& cl, float delta_time) {
    if (!cl.active) return;

    cl.elapsed += delta_time;

    for (auto& eng : cl.engagements) {
        if (eng.outcome == CL::EngagementOutcome::Ongoing) {
            eng.duration += delta_time;
        }
    }
}

bool CombatLogSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CombatLog>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CombatLogSystem::logDamage(const std::string& entity_id,
    const std::string& attacker, const std::string& defender,
    int damage_type, float amount, const std::string& weapon, bool hit) {
    auto* cl = getComponentFor(entity_id);
    if (!cl) return false;

    if (static_cast<int>(cl->entries.size()) >= cl->max_entries) {
        cl->entries.erase(cl->entries.begin());
    }

    Entry entry;
    entry.attacker_id = attacker;
    entry.defender_id = defender;
    entry.damage_type = static_cast<CL::DamageType>(damage_type);
    entry.damage_amount = amount;
    entry.weapon_type = weapon;
    entry.hit = hit;
    entry.timestamp = cl->elapsed;
    cl->entries.push_back(entry);
    cl->total_entries_recorded++;

    if (hit) {
        cl->total_damage_dealt += amount;
    }
    return true;
}

bool CombatLogSystem::startEngagement(const std::string& entity_id,
    const std::string& engagement_id) {
    auto* cl = getComponentFor(entity_id);
    if (!cl) return false;
    if (static_cast<int>(cl->engagements.size()) >= cl->max_engagements) return false;
    if (findEngagement(cl, engagement_id)) return false;

    Engagement eng;
    eng.engagement_id = engagement_id;
    eng.start_time = cl->elapsed;
    cl->engagements.push_back(eng);
    return true;
}

bool CombatLogSystem::endEngagement(const std::string& entity_id,
    const std::string& engagement_id, int outcome) {
    auto* cl = getComponentFor(entity_id);
    if (!cl) return false;

    auto* eng = findEngagement(cl, engagement_id);
    if (!eng || eng->outcome != CL::EngagementOutcome::Ongoing) return false;

    eng->outcome = static_cast<CL::EngagementOutcome>(outcome);
    return true;
}

bool CombatLogSystem::recordKill(const std::string& entity_id,
    const std::string& engagement_id) {
    auto* cl = getComponentFor(entity_id);
    if (!cl) return false;

    auto* eng = findEngagement(cl, engagement_id);
    if (!eng) return false;

    eng->kills++;
    cl->total_kills++;
    return true;
}

bool CombatLogSystem::recordLoss(const std::string& entity_id,
    const std::string& engagement_id) {
    auto* cl = getComponentFor(entity_id);
    if (!cl) return false;

    auto* eng = findEngagement(cl, engagement_id);
    if (!eng) return false;

    eng->losses++;
    cl->total_losses++;
    return true;
}

int CombatLogSystem::getEntryCount(const std::string& entity_id) const {
    auto* cl = getComponentFor(entity_id);
    return cl ? static_cast<int>(cl->entries.size()) : 0;
}

int CombatLogSystem::getEngagementCount(const std::string& entity_id) const {
    auto* cl = getComponentFor(entity_id);
    return cl ? static_cast<int>(cl->engagements.size()) : 0;
}

float CombatLogSystem::getTotalDamageDealt(const std::string& entity_id) const {
    auto* cl = getComponentFor(entity_id);
    return cl ? cl->total_damage_dealt : 0.0f;
}

float CombatLogSystem::getTotalDamageReceived(const std::string& entity_id) const {
    auto* cl = getComponentFor(entity_id);
    return cl ? cl->total_damage_received : 0.0f;
}

float CombatLogSystem::getAverageDPS(const std::string& entity_id,
    const std::string& engagement_id) const {
    auto* cl = getComponentFor(entity_id);
    if (!cl) return 0.0f;

    const auto* eng = findEngagementConst(cl, engagement_id);
    if (!eng || eng->duration <= 0.0f) return 0.0f;

    return eng->total_damage_dealt / eng->duration;
}

int CombatLogSystem::getTotalKills(const std::string& entity_id) const {
    auto* cl = getComponentFor(entity_id);
    return cl ? cl->total_kills : 0;
}

int CombatLogSystem::getTotalLosses(const std::string& entity_id) const {
    auto* cl = getComponentFor(entity_id);
    return cl ? cl->total_losses : 0;
}

} // namespace systems
} // namespace atlas
