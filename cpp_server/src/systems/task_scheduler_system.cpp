#include "systems/task_scheduler_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

TaskSchedulerSystem::TaskSchedulerSystem(ecs::World* world)
    : System(world) {
}

void TaskSchedulerSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::TaskScheduler>();
    for (auto* entity : entities) {
        auto* sched = entity->getComponent<components::TaskScheduler>();
        if (!sched || !sched->active) continue;

        sched->total_time += delta_time;

        // Mark tasks as Failed if a dependency is Failed or Cancelled
        for (auto& task : sched->tasks) {
            if (task.state == components::TaskScheduler::Queued) {
                for (int dep_id : task.dependencies) {
                    for (const auto& other : sched->tasks) {
                        if (other.id == dep_id &&
                            (other.state == components::TaskScheduler::Failed ||
                             other.state == components::TaskScheduler::Cancelled)) {
                            task.state = components::TaskScheduler::Failed;
                            sched->total_failed++;
                            break;
                        }
                    }
                    if (task.state == components::TaskScheduler::Failed) break;
                }
            }
        }

        // Count running tasks
        int running = 0;
        for (const auto& task : sched->tasks) {
            if (task.state == components::TaskScheduler::Running) running++;
        }

        // Start queued tasks whose dependencies are all Complete (higher priority first)
        // Collect startable tasks
        std::vector<int> startable;
        for (size_t i = 0; i < sched->tasks.size(); i++) {
            auto& task = sched->tasks[i];
            if (task.state != components::TaskScheduler::Queued) continue;
            bool deps_met = true;
            for (int dep_id : task.dependencies) {
                for (const auto& other : sched->tasks) {
                    if (other.id == dep_id && other.state != components::TaskScheduler::Complete) {
                        deps_met = false;
                        break;
                    }
                }
                if (!deps_met) break;
            }
            if (deps_met) startable.push_back(static_cast<int>(i));
        }

        // Sort by priority descending
        std::sort(startable.begin(), startable.end(), [&](int a, int b) {
            return sched->tasks[a].priority > sched->tasks[b].priority;
        });

        for (int idx : startable) {
            if (running >= sched->max_concurrent) break;
            sched->tasks[idx].state = components::TaskScheduler::Running;
            running++;
        }

        // Advance running tasks
        for (auto& task : sched->tasks) {
            if (task.state == components::TaskScheduler::Running) {
                task.progress += delta_time;
                if (task.progress >= 1.0f) {
                    task.progress = 1.0f;
                    task.state = components::TaskScheduler::Complete;
                    sched->total_completed++;
                }
            }
        }

        // Update throughput
        if (sched->total_time > 0.0f) {
            sched->throughput = static_cast<float>(sched->total_completed) / sched->total_time;
        }
    }
}

bool TaskSchedulerSystem::createScheduler(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->getComponent<components::TaskScheduler>()) return false;

    auto comp = std::make_unique<components::TaskScheduler>();
    entity->addComponent(std::move(comp));
    return true;
}

int TaskSchedulerSystem::addTask(const std::string& entity_id, const std::string& name, int priority) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return -1;

    components::TaskScheduler::TaskEntry task;
    task.id = sched->next_task_id++;
    task.name = name;
    task.priority = priority;
    task.state = components::TaskScheduler::Queued;
    task.progress = 0.0f;
    sched->tasks.push_back(task);
    return task.id;
}

bool TaskSchedulerSystem::addDependency(const std::string& entity_id, int task_id, int depends_on_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return false;

    for (auto& task : sched->tasks) {
        if (task.id == task_id) {
            task.dependencies.push_back(depends_on_id);
            return true;
        }
    }
    return false;
}

bool TaskSchedulerSystem::cancelTask(const std::string& entity_id, int task_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return false;

    for (auto& task : sched->tasks) {
        if (task.id == task_id && task.state == components::TaskScheduler::Queued) {
            task.state = components::TaskScheduler::Cancelled;
            return true;
        }
    }
    return false;
}

int TaskSchedulerSystem::getTaskState(const std::string& entity_id, int task_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return -1;

    for (const auto& task : sched->tasks) {
        if (task.id == task_id) return task.state;
    }
    return -1;
}

float TaskSchedulerSystem::getTaskProgress(const std::string& entity_id, int task_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return 0.0f;

    for (const auto& task : sched->tasks) {
        if (task.id == task_id) return task.progress;
    }
    return 0.0f;
}

int TaskSchedulerSystem::getRunningCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return 0;

    int count = 0;
    for (const auto& task : sched->tasks) {
        if (task.state == components::TaskScheduler::Running) count++;
    }
    return count;
}

int TaskSchedulerSystem::getQueuedCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return 0;

    int count = 0;
    for (const auto& task : sched->tasks) {
        if (task.state == components::TaskScheduler::Queued) count++;
    }
    return count;
}

int TaskSchedulerSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return 0;

    return sched->total_completed;
}

float TaskSchedulerSystem::getThroughput(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* sched = entity->getComponent<components::TaskScheduler>();
    if (!sched) return 0.0f;

    return sched->throughput;
}

} // namespace systems
} // namespace atlas
