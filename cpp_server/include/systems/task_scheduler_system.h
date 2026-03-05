#ifndef NOVAFORGE_SYSTEMS_TASK_SCHEDULER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TASK_SCHEDULER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Multi-threaded server task processing scheduler
 *
 * Manages queued tasks with priorities, dependencies, and concurrency limits.
 * Tasks progress through Queued -> Running -> Complete/Failed/Cancelled states.
 */
class TaskSchedulerSystem : public ecs::SingleComponentSystem<components::TaskScheduler> {
public:
    explicit TaskSchedulerSystem(ecs::World* world);
    ~TaskSchedulerSystem() override = default;

    std::string getName() const override { return "TaskSchedulerSystem"; }

    bool createScheduler(const std::string& entity_id);
    int addTask(const std::string& entity_id, const std::string& name, int priority);
    bool addDependency(const std::string& entity_id, int task_id, int depends_on_id);
    bool cancelTask(const std::string& entity_id, int task_id);
    int getTaskState(const std::string& entity_id, int task_id) const;
    float getTaskProgress(const std::string& entity_id, int task_id) const;
    int getRunningCount(const std::string& entity_id) const;
    int getQueuedCount(const std::string& entity_id) const;
    int getTotalCompleted(const std::string& entity_id) const;
    float getThroughput(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TaskScheduler& sched,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TASK_SCHEDULER_SYSTEM_H
