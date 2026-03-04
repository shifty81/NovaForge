// Tests for: ServerPerformanceMonitor Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/ai_system.h"
#include "systems/server_performance_monitor_system.h"

using namespace atlas;

// ==================== ServerPerformanceMonitor Tests ====================

static void testPerfMonitorInit() {
    std::cout << "\n=== PerfMonitor: Init ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    assertTrue(sys.initializeMonitor("mon_1", "server_1", 50.0f), "Monitor initialized");
    assertTrue(approxEqual(sys.getAverageTickTime("mon_1"), 0.0f), "Avg tick 0 initially");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon_1"), 0.0f), "Budget 0 initially");
    assertTrue(!sys.isAlertActive("mon_1"), "No alert initially");
    assertTrue(!sys.initializeMonitor("mon_1", "server_1", 50.0f), "Duplicate init fails");
}

static void testPerfMonitorRecordTiming() {
    std::cout << "\n=== PerfMonitor: Record Timing ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    assertTrue(sys.recordSystemTiming("mon_1", "PhysicsSystem", 10.0f), "Timing recorded");
    assertTrue(sys.recordSystemTiming("mon_1", "RenderSystem", 5.0f), "Second timing recorded");
    sys.update(0.016f);
    assertTrue(sys.getSlowestSystem("mon_1") == "PhysicsSystem", "Physics is slowest");
}

static void testPerfMonitorTickComplete() {
    std::cout << "\n=== PerfMonitor: Tick Complete ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    assertTrue(sys.recordTickComplete("mon_1", 30.0f, 100), "Tick recorded");
    assertTrue(approxEqual(sys.getAverageTickTime("mon_1"), 30.0f), "Avg is 30ms");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon_1"), 0.6f), "Budget util is 0.6");
}

static void testPerfMonitorAverage() {
    std::cout << "\n=== PerfMonitor: Average ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 20.0f, 100);
    sys.recordTickComplete("mon_1", 40.0f, 100);
    float avg = sys.getAverageTickTime("mon_1");
    assertTrue(approxEqual(avg, 30.0f), "Average of 20 and 40 is 30");
}

static void testPerfMonitorBudget() {
    std::cout << "\n=== PerfMonitor: Budget ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 25.0f, 50);
    float util = sys.getBudgetUtilization("mon_1");
    assertTrue(approxEqual(util, 0.5f), "25/50 = 50% utilization");
}

static void testPerfMonitorAlert() {
    std::cout << "\n=== PerfMonitor: Alert ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 45.0f, 100);
    sys.update(0.016f);
    assertTrue(sys.isAlertActive("mon_1"), "Alert active at 90% budget");
    sys.resetMetrics("mon_1");
    sys.recordTickComplete("mon_1", 10.0f, 100);
    sys.update(0.016f);
    assertTrue(!sys.isAlertActive("mon_1"), "No alert at 20% budget");
}

static void testPerfMonitorHotPath() {
    std::cout << "\n=== PerfMonitor: Hot Path ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    // Hot path threshold: 25% of 50ms = 12.5ms
    sys.recordSystemTiming("mon_1", "PhysicsSystem", 15.0f);
    sys.recordSystemTiming("mon_1", "AISystem", 14.0f);
    sys.recordSystemTiming("mon_1", "RenderSystem", 5.0f);
    sys.update(0.016f);
    assertTrue(sys.getHotPathCount("mon_1") == 2, "2 hot paths (Physics + AI)");
}

static void testPerfMonitorSlowest() {
    std::cout << "\n=== PerfMonitor: Slowest ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordSystemTiming("mon_1", "SystemA", 5.0f);
    sys.recordSystemTiming("mon_1", "SystemB", 20.0f);
    sys.recordSystemTiming("mon_1", "SystemC", 8.0f);
    sys.update(0.016f);
    assertTrue(sys.getSlowestSystem("mon_1") == "SystemB", "SystemB is slowest");
}

static void testPerfMonitorReset() {
    std::cout << "\n=== PerfMonitor: Reset ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 40.0f, 200);
    sys.recordSystemTiming("mon_1", "SysA", 15.0f);
    assertTrue(sys.resetMetrics("mon_1"), "Reset succeeds");
    assertTrue(approxEqual(sys.getAverageTickTime("mon_1"), 0.0f), "Avg 0 after reset");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon_1"), 0.0f), "Budget 0 after reset");
    assertTrue(sys.getSlowestSystem("mon_1").empty(), "No slowest after reset");
}

static void testPerfMonitorMissing() {
    std::cout << "\n=== PerfMonitor: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    assertTrue(!sys.initializeMonitor("nonexistent", "s", 50.0f), "Init fails on missing");
    assertTrue(approxEqual(sys.getAverageTickTime("nonexistent"), 0.0f), "Avg 0 on missing");
    assertTrue(approxEqual(sys.getBudgetUtilization("nonexistent"), 0.0f), "Budget 0 on missing");
    assertTrue(!sys.isAlertActive("nonexistent"), "No alert on missing");
    assertTrue(sys.getSlowestSystem("nonexistent").empty(), "Empty slowest on missing");
}


void run_server_performance_monitor_tests() {
    testPerfMonitorInit();
    testPerfMonitorRecordTiming();
    testPerfMonitorTickComplete();
    testPerfMonitorAverage();
    testPerfMonitorBudget();
    testPerfMonitorAlert();
    testPerfMonitorHotPath();
    testPerfMonitorSlowest();
    testPerfMonitorReset();
    testPerfMonitorMissing();
}
