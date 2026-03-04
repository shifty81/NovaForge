// Tests for: ScanProbe System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/scan_probe_system.h"

using namespace atlas;

// ==================== ScanProbe System Tests ====================

static void testScanProbeCreate() {
    std::cout << "\n=== ScanProbe: Create ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeProbes("ship1", "player1"), "Init probes succeeds");
    assertTrue(sys.getProbeCount("ship1") == 0, "No probes initially");
    assertTrue(sys.getTotalScansCompleted("ship1") == 0, "No scans initially");
    assertTrue(sys.getTotalSitesFound("ship1") == 0, "No sites initially");
}

static void testScanProbeLaunch() {
    std::cout << "\n=== ScanProbe: Launch ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    assertTrue(sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f), "Launch probe succeeds");
    assertTrue(sys.getProbeCount("ship1") == 1, "1 probe launched");
    assertTrue(approxEqual(sys.getScanProgress("ship1", "p1"), 0.0f), "Initial progress 0");
}

static void testScanProbeDuplicate() {
    std::cout << "\n=== ScanProbe: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    assertTrue(!sys.launchProbe("ship1", "p1", 1, 0.0f, 0.0f, 0.0f), "Duplicate probe rejected");
    assertTrue(sys.getProbeCount("ship1") == 1, "Still 1 probe");
}

static void testScanProbeScan() {
    std::cout << "\n=== ScanProbe: Scan ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    assertTrue(sys.startScan("ship1", "p1"), "Start scan succeeds");
    assertTrue(sys.getActiveProbeCount("ship1") == 1, "1 active probe");
    // scan_time=10, scan_strength=1, delta=5 → progress = 0.5
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getScanProgress("ship1", "p1"), 0.5f), "Progress at 0.5");
}

static void testScanProbeComplete() {
    std::cout << "\n=== ScanProbe: Complete ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    sys.startScan("ship1", "p1");
    sys.update(10.0f); // completes scan
    assertTrue(approxEqual(sys.getScanProgress("ship1", "p1"), 1.0f), "Scan complete at 1.0");
    assertTrue(sys.getTotalScansCompleted("ship1") == 1, "1 scan completed");
    assertTrue(sys.getActiveProbeCount("ship1") == 0, "No active probes after complete");
}

static void testScanProbeExpiry() {
    std::cout << "\n=== ScanProbe: Expiry ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    // Default lifetime = 300s
    sys.update(300.0f);
    assertTrue(sys.getProbeCount("ship1") == 0, "Probe expired and removed");
}

static void testScanProbeRecall() {
    std::cout << "\n=== ScanProbe: Recall ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    sys.launchProbe("ship1", "p2", 1, 50.0f, 60.0f, 70.0f);
    assertTrue(sys.recallProbe("ship1", "p1"), "Recall succeeds");
    assertTrue(sys.getProbeCount("ship1") == 1, "1 probe remaining");
    assertTrue(!sys.recallProbe("ship1", "p1"), "Recall nonexistent fails");
}

static void testScanProbeResults() {
    std::cout << "\n=== ScanProbe: Results ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    assertTrue(sys.addResult("ship1", "sig1", "Anomaly", 0.75f), "Add result succeeds");
    assertTrue(sys.getResultCount("ship1") == 1, "1 result");
    assertTrue(sys.getTotalSitesFound("ship1") == 1, "1 site found");
    assertTrue(!sys.addResult("ship1", "sig1", "Wormhole", 0.5f), "Duplicate result rejected");
    assertTrue(sys.getResultCount("ship1") == 1, "Still 1 result");
}

static void testScanProbeMaxLimit() {
    std::cout << "\n=== ScanProbe: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    // max_probes = 8
    for (int i = 0; i < 8; i++) {
        sys.launchProbe("ship1", "p" + std::to_string(i), 0, 0, 0, 0);
    }
    assertTrue(!sys.launchProbe("ship1", "p8", 0, 0, 0, 0), "Max probes enforced at 8");
    assertTrue(sys.getProbeCount("ship1") == 8, "Still 8 probes");
}

static void testScanProbeMissing() {
    std::cout << "\n=== ScanProbe: Missing ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    assertTrue(!sys.initializeProbes("nonexistent", "p1"), "Init fails on missing");
    assertTrue(!sys.launchProbe("nonexistent", "p1", 0, 0, 0, 0), "Launch fails on missing");
    assertTrue(!sys.recallProbe("nonexistent", "p1"), "Recall fails on missing");
    assertTrue(!sys.startScan("nonexistent", "p1"), "Start scan fails on missing");
    assertTrue(sys.getProbeCount("nonexistent") == 0, "0 probes on missing");
    assertTrue(sys.getActiveProbeCount("nonexistent") == 0, "0 active on missing");
    assertTrue(approxEqual(sys.getScanProgress("nonexistent", "p1"), 0.0f), "0 progress on missing");
    assertTrue(!sys.addResult("nonexistent", "r1", "Anomaly", 0.5f), "Add result fails on missing");
    assertTrue(sys.getResultCount("nonexistent") == 0, "0 results on missing");
    assertTrue(sys.getTotalScansCompleted("nonexistent") == 0, "0 scans on missing");
    assertTrue(sys.getTotalSitesFound("nonexistent") == 0, "0 sites on missing");
}


void run_scan_probe_system_tests() {
    testScanProbeCreate();
    testScanProbeLaunch();
    testScanProbeDuplicate();
    testScanProbeScan();
    testScanProbeComplete();
    testScanProbeExpiry();
    testScanProbeRecall();
    testScanProbeResults();
    testScanProbeMaxLimit();
    testScanProbeMissing();
}
