// Tests for: ScanProbeDeploymentSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/scan_probe_deployment_system.h"

using namespace atlas;

// ==================== ScanProbeDeploymentSystem Tests ====================

static void testScanProbeDeploymentCreate() {
    std::cout << "\n=== ScanProbeDeployment: Create ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    assertTrue(sys.initialize("scanner1"), "Init succeeds");
    assertTrue(sys.getActiveProbeCount("scanner1") == 0, "Zero active probes");
    assertTrue(sys.getSignatureCount("scanner1") == 0, "Zero signatures");
    assertTrue(sys.getTotalProbesLaunched("scanner1") == 0, "Zero total probes");
    assertTrue(sys.getTotalSignaturesResolved("scanner1") == 0, "Zero resolved");
}

static void testScanProbeDeploymentInvalidInit() {
    std::cout << "\n=== ScanProbeDeployment: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    assertTrue(!sys.initialize("missing"), "Missing entity fails");
}

static void testScanProbeDeploymentLaunchProbe() {
    std::cout << "\n=== ScanProbeDeployment: LaunchProbe ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    assertTrue(sys.launchProbe("scanner1", "probe1", 10.0f, 20.0f, 0.0f, 4.0f, 1.0f),
               "Launch probe 1");
    assertTrue(sys.launchProbe("scanner1", "probe2", 30.0f, 40.0f, 0.0f, 4.0f, 1.5f),
               "Launch probe 2");
    assertTrue(sys.getActiveProbeCount("scanner1") == 2, "2 active probes");
    assertTrue(sys.getTotalProbesLaunched("scanner1") == 2, "2 total probes launched");

    // Duplicate rejected
    assertTrue(!sys.launchProbe("scanner1", "probe1", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f),
               "Duplicate probe ID rejected");
}

static void testScanProbeDeploymentInvalidLaunch() {
    std::cout << "\n=== ScanProbeDeployment: InvalidLaunch ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    assertTrue(!sys.launchProbe("scanner1", "", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f),
               "Empty probe ID rejected");
    assertTrue(!sys.launchProbe("scanner1", "p1", 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
               "Zero radius rejected");
    assertTrue(!sys.launchProbe("scanner1", "p1", 0.0f, 0.0f, 0.0f, -1.0f, 1.0f),
               "Negative radius rejected");
    assertTrue(!sys.launchProbe("scanner1", "p1", 0.0f, 0.0f, 0.0f, 4.0f, 0.0f),
               "Zero strength rejected");
    assertTrue(!sys.launchProbe("nonexistent", "p1", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f),
               "Missing entity rejected");
}

static void testScanProbeDeploymentMaxProbes() {
    std::cout << "\n=== ScanProbeDeployment: MaxProbes ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    for (int i = 0; i < 8; ++i) {
        std::string id = "probe" + std::to_string(i);
        assertTrue(sys.launchProbe("scanner1", id, 0.0f, 0.0f, 0.0f, 4.0f, 1.0f),
                   ("Launch probe " + std::to_string(i)).c_str());
    }
    assertTrue(sys.getActiveProbeCount("scanner1") == 8, "8 probes (max)");
    assertTrue(!sys.launchProbe("scanner1", "probe_extra", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f),
               "9th probe rejected (max 8)");
}

static void testScanProbeDeploymentRecallProbe() {
    std::cout << "\n=== ScanProbeDeployment: RecallProbe ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    sys.launchProbe("scanner1", "probe1", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f);
    sys.launchProbe("scanner1", "probe2", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f);

    assertTrue(sys.recallProbe("scanner1", "probe1"), "Recall probe 1");
    assertTrue(sys.getActiveProbeCount("scanner1") == 1, "1 active probe after recall");
    assertTrue(!sys.recallProbe("scanner1", "probe1"), "Double recall fails");
    assertTrue(!sys.recallProbe("scanner1", "nonexistent"), "Recall nonexistent fails");
}

static void testScanProbeDeploymentRecallAll() {
    std::cout << "\n=== ScanProbeDeployment: RecallAll ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    sys.launchProbe("scanner1", "probe1", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f);
    sys.launchProbe("scanner1", "probe2", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f);

    assertTrue(sys.recallAllProbes("scanner1"), "Recall all succeeds");
    assertTrue(sys.getActiveProbeCount("scanner1") == 0, "Zero active probes after recall all");
    assertTrue(!sys.recallAllProbes("scanner1"), "Second recall all has nothing to recall");
}

