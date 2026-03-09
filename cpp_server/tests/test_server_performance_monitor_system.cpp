// Tests for: ServerPerformanceMonitorSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/server_performance_monitor_system.h"

using namespace atlas;

// ==================== ServerPerformanceMonitorSystem Tests ====================

static void testPerfMonitorInitialize() {
    std::cout << "\n=== PerfMonitor: Initialize ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");

    assertTrue(sys.initializeMonitor("mon1", "server_main", 50.0f), "Initialize monitor");
    assertTrue(approxEqual(sys.getAverageTickTime("mon1"), 0.0f), "Avg tick time is 0");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon1"), 0.0f), "Budget utilization is 0");
    assertTrue(sys.getSlowestSystem("mon1").empty(), "No slowest system");
    assertTrue(sys.getHotPathCount("mon1") == 0, "No hot paths");
    assertTrue(!sys.isAlertActive("mon1"), "No alert active");
}

static void testPerfMonitorDuplicateInitRejected() {
    std::cout << "\n=== PerfMonitor: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");

    assertTrue(sys.initializeMonitor("mon1", "server_main", 50.0f), "First init ok");
    assertTrue(!sys.initializeMonitor("mon1", "server_2", 25.0f), "Duplicate init rejected");
}

static void testPerfMonitorRecordSystemTiming() {
    std::cout << "\n=== PerfMonitor: RecordSystemTiming ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    assertTrue(sys.recordSystemTiming("mon1", "MovementSystem", 5.0f), "Record timing");
    assertTrue(sys.recordSystemTiming("mon1", "CombatSystem", 8.0f), "Record another timing");

    // After update, CombatSystem should be slowest
    sys.update(0.016f);
    assertTrue(sys.getSlowestSystem("mon1") == "CombatSystem", "CombatSystem is slowest");
}

static void testPerfMonitorRecordMultipleSamples() {
    std::cout << "\n=== PerfMonitor: RecordMultipleSamples ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    sys.recordSystemTiming("mon1", "MovementSystem", 5.0f);
    sys.recordSystemTiming("mon1", "MovementSystem", 10.0f);
    sys.recordSystemTiming("mon1", "MovementSystem", 15.0f);

    sys.update(0.016f);
    assertTrue(sys.getSlowestSystem("mon1") == "MovementSystem", "Movement is tracked");
}

static void testPerfMonitorRecordTickComplete() {
    std::cout << "\n=== PerfMonitor: RecordTickComplete ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    assertTrue(sys.recordTickComplete("mon1", 30.0f, 100), "Record tick complete");
    assertTrue(approxEqual(sys.getAverageTickTime("mon1"), 30.0f), "Avg tick is 30ms");

    assertTrue(sys.recordTickComplete("mon1", 40.0f, 150), "Record another tick");
    assertTrue(sys.getAverageTickTime("mon1") > 30.0f, "Avg tick increased");
}

static void testPerfMonitorBudgetUtilization() {
    std::cout << "\n=== PerfMonitor: BudgetUtilization ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    // 25ms tick on 50ms budget = 50% utilization
    sys.recordTickComplete("mon1", 25.0f, 100);
    assertTrue(approxEqual(sys.getBudgetUtilization("mon1"), 0.5f), "Budget utilization is 50%");
}

static void testPerfMonitorAlertActive() {
    std::cout << "\n=== PerfMonitor: AlertActive ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    // 45ms on 50ms budget = 90% > 80% threshold
    sys.recordTickComplete("mon1", 45.0f, 200);
    sys.update(0.016f);
    assertTrue(sys.isAlertActive("mon1"), "Alert active at 90% utilization");
}

static void testPerfMonitorNoAlertUnderThreshold() {
    std::cout << "\n=== PerfMonitor: NoAlertUnderThreshold ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    // 20ms on 50ms budget = 40% < 80% threshold
    sys.recordTickComplete("mon1", 20.0f, 100);
    sys.update(0.016f);
    assertTrue(!sys.isAlertActive("mon1"), "No alert at 40% utilization");
}

