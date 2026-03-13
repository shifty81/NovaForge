#include "systems/agent_mission_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AgentMissionSystem::AgentMissionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AgentMissionSystem::updateComponent(ecs::Entity& /*entity*/,
                                          components::AgentMissionState& comp,
                                          float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& m : comp.missions) {
        if (m.status != components::AgentMissionState::MissionStatus::Active) continue;
        m.time_elapsed += delta_time;
        if (m.time_elapsed >= m.time_limit) {
            m.status = components::AgentMissionState::MissionStatus::Failed;
            comp.total_failed++;
        }
    }
}

bool AgentMissionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AgentMissionState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool AgentMissionSystem::offerMission(
        const std::string& entity_id,
        const std::string& mission_id,
        const std::string& mission_name,
        const std::string& agent_id,
        components::AgentMissionState::MissionType type,
        float isk_reward,
        int   lp_reward,
        float time_limit) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (mission_id.empty()) return false;
    if (mission_name.empty()) return false;
    if (agent_id.empty()) return false;
    if (isk_reward <= 0.0f) return false;
    if (lp_reward <= 0) return false;
    if (time_limit <= 0.0f) return false;
    if (static_cast<int>(comp->missions.size()) >= comp->max_missions) return false;

    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it != comp->missions.end()) return false;

    components::AgentMissionState::AgentMission mission;
    mission.mission_id   = mission_id;
    mission.mission_name = mission_name;
    mission.agent_id     = agent_id;
    mission.status       = components::AgentMissionState::MissionStatus::Offered;
    mission.type         = type;
    mission.isk_reward   = isk_reward;
    mission.lp_reward    = lp_reward;
    mission.time_limit   = time_limit;
    mission.offered_count = 1;
    comp->missions.push_back(mission);
    return true;
}

bool AgentMissionSystem::acceptMission(const std::string& entity_id,
                                        const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it == comp->missions.end()) return false;
    if (it->status != components::AgentMissionState::MissionStatus::Offered) return false;
    it->status = components::AgentMissionState::MissionStatus::Active;
    return true;
}

bool AgentMissionSystem::completeMission(const std::string& entity_id,
                                          const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it == comp->missions.end()) return false;
    if (it->status != components::AgentMissionState::MissionStatus::Active) return false;
    it->status = components::AgentMissionState::MissionStatus::Completed;
    comp->total_isk_earned += it->isk_reward;
    comp->total_lp_earned  += it->lp_reward;
    comp->total_completed++;
    return true;
}

bool AgentMissionSystem::failMission(const std::string& entity_id,
                                      const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it == comp->missions.end()) return false;
    if (it->status != components::AgentMissionState::MissionStatus::Active &&
        it->status != components::AgentMissionState::MissionStatus::Offered) return false;
    it->status = components::AgentMissionState::MissionStatus::Failed;
    comp->total_failed++;
    return true;
}

bool AgentMissionSystem::expireMission(const std::string& entity_id,
                                        const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it == comp->missions.end()) return false;
    if (it->status != components::AgentMissionState::MissionStatus::Offered) return false;
    it->status = components::AgentMissionState::MissionStatus::Expired;
    comp->total_expired++;
    return true;
}

bool AgentMissionSystem::removeMission(const std::string& entity_id,
                                        const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it == comp->missions.end()) return false;
    comp->missions.erase(it);
    return true;
}

bool AgentMissionSystem::clearMissions(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->missions.clear();
    return true;
}

int AgentMissionSystem::getMissionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->missions.size());
}

int AgentMissionSystem::getActiveMissionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->missions)
        if (m.status == components::AgentMissionState::MissionStatus::Active) count++;
    return count;
}

components::AgentMissionState::MissionStatus
AgentMissionSystem::getMissionStatus(const std::string& entity_id,
                                      const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::AgentMissionState::MissionStatus::Expired;
    auto it = std::find_if(comp->missions.begin(), comp->missions.end(),
                           [&](const components::AgentMissionState::AgentMission& m) {
                               return m.mission_id == mission_id;
                           });
    if (it == comp->missions.end())
        return components::AgentMissionState::MissionStatus::Expired;
    return it->status;
}

bool AgentMissionSystem::hasMission(const std::string& entity_id,
                                     const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return std::find_if(comp->missions.begin(), comp->missions.end(),
                        [&](const components::AgentMissionState::AgentMission& m) {
                            return m.mission_id == mission_id;
                        }) != comp->missions.end();
}

float AgentMissionSystem::getTotalIskEarned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_isk_earned;
}

int AgentMissionSystem::getTotalLpEarned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_lp_earned;
}

int AgentMissionSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_completed;
}

int AgentMissionSystem::getTotalFailed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_failed;
}

int AgentMissionSystem::getTotalExpired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_expired;
}

int AgentMissionSystem::getCountByType(
        const std::string& entity_id,
        components::AgentMissionState::MissionType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->missions)
        if (m.type == type) count++;
    return count;
}

} // namespace systems
} // namespace atlas
