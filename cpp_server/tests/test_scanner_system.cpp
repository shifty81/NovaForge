// Tests for: Scanner System Tests, Scan → Discover → Warp Integration Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "network/protocol_handler.h"
#include "systems/anomaly_system.h"
#include "systems/scanner_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Scanner System Tests ====================

static void testScannerStartScan() {
    ecs::World world;
    systems::ScannerSystem sys(&world);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    scanner_entity->addComponent(std::move(sc));

    bool started = sys.startScan("scanner1", "sys1");
    assertTrue(started, "startScan returns true");
    assertTrue(sys.getActiveScannerCount() == 1, "One active scanner");
}

static void testScannerStopScan() {
    ecs::World world;
    systems::ScannerSystem sys(&world);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    scanner_entity->addComponent(std::move(sc));

    sys.startScan("scanner1", "sys1");
    bool stopped = sys.stopScan("scanner1");
    assertTrue(stopped, "stopScan returns true");
    assertTrue(sys.getActiveScannerCount() == 0, "No active scanners");
}

static void testScannerDetectsAnomaly() {
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys1", 42, 0.8f);  // easy to scan (highsec)

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 100.0f;  // strong scanner
    sc->scan_duration = 1.0f;    // fast scans
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "sys1");

    // Run enough scan cycles to detect
    for (int i = 0; i < 5; ++i) {
        scanSys.update(1.1f);  // complete one cycle
    }

    auto results = scanSys.getScanResults("scanner1");
    assertTrue(!results.empty(), "Scanner detected anomalies");
    assertTrue(results[0].signal_strength > 0.0f, "Signal strength is positive");
}

static void testScannerSignalAccumulates() {
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys1", 42, 0.8f);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 80.0f;
    sc->scan_duration = 1.0f;
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "sys1");
    scanSys.update(1.1f);  // first cycle

    auto results1 = scanSys.getScanResults("scanner1");
    float sig1 = results1.empty() ? 0.0f : results1[0].signal_strength;

    scanSys.update(1.1f);  // second cycle
    auto results2 = scanSys.getScanResults("scanner1");
    float sig2 = results2.empty() ? 0.0f : results2[0].signal_strength;

    assertTrue(sig2 > sig1, "Signal strength increases with scans");
}

static void testScannerEffectiveScanStrength() {
    float s8 = systems::ScannerSystem::effectiveScanStrength(50.0f, 8);
    float s4 = systems::ScannerSystem::effectiveScanStrength(50.0f, 4);
    float s1 = systems::ScannerSystem::effectiveScanStrength(50.0f, 1);

    assertTrue(s8 > s4, "8 probes stronger than 4");
    assertTrue(s4 > s1, "4 probes stronger than 1");
    assertTrue(approxEqual(s8, 50.0f, 0.1f), "8 probes at base 50 = 50 effective");
}

static void testScannerSignalGainPerCycle() {
    float gain_strong = systems::ScannerSystem::signalGainPerCycle(100.0f, 1.0f);
    float gain_weak   = systems::ScannerSystem::signalGainPerCycle(20.0f, 0.2f);

    assertTrue(gain_strong > gain_weak,
               "Strong scanner + strong signal > weak scanner + weak signal");
}

static void testScannerWarpableAtFullSignal() {
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys1", 42, 0.9f);  // trivial difficulty = strong signal

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 200.0f;   // very strong
    sc->scan_duration = 0.5f;
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "sys1");

    // Scan many times to get full signal
    for (int i = 0; i < 20; ++i) {
        scanSys.update(0.6f);
    }

    auto results = scanSys.getScanResults("scanner1");
    bool any_warpable = false;
    for (const auto& r : results) {
        if (r.warpable) any_warpable = true;
    }
    assertTrue(any_warpable, "At least one anomaly is warpable after many scans");
}

static void testScannerNoResultWithoutAnomalies() {
    ecs::World world;
    systems::ScannerSystem scanSys(&world);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_duration = 1.0f;
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "empty_sys");
    scanSys.update(1.1f);

    auto results = scanSys.getScanResults("scanner1");
    assertTrue(results.empty(), "No results in system without anomalies");
}


// ==================== Scan → Discover → Warp Integration Tests ====================

