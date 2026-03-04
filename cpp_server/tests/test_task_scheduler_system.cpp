// Tests for: TaskSchedulerSystem Tests
#include "test_log.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/task_scheduler_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== TaskSchedulerSystem Tests ====================

static void testTaskSchedulerCreate() {
    std::cout << "\n=== TaskScheduler: Create ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    assertTrue(sys.createScheduler("sched1"), "Create scheduler succeeds");
    auto* s = e->getComponent<components::TaskScheduler>();
    assertTrue(s != nullptr, "Component exists");
    assertTrue(s->max_concurrent == 4, "Default max_concurrent is 4");
    assertTrue(s->total_completed == 0, "Default total_completed is 0");
    assertTrue(s->active, "Scheduler is active by default");
}

static void testTaskSchedulerAddTask() {
    std::cout << "\n=== TaskScheduler: AddTask ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int id = sys.addTask("sched1", "build", 1);
    assertTrue(id > 0, "Task ID is positive");
    assertTrue(sys.getTaskState("sched1", id) == 0, "Task state is Queued (0)");
    assertTrue(sys.getQueuedCount("sched1") == 1, "Queued count is 1");
}

static void testTaskSchedulerRunning() {
    std::cout << "\n=== TaskScheduler: Running ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int id = sys.addTask("sched1", "compile", 1);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", id) == 1, "Task state is Running (1)");
    assertTrue(sys.getRunningCount("sched1") == 1, "Running count is 1");
    assertTrue(sys.getTaskProgress("sched1", id) > 0.0f, "Progress advanced");
}

static void testTaskSchedulerComplete() {
    std::cout << "\n=== TaskScheduler: Complete ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int id = sys.addTask("sched1", "link", 1);
    sys.update(0.5f);
    sys.update(0.6f);
    assertTrue(sys.getTaskState("sched1", id) == 2, "Task state is Complete (2)");
    assertTrue(sys.getTotalCompleted("sched1") == 1, "Total completed is 1");
    assertTrue(approxEqual(sys.getTaskProgress("sched1", id), 1.0f), "Progress is 1.0");
}

static void testTaskSchedulerPriority() {
    std::cout << "\n=== TaskScheduler: Priority ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    sys.createScheduler("sched1");
    auto* s = e->getComponent<components::TaskScheduler>();
    s->max_concurrent = 1;
    int low_id = sys.addTask("sched1", "low_task", 0);
    int high_id = sys.addTask("sched1", "high_task", 3);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", high_id) == 1, "High priority task is Running");
    assertTrue(sys.getTaskState("sched1", low_id) == 0, "Low priority task still Queued");
    assertTrue(sys.getRunningCount("sched1") == 1, "Only 1 running");
}

static void testTaskSchedulerMaxConcurrent() {
    std::cout << "\n=== TaskScheduler: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    sys.createScheduler("sched1");
    auto* s = e->getComponent<components::TaskScheduler>();
    s->max_concurrent = 2;
    sys.addTask("sched1", "t1", 1);
    sys.addTask("sched1", "t2", 1);
    sys.addTask("sched1", "t3", 1);
    sys.update(0.1f);
    assertTrue(sys.getRunningCount("sched1") == 2, "Only 2 tasks running");
    assertTrue(sys.getQueuedCount("sched1") == 1, "1 task still queued");
    assertTrue(sys.getTaskProgress("sched1", 3) == 0.0f, "Third task has no progress");
}

static void testTaskSchedulerDependency() {
    std::cout << "\n=== TaskScheduler: Dependency ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int t1 = sys.addTask("sched1", "first", 1);
    int t2 = sys.addTask("sched1", "second", 1);
    sys.addDependency("sched1", t2, t1);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", t1) == 1, "First task is Running");
    assertTrue(sys.getTaskState("sched1", t2) == 0, "Second task still Queued (dependency)");
    // Complete first task
    sys.update(1.0f);
    // Next tick: t2's dependency is met, it can start
    sys.update(0.01f);
    assertTrue(sys.getTaskState("sched1", t2) == 1 || sys.getTaskState("sched1", t2) == 2,
               "Second task started after dependency complete");
}

static void testTaskSchedulerCancel() {
    std::cout << "\n=== TaskScheduler: Cancel ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    sys.createScheduler("sched1");
    auto* s = e->getComponent<components::TaskScheduler>();
    s->max_concurrent = 1;
    sys.addTask("sched1", "blocker", 2);
    int t2 = sys.addTask("sched1", "cancel_me", 0);
    assertTrue(sys.cancelTask("sched1", t2), "Cancel queued task succeeds");
    assertTrue(sys.getTaskState("sched1", t2) == 4, "Cancelled task state is 4");
    assertTrue(!sys.cancelTask("sched1", t2), "Cannot cancel already cancelled task");
}

static void testTaskSchedulerFailedDep() {
    std::cout << "\n=== TaskScheduler: FailedDep ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int t1 = sys.addTask("sched1", "will_cancel", 1);
    int t2 = sys.addTask("sched1", "depends_on_cancelled", 1);
    sys.addDependency("sched1", t2, t1);
    sys.cancelTask("sched1", t1);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", t2) == 3, "Dependent task Failed");
    assertTrue(sys.getTaskState("sched1", t1) == 4, "Original task still Cancelled");
    assertTrue(sys.getRunningCount("sched1") == 0, "No running tasks");
}

static void testTaskSchedulerMissing() {
    std::cout << "\n=== TaskScheduler: Missing ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    assertTrue(!sys.createScheduler("nonexistent"), "Create fails on missing entity");
    assertTrue(sys.addTask("nonexistent", "t", 1) == -1, "addTask returns -1 on missing");
    assertTrue(sys.getTaskState("nonexistent", 1) == -1, "getTaskState returns -1 on missing");
    assertTrue(sys.getRunningCount("nonexistent") == 0, "0 running on missing");
    assertTrue(sys.getQueuedCount("nonexistent") == 0, "0 queued on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getThroughput("nonexistent"), 0.0f), "0 throughput on missing");
    assertTrue(approxEqual(sys.getTaskProgress("nonexistent", 1), 0.0f), "0 progress on missing");
    assertTrue(!sys.cancelTask("nonexistent", 1), "Cancel fails on missing");
}


void run_task_scheduler_system_tests() {
    testTaskSchedulerCreate();
    testTaskSchedulerAddTask();
    testTaskSchedulerRunning();
    testTaskSchedulerComplete();
    testTaskSchedulerPriority();
    testTaskSchedulerMaxConcurrent();
    testTaskSchedulerDependency();
    testTaskSchedulerCancel();
    testTaskSchedulerFailedDep();
    testTaskSchedulerMissing();
}
