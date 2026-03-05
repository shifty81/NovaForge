#ifndef NOVAFORGE_SYSTEMS_PROCEDURAL_MISSION_GENERATOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PROCEDURAL_MISSION_GENERATOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/mission_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Procedural mission generation with template-based objectives
 *
 * Generates missions from templates with scaling difficulty, reward calculation
 * based on difficulty and faction standing, accept/complete/expire lifecycle,
 * and automatic generation on a timer.
 */
class ProceduralMissionGeneratorSystem : public ecs::SingleComponentSystem<components::ProceduralMissionGenerator> {
public:
    explicit ProceduralMissionGeneratorSystem(ecs::World* world);
    ~ProceduralMissionGeneratorSystem() override = default;

    std::string getName() const override { return "ProceduralMissionGeneratorSystem"; }

    bool initialize(const std::string& entity_id, const std::string& generator_id,
                    const std::string& faction_id);
    bool generateMission(const std::string& entity_id, const std::string& mission_id,
                         const std::string& type, int difficulty,
                         const std::string& target_system);
    bool acceptMission(const std::string& entity_id, const std::string& mission_id);
    bool completeMission(const std::string& entity_id, const std::string& mission_id);
    bool expireMission(const std::string& entity_id, const std::string& mission_id);
    bool removeMission(const std::string& entity_id, const std::string& mission_id);
    int getAvailableCount(const std::string& entity_id) const;
    int getCompletedCount(const std::string& entity_id) const;
    float getMissionReward(const std::string& entity_id, const std::string& mission_id) const;
    int getMissionDifficulty(const std::string& entity_id, const std::string& mission_id) const;
    bool isMissionAccepted(const std::string& entity_id, const std::string& mission_id) const;
    int getTotalGenerated(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ProceduralMissionGenerator& gen, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PROCEDURAL_MISSION_GENERATOR_SYSTEM_H
