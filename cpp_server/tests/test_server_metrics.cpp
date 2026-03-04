// Tests for: ServerMetrics Tests
#include "test_log.h"
#include "components/core_components.h"
#include "utils/server_metrics.h"
#include <thread>
#include <sys/stat.h>

using namespace atlas;

// ==================== ServerMetrics Tests ====================

static void testMetricsTickTiming() {
    std::cout << "\n=== Metrics Tick Timing ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getTotalTicks() == 0, "No ticks recorded initially");
    assertTrue(metrics.getAvgTickMs() == 0.0, "Avg tick 0 with no data");
    assertTrue(metrics.getMaxTickMs() == 0.0, "Max tick 0 with no data");
    assertTrue(metrics.getMinTickMs() == 0.0, "Min tick 0 with no data");
    
    // Record a few ticks with a known sleep
    for (int i = 0; i < 5; ++i) {
        metrics.recordTickStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        metrics.recordTickEnd();
    }
    
    assertTrue(metrics.getTotalTicks() == 5, "5 ticks recorded");
    assertTrue(metrics.getAvgTickMs() >= 1.0, "Average tick >= 1ms");
    assertTrue(metrics.getMaxTickMs() >= 1.0, "Max tick >= 1ms");
    assertTrue(metrics.getMinTickMs() >= 1.0, "Min tick >= 1ms");
    assertTrue(metrics.getMaxTickMs() >= metrics.getMinTickMs(), "Max >= Min");
}

static void testMetricsCounters() {
    std::cout << "\n=== Metrics Counters ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getEntityCount() == 0, "Entity count starts at 0");
    assertTrue(metrics.getPlayerCount() == 0, "Player count starts at 0");
    
    metrics.setEntityCount(42);
    metrics.setPlayerCount(3);
    
    assertTrue(metrics.getEntityCount() == 42, "Entity count set to 42");
    assertTrue(metrics.getPlayerCount() == 3, "Player count set to 3");
}

static void testMetricsUptime() {
    std::cout << "\n=== Metrics Uptime ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getUptimeSeconds() >= 0.0, "Uptime is non-negative");
    
    std::string uptime = metrics.getUptimeString();
    assertTrue(!uptime.empty(), "Uptime string is not empty");
    assertTrue(uptime.find("d") != std::string::npos, "Uptime contains 'd'");
    assertTrue(uptime.find("h") != std::string::npos, "Uptime contains 'h'");
    assertTrue(uptime.find("m") != std::string::npos, "Uptime contains 'm'");
    assertTrue(uptime.find("s") != std::string::npos, "Uptime contains 's'");
}

static void testMetricsSummary() {
    std::cout << "\n=== Metrics Summary ===" << std::endl;
    
    utils::ServerMetrics metrics;
    metrics.setEntityCount(10);
    metrics.setPlayerCount(2);
    
    metrics.recordTickStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    metrics.recordTickEnd();
    
    std::string s = metrics.summary();
    assertTrue(!s.empty(), "Summary is not empty");
    assertTrue(s.find("[Metrics]") != std::string::npos, "Summary contains [Metrics]");
    assertTrue(s.find("entities=10") != std::string::npos, "Summary contains entity count");
    assertTrue(s.find("players=2") != std::string::npos, "Summary contains player count");
    assertTrue(s.find("uptime") != std::string::npos, "Summary contains uptime");
    assertTrue(s.find("ticks=") != std::string::npos, "Summary contains tick count");
}

static void testMetricsResetWindow() {
    std::cout << "\n=== Metrics Reset Window ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    // Record some ticks
    for (int i = 0; i < 3; ++i) {
        metrics.recordTickStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        metrics.recordTickEnd();
    }
    
    assertTrue(metrics.getTotalTicks() == 3, "3 ticks before reset");
    assertTrue(metrics.getAvgTickMs() > 0.0, "Avg > 0 before reset");
    
    metrics.resetWindow();
    
    // Total ticks should remain, but window stats reset
    assertTrue(metrics.getTotalTicks() == 3, "Total ticks preserved after reset");
    assertTrue(metrics.getAvgTickMs() == 0.0, "Avg reset to 0 after window reset");
    assertTrue(metrics.getMaxTickMs() == 0.0, "Max reset to 0 after window reset");
    assertTrue(metrics.getMinTickMs() == 0.0, "Min reset to 0 after window reset");
}


void run_server_metrics_tests() {
    testMetricsTickTiming();
    testMetricsCounters();
    testMetricsUptime();
    testMetricsSummary();
    testMetricsResetWindow();
}
