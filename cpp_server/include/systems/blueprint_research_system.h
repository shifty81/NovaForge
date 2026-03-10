#ifndef NOVAFORGE_SYSTEMS_BLUEPRINT_RESEARCH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BLUEPRINT_RESEARCH_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Blueprint Material / Time Efficiency research system
 *
 * Manages research jobs that improve blueprint ME or TE levels.
 * Each job takes a configurable amount of time scaled by facility
 * bonuses.  Completed jobs increment the blueprint's research level.
 */
class BlueprintResearchSystem : public ecs::SingleComponentSystem<components::BlueprintResearchState> {
public:
    explicit BlueprintResearchSystem(ecs::World* world);
    ~BlueprintResearchSystem() override = default;

    std::string getName() const override { return "BlueprintResearchSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& facility_id = "");
    bool startResearch(const std::string& entity_id, const std::string& blueprint_id,
                       components::BlueprintResearchState::ResearchType type,
                       int current_level, int target_level,
                       float time_required = 600.0f);
    bool cancelResearch(const std::string& entity_id, const std::string& blueprint_id);
    bool setResearchSpeed(const std::string& entity_id, float speed);

    int  getJobCount(const std::string& entity_id) const;
    int  getActiveJobCount(const std::string& entity_id) const;
    int  getTotalCompleted(const std::string& entity_id) const;
    int  getTotalCancelled(const std::string& entity_id) const;
    float getJobProgress(const std::string& entity_id,
                         const std::string& blueprint_id) const;
    bool isJobCompleted(const std::string& entity_id,
                        const std::string& blueprint_id) const;
    float getResearchSpeed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::BlueprintResearchState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BLUEPRINT_RESEARCH_SYSTEM_H