static void testScanDiscoverWarpFlow() {
    std::cout << "\n=== Scan Discover Warp Flow ===" << std::endl;
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    // 1. Generate anomalies in a system
    int created = anomSys.generateAnomalies("sys_flow", 777, 0.3f);  // low-sec = more anomalies
    assertTrue(created > 0, "Anomalies generated for flow test");

    // 2. Create a scanner entity
    auto* scanner = world.createEntity("player_scanner");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 150.0f;
    sc->scan_duration = 0.5f;
    sc->probe_count = 8;
    scanner->addComponent(std::move(sc));

    // 3. Start scanning
    bool started = scanSys.startScan("player_scanner", "sys_flow");
    assertTrue(started, "Scan started successfully");

    // 4. Verify scanner is active
    assertTrue(scanSys.getActiveScannerCount() == 1, "One active scanner");

    // 5. Run enough scan cycles to discover anomalies
    for (int i = 0; i < 30; ++i) {
        scanSys.update(0.6f);
    }

    // 6. Check results - should have discovered at least one anomaly
    auto results = scanSys.getScanResults("player_scanner");
    assertTrue(!results.empty(), "Scanner discovered anomalies");

    // 7. Verify at least one anomaly has signal strength > 0
    bool has_signal = false;
    for (const auto& r : results) {
        if (r.signal_strength > 0.0f) has_signal = true;
    }
    assertTrue(has_signal, "At least one anomaly has signal strength");

    // 8. Check warpable status - strong scanner should get full signal
    bool any_warpable = false;
    for (const auto& r : results) {
        if (r.warpable) any_warpable = true;
    }
    assertTrue(any_warpable, "At least one anomaly is warpable");

    // 9. Get anomaly IDs from results
    std::string warpable_id;
    for (const auto& r : results) {
        if (r.warpable) {
            warpable_id = r.anomaly_id;
            break;
        }
    }
    assertTrue(!warpable_id.empty(), "Warpable anomaly has a valid ID");

    // 10. Verify anomaly still exists in the system
    auto system_anomalies = anomSys.getAnomaliesInSystem("sys_flow");
    bool found = false;
    for (const auto& id : system_anomalies) {
        if (id == warpable_id) found = true;
    }
    assertTrue(found, "Warpable anomaly exists in system anomaly list");
}

static void testScanStopPreservesResults() {
    std::cout << "\n=== Scan Stop Preserves Results ===" << std::endl;
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys_stop", 888, 0.5f);

    auto* scanner = world.createEntity("scanner_stop");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 100.0f;
    sc->scan_duration = 0.5f;
    scanner->addComponent(std::move(sc));

    scanSys.startScan("scanner_stop", "sys_stop");
    // Run a few cycles
    for (int i = 0; i < 5; ++i) {
        scanSys.update(0.6f);
    }

    auto results_before = scanSys.getScanResults("scanner_stop");
    scanSys.stopScan("scanner_stop");
    auto results_after = scanSys.getScanResults("scanner_stop");

    assertTrue(results_before.size() == results_after.size(),
               "Results preserved after stopping scan");
}

static void testScanAnomalyComplete() {
    std::cout << "\n=== Scan Anomaly Complete ===" << std::endl;
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys_complete", 999, 0.8f);

    // Get anomaly count before completion
    int before = anomSys.getActiveAnomalyCount("sys_complete");
    assertTrue(before > 0, "System has active anomalies");

    // Complete the first anomaly
    auto ids = anomSys.getAnomaliesInSystem("sys_complete");
    assertTrue(!ids.empty(), "Anomaly IDs available");
    bool completed = anomSys.completeAnomaly(ids[0]);
    assertTrue(completed, "Anomaly marked as completed");

    // Active count should decrease
    int after = anomSys.getActiveAnomalyCount("sys_complete");
    assertTrue(after < before, "Active anomaly count decreased after completion");
}

static void testScanProtocolRoundTrip() {
    std::cout << "\n=== Scan Protocol Round Trip ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Create scan start message and verify it round-trips
    std::string scan_msg = "{\"message_type\":\"scan_start\",\"data\":{\"system_id\":\"sys_rt\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(scan_msg, type, data);
    assertTrue(ok, "Scan start message parses");
    assertTrue(type == atlas::network::MessageType::SCAN_START, "Type is SCAN_START");
    assertTrue(data.find("sys_rt") != std::string::npos, "Data contains system_id");

    // Create scan result and verify it contains all expected fields
    std::string result = proto.createScanResult("sc_rt", 1,
        "[{\"anomaly_id\":\"a_rt\",\"signal_strength\":1.0,\"warpable\":true}]");
    assertTrue(result.find("scan_result") != std::string::npos, "Result type is scan_result");
    assertTrue(result.find("sc_rt") != std::string::npos, "Result contains scanner_id");
    assertTrue(result.find("a_rt") != std::string::npos, "Result contains anomaly_id");
    assertTrue(result.find("warpable") != std::string::npos, "Result contains warpable field");

    // Create anomaly list and verify it round-trips
    std::string anom_list = proto.createAnomalyList("sys_rt", 2, "[\"a1\",\"a2\"]");
    assertTrue(anom_list.find("anomaly_list") != std::string::npos, "List type is anomaly_list");
    assertTrue(anom_list.find("sys_rt") != std::string::npos, "List contains system_id");
}


void run_scanner_system_tests() {
    testScannerStartScan();
    testScannerStopScan();
    testScannerDetectsAnomaly();
    testScannerSignalAccumulates();
    testScannerEffectiveScanStrength();
    testScannerSignalGainPerCycle();
    testScannerWarpableAtFullSignal();
    testScannerNoResultWithoutAnomalies();
    testScanDiscoverWarpFlow();
    testScanStopPreservesResults();
    testScanAnomalyComplete();
    testScanProtocolRoundTrip();
}