static void testScanProbeDeploymentAddSignature() {
    std::cout << "\n=== ScanProbeDeployment: AddSignature ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    assertTrue(sys.addSignature("scanner1", "sig1", "anomaly", 10.0f, 20.0f, 0.0f),
               "Add anomaly signature");
    assertTrue(sys.addSignature("scanner1", "sig2", "wormhole", 30.0f, 40.0f, 0.0f),
               "Add wormhole signature");
    assertTrue(sys.addSignature("scanner1", "sig3", "data", 50.0f, 60.0f, 0.0f),
               "Add data signature");
    assertTrue(sys.getSignatureCount("scanner1") == 3, "3 signatures");

    // Duplicate rejected
    assertTrue(!sys.addSignature("scanner1", "sig1", "relic", 0.0f, 0.0f, 0.0f),
               "Duplicate sig ID rejected");

    // Invalid inputs
    assertTrue(!sys.addSignature("scanner1", "", "anomaly", 0.0f, 0.0f, 0.0f),
               "Empty sig ID rejected");
    assertTrue(!sys.addSignature("scanner1", "sig4", "", 0.0f, 0.0f, 0.0f),
               "Empty sig type rejected");
}

static void testScanProbeDeploymentScanProgress() {
    std::cout << "\n=== ScanProbeDeployment: ScanProgress ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    // Place signature at (10, 10, 0)
    sys.addSignature("scanner1", "sig1", "anomaly", 10.0f, 10.0f, 0.0f);

    // Place probe at same position, radius 4, strength 1.0
    sys.launchProbe("scanner1", "probe1", 10.0f, 10.0f, 0.0f, 4.0f, 1.0f);

    // Probe is at distance 0, coverage = 1.0, strength = 1.0
    // Progress per second = 1.0 * 1.0 * 0.1 = 0.1
    sys.update(5.0f); // Should add 0.5 progress
    float progress = sys.getSignatureScanProgress("scanner1", "sig1");
    assertTrue(progress > 0.49f && progress < 0.51f, "Progress ~0.5 after 5s");
    assertTrue(!sys.isSignatureResolved("scanner1", "sig1"), "Not yet resolved");

    sys.update(6.0f); // Should complete (0.5 + 0.6 >= 1.0)
    assertTrue(sys.isSignatureResolved("scanner1", "sig1"), "Signature resolved");
    assertTrue(sys.getResolvedSignatureCount("scanner1") == 1, "1 resolved");
    assertTrue(sys.getTotalSignaturesResolved("scanner1") == 1, "1 total resolved");
}

static void testScanProbeDeploymentOutOfRange() {
    std::cout << "\n=== ScanProbeDeployment: OutOfRange ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    // Signature at (100, 100, 0), probe at (0, 0, 0) with radius 4
    sys.addSignature("scanner1", "sig1", "anomaly", 100.0f, 100.0f, 0.0f);
    sys.launchProbe("scanner1", "probe1", 0.0f, 0.0f, 0.0f, 4.0f, 1.0f);

    sys.update(10.0f);
    assertTrue(approxEqual(sys.getSignatureScanProgress("scanner1", "sig1"), 0.0f),
               "No progress when out of range");
}

static void testScanProbeDeploymentUpdate() {
    std::cout << "\n=== ScanProbeDeployment: Update ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testScanProbeDeploymentMissing() {
    std::cout << "\n=== ScanProbeDeployment: Missing ===" << std::endl;
    ecs::World world;
    systems::ScanProbeDeploymentSystem sys(&world);
    assertTrue(sys.getActiveProbeCount("x") == 0, "Default active probes on missing");
    assertTrue(sys.getSignatureCount("x") == 0, "Default signatures on missing");
    assertTrue(sys.getResolvedSignatureCount("x") == 0, "Default resolved on missing");
    assertTrue(approxEqual(sys.getSignatureScanProgress("x", "s"), 0.0f), "Default progress on missing");
    assertTrue(!sys.isSignatureResolved("x", "s"), "Default resolved on missing");
    assertTrue(sys.getTotalProbesLaunched("x") == 0, "Default total launched on missing");
    assertTrue(sys.getTotalSignaturesResolved("x") == 0, "Default total resolved on missing");
}

void run_scan_probe_deployment_system_tests() {
    testScanProbeDeploymentCreate();
    testScanProbeDeploymentInvalidInit();
    testScanProbeDeploymentLaunchProbe();
    testScanProbeDeploymentInvalidLaunch();
    testScanProbeDeploymentMaxProbes();
    testScanProbeDeploymentRecallProbe();
    testScanProbeDeploymentRecallAll();
    testScanProbeDeploymentAddSignature();
    testScanProbeDeploymentScanProgress();
    testScanProbeDeploymentOutOfRange();
    testScanProbeDeploymentUpdate();
    testScanProbeDeploymentMissing();
}
