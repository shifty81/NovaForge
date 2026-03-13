#ifndef NOVAFORGE_SYSTEMS_REPAIR_TIMER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_REPAIR_TIMER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

class RepairTimerSystem
    : public ecs::SingleComponentSystem<components::RepairTimerState> {
public:
    explicit RepairTimerSystem(ecs::World* world);
    ~RepairTimerSystem() override = default;

    std::string getName() const override { return "RepairTimerSystem"; }

    bool initialize(const std::string& entity_id);

    bool addJob(const std::string& entity_id,
                const std::string& job_id,
                components::RepairTimerState::RepairLayer layer,
                float amount_per_tick,
                int   ticks_remaining);
    bool cancelJob(const std::string& entity_id, const std::string& job_id);
    bool clearJobs(const std::string& entity_id);

    int   getJobCount(const std::string& entity_id) const;
    int   getActiveJobCount(const std::string& entity_id) const;
    bool  isJobActive(const std::string& entity_id, const std::string& job_id) const;
    bool  isJobComplete(const std::string& entity_id, const std::string& job_id) const;
    bool  hasJob(const std::string& entity_id, const std::string& job_id) const;
    int   getTicksRemaining(const std::string& entity_id, const std::string& job_id) const;
    float getTotalRepaired(const std::string& entity_id) const;
    int   getTotalJobsStarted(const std::string& entity_id) const;
    int   getTotalJobsCompleted(const std::string& entity_id) const;
    float getRepairByLayer(const std::string& entity_id,
                           components::RepairTimerState::RepairLayer layer) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RepairTimerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_REPAIR_TIMER_SYSTEM_H
