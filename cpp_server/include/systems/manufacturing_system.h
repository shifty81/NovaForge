#ifndef NOVAFORGE_SYSTEMS_MANUFACTURING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MANUFACTURING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manufacturing system for blueprint-based production
 *
 * Manages manufacturing jobs: starting jobs, ticking time,
 * completing runs, and delivering output.
 */
class ManufacturingSystem : public ecs::SingleComponentSystem<components::ManufacturingFacility> {
public:
    explicit ManufacturingSystem(ecs::World* world);
    ~ManufacturingSystem() override = default;

    std::string getName() const override { return "ManufacturingSystem"; }

    /**
     * @brief Start a manufacturing job at a facility
     * @return job_id or empty string on failure
     */
    std::string startJob(const std::string& facility_entity_id,
                         const std::string& owner_id,
                         const std::string& blueprint_id,
                         const std::string& output_item_id,
                         const std::string& output_item_name,
                         int runs,
                         float time_per_run,
                         double install_cost);

    /**
     * @brief Cancel a manufacturing job
     * @return true if cancelled
     */
    bool cancelJob(const std::string& facility_entity_id,
                   const std::string& job_id);

    /**
     * @brief Get the number of active jobs at a facility
     */
    int getActiveJobCount(const std::string& facility_entity_id);

    /**
     * @brief Get the number of completed jobs at a facility
     */
    int getCompletedJobCount(const std::string& facility_entity_id);

    /**
     * @brief Get the total number of runs completed across all jobs
     */
    int getTotalRunsCompleted(const std::string& facility_entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::ManufacturingFacility& facility, float delta_time) override;

private:
    int job_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MANUFACTURING_SYSTEM_H
