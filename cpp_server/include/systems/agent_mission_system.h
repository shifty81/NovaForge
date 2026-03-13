#ifndef NOVAFORGE_SYSTEMS_AGENT_MISSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_AGENT_MISSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/mission_components.h"
#include <string>

namespace atlas {
namespace systems {

class AgentMissionSystem
    : public ecs::SingleComponentSystem<components::AgentMissionState> {
public:
    explicit AgentMissionSystem(ecs::World* world);
    ~AgentMissionSystem() override = default;

    std::string getName() const override { return "AgentMissionSystem"; }

    bool initialize(const std::string& entity_id);

    bool offerMission(const std::string& entity_id,
                      const std::string& mission_id,
                      const std::string& mission_name,
                      const std::string& agent_id,
                      components::AgentMissionState::MissionType type,
                      float isk_reward,
                      int   lp_reward,
                      float time_limit);
    bool acceptMission(const std::string& entity_id, const std::string& mission_id);
    bool completeMission(const std::string& entity_id, const std::string& mission_id);
    bool failMission(const std::string& entity_id, const std::string& mission_id);
    bool expireMission(const std::string& entity_id, const std::string& mission_id);
    bool removeMission(const std::string& entity_id, const std::string& mission_id);
    bool clearMissions(const std::string& entity_id);

    int    getMissionCount(const std::string& entity_id) const;
    int    getActiveMissionCount(const std::string& entity_id) const;
    components::AgentMissionState::MissionStatus
           getMissionStatus(const std::string& entity_id,
                            const std::string& mission_id) const;
    bool   hasMission(const std::string& entity_id, const std::string& mission_id) const;
    float  getTotalIskEarned(const std::string& entity_id) const;
    int    getTotalLpEarned(const std::string& entity_id) const;
    int    getTotalCompleted(const std::string& entity_id) const;
    int    getTotalFailed(const std::string& entity_id) const;
    int    getTotalExpired(const std::string& entity_id) const;
    int    getCountByType(const std::string& entity_id,
                          components::AgentMissionState::MissionType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AgentMissionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_AGENT_MISSION_SYSTEM_H
