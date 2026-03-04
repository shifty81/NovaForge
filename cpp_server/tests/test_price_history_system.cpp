// Tests for: Price History System Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/price_history_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Price History System Tests ====================

static void testPriceHistoryDefaults() {
    std::cout << "\n=== Price History Defaults ===" << std::endl;
    ecs::World world;
    systems::PriceHistorySystem priceHistSys(&world);
    
    // No history recorded yet
    auto history = priceHistSys.getHistory("region1", "tritanium");
    assertTrue(history.empty(), "No history for untracked item");
    
    double avg = priceHistSys.getAveragePrice("region1", "tritanium", 3600.0f, 0.0f);
    assertTrue(avg < 0.0, "Average returns -1 when no data");
}

static void testPriceHistoryRecording() {
    std::cout << "\n=== Price History Recording ===" << std::endl;
    ecs::World world;
    systems::PriceHistorySystem priceHistSys(&world);
    
    priceHistSys.recordPrice("jita", "tritanium", 6.0, 5.5, 1000, 0.0f);
    priceHistSys.recordPrice("jita", "tritanium", 6.2, 5.6, 800, 3600.0f);
    priceHistSys.recordPrice("jita", "tritanium", 6.5, 5.8, 1200, 7200.0f);
    
    auto history = priceHistSys.getHistory("jita", "tritanium");
    assertTrue(history.size() == 3, "3 price entries recorded");
    assertTrue(approxEqual(static_cast<float>(history[0].sell_price), 6.0f), "First entry sell price correct");
    assertTrue(approxEqual(static_cast<float>(history[2].sell_price), 6.5f), "Last entry sell price correct");
}

static void testPriceHistoryAverage() {
    std::cout << "\n=== Price History Average ===" << std::endl;
    ecs::World world;
    systems::PriceHistorySystem priceHistSys(&world);
    
    priceHistSys.recordPrice("amarr", "mexallon", 40.0, 38.0, 500, 0.0f);
    priceHistSys.recordPrice("amarr", "mexallon", 44.0, 42.0, 600, 1800.0f);
    priceHistSys.recordPrice("amarr", "mexallon", 46.0, 44.0, 400, 3600.0f);
    
    double avg = priceHistSys.getAveragePrice("amarr", "mexallon", 4000.0f, 3600.0f);
    // Average of 40, 44, 46 = 43.33
    assertTrue(approxEqual(static_cast<float>(avg), 43.33f, 0.1f), "Average price calculated correctly");
}

static void testPriceHistoryTrend() {
    std::cout << "\n=== Price History Trend ===" << std::endl;
    ecs::World world;
    systems::PriceHistorySystem priceHistSys(&world);
    
    // Rising prices
    priceHistSys.recordPrice("dodixie", "pyerite", 10.0, 9.0, 100, 0.0f);
    priceHistSys.recordPrice("dodixie", "pyerite", 12.0, 11.0, 100, 3600.0f);
    priceHistSys.recordPrice("dodixie", "pyerite", 14.0, 13.0, 100, 7200.0f);
    priceHistSys.recordPrice("dodixie", "pyerite", 16.0, 15.0, 100, 10800.0f);
    
    float trend = priceHistSys.getPriceTrend("dodixie", "pyerite", 4);
    assertTrue(trend > 0.0f, "Rising prices show positive trend");
}

static void testPriceHistoryVolume() {
    std::cout << "\n=== Price History Volume ===" << std::endl;
    ecs::World world;
    systems::PriceHistorySystem priceHistSys(&world);
    
    priceHistSys.recordPrice("rens", "isogen", 50.0, 48.0, 1000, 0.0f);
    priceHistSys.recordPrice("rens", "isogen", 52.0, 50.0, 2000, 1800.0f);
    priceHistSys.recordPrice("rens", "isogen", 51.0, 49.0, 1500, 3600.0f);
    
    int vol = priceHistSys.getTotalVolume("rens", "isogen", 4000.0f, 3600.0f);
    assertTrue(vol == 4500, "Total volume is sum of all entries (1000+2000+1500=4500)");
}

static void testPriceHistoryIntervalConfig() {
    std::cout << "\n=== Price History Interval Config ===" << std::endl;
    ecs::World world;
    systems::PriceHistorySystem priceHistSys(&world);
    
    assertTrue(approxEqual(priceHistSys.getSnapshotInterval(), 3600.0f), "Default interval is 1 hour");
    priceHistSys.setSnapshotInterval(1800.0f);
    assertTrue(approxEqual(priceHistSys.getSnapshotInterval(), 1800.0f), "Interval updated to 30 minutes");
    
    assertTrue(priceHistSys.getMaxHistoryEntries() == 720, "Default max entries is 720");
    priceHistSys.setMaxHistoryEntries(168);
    assertTrue(priceHistSys.getMaxHistoryEntries() == 168, "Max entries updated to 168");
}


void run_price_history_system_tests() {
    testPriceHistoryDefaults();
    testPriceHistoryRecording();
    testPriceHistoryAverage();
    testPriceHistoryTrend();
    testPriceHistoryVolume();
    testPriceHistoryIntervalConfig();
}
