// Tests for: PlanetScanSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/planet_scan_system.h"

using namespace atlas;

// ==================== PlanetScanSystem Tests ====================

static void testPlanetScanInit() {
    std::cout << "\n=== PlanetScan: Init ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    assertTrue(sys.initialize("planet1", "p_0001", "barren"), "Init succeeds");
    assertTrue(!sys.isScanning("planet1"), "Not scanning initially");
    assertTrue(approxEqual(sys.getScanProgress("planet1"), 0.0f), "0% progress");
    assertTrue(sys.getResultCount("planet1") == 0, "No results yet");
    assertTrue(sys.getProbesLaunched("planet1") == 0, "0 probes launched");
    assertTrue(sys.getTotalScansCompleted("planet1") == 0, "0 scans completed");
}

static void testPlanetScanLaunch() {
    std::cout << "\n=== PlanetScan: Launch ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");

    assertTrue(sys.launchScan("planet1", 80.0f), "Launch scan");
    assertTrue(sys.isScanning("planet1"), "Scanning after launch");
    assertTrue(sys.getProbesLaunched("planet1") == 1, "1 probe launched");
}

static void testPlanetScanDoubleLaunch() {
    std::cout << "\n=== PlanetScan: DoubleLaunch ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");

    sys.launchScan("planet1", 80.0f);
    assertTrue(!sys.launchScan("planet1", 50.0f), "Can't launch while scanning");
    assertTrue(sys.getProbesLaunched("planet1") == 1, "Still 1 probe");
}

static void testPlanetScanCancel() {
    std::cout << "\n=== PlanetScan: Cancel ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");

    sys.launchScan("planet1", 80.0f);
    assertTrue(sys.cancelScan("planet1"), "Cancel scan");
    assertTrue(!sys.isScanning("planet1"), "Not scanning after cancel");
    assertTrue(approxEqual(sys.getScanProgress("planet1"), 0.0f), "Progress reset");
    // Cancel when not scanning fails
    assertTrue(!sys.cancelScan("planet1"), "Cancel again fails");
}

static void testPlanetScanProgress() {
    std::cout << "\n=== PlanetScan: Progress ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");
    sys.launchScan("planet1", 100.0f);  // strength=100 so rate=100/60=1.667%/s

    sys.update(10.0f);  // 10s → ~16.7% progress (rate=100/60 * 10 * 100)
    float prog = sys.getScanProgress("planet1");
    assertTrue(prog > 0.0f && prog < 100.0f, "Progress between 0 and 100");
    assertTrue(sys.isScanning("planet1"), "Still scanning");
}

static void testPlanetScanComplete() {
    std::cout << "\n=== PlanetScan: Complete ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");
    sys.launchScan("planet1", 100.0f);

    sys.update(120.0f);  // well past scan_duration=60s
    assertTrue(!sys.isScanning("planet1"), "Not scanning after completion");
    assertTrue(approxEqual(sys.getScanProgress("planet1"), 100.0f), "100% progress");
    assertTrue(sys.getResultCount("planet1") > 0, "Results populated");
    assertTrue(sys.getTotalScansCompleted("planet1") == 1, "1 scan completed");
}

static void testPlanetScanTemperateResults() {
    std::cout << "\n=== PlanetScan: TemperateResults ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_temp", "temperate");
    sys.launchScan("planet1", 100.0f);

    sys.update(120.0f);
    // Temperate planets have 5 resource types
    assertTrue(sys.getResultCount("planet1") == 5, "5 temperate results");
}

static void testPlanetScanConfirmResult() {
    std::cout << "\n=== PlanetScan: ConfirmResult ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");
    sys.launchScan("planet1", 100.0f);
    sys.update(120.0f);

    assertTrue(sys.confirmResult("planet1", "base_metals"), "Confirm base_metals");
    assertTrue(!sys.confirmResult("planet1", "nonexistent_res"),
               "Confirm unknown fails");
}

static void testPlanetScanMultipleScans() {
    std::cout << "\n=== PlanetScan: MultipleScans ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    world.createEntity("planet1");
    sys.initialize("planet1", "p_0001", "barren");

    sys.launchScan("planet1", 100.0f);
    sys.update(120.0f);
    assertTrue(sys.getTotalScansCompleted("planet1") == 1, "1st scan done");

    // Re-scan
    sys.launchScan("planet1", 60.0f);
    sys.update(120.0f);
    assertTrue(sys.getTotalScansCompleted("planet1") == 2, "2nd scan done");
    assertTrue(sys.getProbesLaunched("planet1") == 2, "2 probes launched total");
}

static void testPlanetScanMissing() {
    std::cout << "\n=== PlanetScan: Missing ===" << std::endl;
    ecs::World world;
    systems::PlanetScanSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "p_x", "barren"),
               "Init fails on missing entity");
    assertTrue(!sys.launchScan("nonexistent", 80.0f), "Launch fails on missing");
    assertTrue(!sys.cancelScan("nonexistent"), "Cancel fails on missing");
    assertTrue(!sys.isScanning("nonexistent"), "Not scanning on missing");
    assertTrue(approxEqual(sys.getScanProgress("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getResultCount("nonexistent") == 0, "0 results on missing");
    assertTrue(sys.getProbesLaunched("nonexistent") == 0, "0 probes on missing");
    assertTrue(sys.getTotalScansCompleted("nonexistent") == 0, "0 complete on missing");
}

void run_planet_scan_system_tests() {
    testPlanetScanInit();
    testPlanetScanLaunch();
    testPlanetScanDoubleLaunch();
    testPlanetScanCancel();
    testPlanetScanProgress();
    testPlanetScanComplete();
    testPlanetScanTemperateResults();
    testPlanetScanConfirmResult();
    testPlanetScanMultipleScans();
    testPlanetScanMissing();
}