static void testPerfMonitorHotPaths() {
    std::cout << "\n=== PerfMonitor: HotPaths ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    // Hot path threshold = budget * 0.25 = 12.5ms
    sys.recordSystemTiming("mon1", "MovementSystem", 5.0f);   // Not hot
    sys.recordSystemTiming("mon1", "CombatSystem", 15.0f);    // Hot
    sys.recordSystemTiming("mon1", "PhysicsSystem", 20.0f);   // Hot

    sys.update(0.016f);
    assertTrue(sys.getHotPathCount("mon1") == 2, "2 hot paths");
}

static void testPerfMonitorResetMetrics() {
    std::cout << "\n=== PerfMonitor: ResetMetrics ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    sys.recordSystemTiming("mon1", "MovementSystem", 10.0f);
    sys.recordTickComplete("mon1", 30.0f, 100);

    assertTrue(sys.resetMetrics("mon1"), "Reset metrics");
    assertTrue(approxEqual(sys.getAverageTickTime("mon1"), 0.0f), "Avg tick reset to 0");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon1"), 0.0f), "Utilization reset to 0");
    assertTrue(!sys.isAlertActive("mon1"), "Alert inactive after reset");
    assertTrue(sys.getHotPathCount("mon1") == 0, "Hot paths reset to 0");
    assertTrue(sys.getSlowestSystem("mon1").empty(), "Slowest system cleared");
}

static void testPerfMonitorTicksOverBudget() {
    std::cout << "\n=== PerfMonitor: TicksOverBudget ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    auto* e = world.createEntity("mon1");
    sys.initializeMonitor("mon1", "server_main", 50.0f);

    sys.recordTickComplete("mon1", 30.0f, 100);  // Under budget
    sys.recordTickComplete("mon1", 60.0f, 100);  // Over budget
    sys.recordTickComplete("mon1", 55.0f, 100);  // Over budget

    auto* metrics = e->getComponent<components::ServerPerformanceMetrics>();
    assertTrue(metrics->ticks_over_budget == 2, "2 ticks over budget");
    assertTrue(metrics->total_ticks == 3, "3 total ticks");
}

static void testPerfMonitorMissingEntity() {
    std::cout << "\n=== PerfMonitor: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);

    assertTrue(!sys.initializeMonitor("ghost", "s1", 50.0f), "Init fails for missing entity");
    assertTrue(!sys.recordSystemTiming("ghost", "sys", 10.0f), "recordSystemTiming fails for missing");
    assertTrue(!sys.recordTickComplete("ghost", 30.0f, 100), "recordTickComplete fails for missing");
    assertTrue(approxEqual(sys.getAverageTickTime("ghost"), 0.0f), "Avg 0 for missing");
    assertTrue(approxEqual(sys.getBudgetUtilization("ghost"), 0.0f), "Util 0 for missing");
    assertTrue(sys.getSlowestSystem("ghost").empty(), "Slowest empty for missing");
    assertTrue(sys.getHotPathCount("ghost") == 0, "Hot paths 0 for missing");
    assertTrue(!sys.isAlertActive("ghost"), "Alert false for missing");
    assertTrue(!sys.resetMetrics("ghost"), "Reset fails for missing");
}

void run_server_performance_monitor_system_tests() {
    testPerfMonitorInitialize();
    testPerfMonitorDuplicateInitRejected();
    testPerfMonitorRecordSystemTiming();
    testPerfMonitorRecordMultipleSamples();
    testPerfMonitorRecordTickComplete();
    testPerfMonitorBudgetUtilization();
    testPerfMonitorAlertActive();
    testPerfMonitorNoAlertUnderThreshold();
    testPerfMonitorHotPaths();
    testPerfMonitorResetMetrics();
    testPerfMonitorTicksOverBudget();
    testPerfMonitorMissingEntity();
}
