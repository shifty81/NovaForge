#ifndef NOVAFORGE_SYSTEMS_INVENTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INVENTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tech II blueprint invention system
 *
 * Manages invention jobs that attempt to create T2 blueprint copies
 * from T1 blueprints.  Each job has a configurable base chance of
 * success, modified by skills.  Failed attempts consume data-cores
 * but produce no blueprint.  Progress is accumulated each tick.
 */
class InventionSystem : public ecs::SingleComponentSystem<components::InventionState> {
public:
    explicit InventionSystem(ecs::World* world);
    ~InventionSystem() override = default;

    std::string getName() const override { return "InventionSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& facility_id = "");
    bool startJob(const std::string& entity_id, const std::string& job_id,
                  const std::string& t1_blueprint_id,
                  const std::string& datacore_1, const std::string& datacore_2,
                  float base_chance = 0.25f, float time_required = 900.0f);
    bool cancelJob(const std::string& entity_id, const std::string& job_id);
    bool setResearchSpeed(const std::string& entity_id, float speed);

    int  getJobCount(const std::string& entity_id) const;
    int  getActiveJobCount(const std::string& entity_id) const;
    int  getTotalAttempted(const std::string& entity_id) const;
    int  getTotalSucceeded(const std::string& entity_id) const;
    int  getTotalFailed(const std::string& entity_id) const;
    int  getTotalCancelled(const std::string& entity_id) const;
    float getJobProgress(const std::string& entity_id,
                         const std::string& job_id) const;
    bool isJobCompleted(const std::string& entity_id,
                        const std::string& job_id) const;
    float getResearchSpeed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::InventionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INVENTION_SYSTEM_H
