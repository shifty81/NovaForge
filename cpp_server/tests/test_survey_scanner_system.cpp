// Tests for: SurveyScannerSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/survey_scanner_system.h"

using namespace atlas;

// ==================== SurveyScannerSystem Tests ====================

static void testSurveyScannerInit() {
    std::cout << "\n=== SurveyScanner: Init ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getResultCount("e1") == 0, "Zero results initially");
    assertTrue(sys.getTotalScansCompleted("e1") == 0, "Zero total scans");
    assertTrue(approxEqual(sys.getTotalValueScanned("e1"), 0.0f), "Zero total value");
    assertTrue(!sys.isScanning("e1"), "Not scanning initially");
    assertTrue(!sys.isScanComplete("e1"), "Not complete initially");
    assertTrue(approxEqual(sys.getScanProgress("e1"), 0.0f), "0 progress");
    assertTrue(sys.getTargetBeltId("e1") == "", "No target belt");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSurveyScannerStartScan() {
    std::cout << "\n=== SurveyScanner: StartScan ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.startScan("e1", "belt_alpha"), "Start scan");
    assertTrue(sys.isScanning("e1"), "Is scanning");
    assertTrue(!sys.isScanComplete("e1"), "Not complete yet");
    assertTrue(sys.getTargetBeltId("e1") == "belt_alpha", "Target belt set");
    assertTrue(approxEqual(sys.getScanProgress("e1"), 0.0f), "Progress at 0");

    // Cannot start another scan while scanning
    assertTrue(!sys.startScan("e1", "belt_beta"), "Cannot start while scanning");
    assertTrue(sys.getTargetBeltId("e1") == "belt_alpha", "Target unchanged");

    // Empty belt ID rejected
    assertTrue(!sys.startScan("e1", ""), "Empty belt ID rejected");
}

static void testSurveyScannerScanProgress() {
    std::cout << "\n=== SurveyScanner: ScanProgress ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setScanDuration("e1", 10.0f);
    sys.startScan("e1", "belt_1");

    // Advance halfway
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getScanProgress("e1"), 0.5f), "50% progress");
    assertTrue(sys.isScanning("e1"), "Still scanning at 50%");
    assertTrue(!sys.isScanComplete("e1"), "Not complete at 50%");

    // Advance to completion
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getScanProgress("e1"), 1.0f), "100% progress");
    assertTrue(!sys.isScanning("e1"), "No longer scanning after completion");
    assertTrue(sys.isScanComplete("e1"), "Scan complete");
    assertTrue(sys.getTotalScansCompleted("e1") == 1, "1 total scan completed");

    // Cannot start new scan while Complete — not Idle
    assertTrue(!sys.startScan("e1", "belt_2"), "Cannot start while Complete");
}

static void testSurveyScannerCancelScan() {
    std::cout << "\n=== SurveyScanner: CancelScan ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Cannot cancel when idle
    assertTrue(!sys.cancelScan("e1"), "Cannot cancel while idle");

    sys.startScan("e1", "belt_1");
    sys.update(2.0f);
    assertTrue(sys.cancelScan("e1"), "Cancel succeeds");
    assertTrue(!sys.isScanning("e1"), "No longer scanning");
    assertTrue(!sys.isScanComplete("e1"), "Not complete");
    assertTrue(approxEqual(sys.getScanProgress("e1"), 0.0f), "Progress reset");
    assertTrue(sys.getTargetBeltId("e1") == "", "Target cleared");

    // Can start a new scan after cancel
    assertTrue(sys.startScan("e1", "belt_2"), "Start new scan after cancel");
    assertTrue(sys.isScanning("e1"), "Scanning again");
}

static void testSurveyScannerAddResult() {
    std::cout << "\n=== SurveyScanner: AddResult ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addResult("e1", "r1", "ast1", "Veldspar", 1000.0f, 5000.0f, 3000.0f),
               "Add result");
    assertTrue(sys.getResultCount("e1") == 1, "1 result");
    assertTrue(sys.hasResult("e1", "r1"), "Has r1");
    assertTrue(approxEqual(sys.getResultQuantity("e1", "r1"), 1000.0f), "Quantity matches");
    assertTrue(sys.getResultOreType("e1", "r1") == "Veldspar", "Ore type matches");
    assertTrue(approxEqual(sys.getTotalValueScanned("e1"), 5000.0f), "Total value = 5000");

    assertTrue(sys.addResult("e1", "r2", "ast2", "Scordite", 500.0f, 3000.0f, 5000.0f),
               "Add second result");
    assertTrue(sys.getResultCount("e1") == 2, "2 results");
    assertTrue(approxEqual(sys.getTotalValueScanned("e1"), 8000.0f), "Total value = 8000");
}

static void testSurveyScannerAddResultValidation() {
    std::cout << "\n=== SurveyScanner: AddResultValidation ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.addResult("e1", "", "ast1", "Veldspar", 100, 50, 30),
               "Empty result_id rejected");
    assertTrue(!sys.addResult("e1", "r1", "", "Veldspar", 100, 50, 30),
               "Empty asteroid_id rejected");
    assertTrue(!sys.addResult("e1", "r1", "ast1", "", 100, 50, 30),
               "Empty ore_type rejected");
    assertTrue(!sys.addResult("e1", "r1", "ast1", "Veldspar", 0.0f, 50, 30),
               "Zero quantity rejected");
    assertTrue(!sys.addResult("e1", "r1", "ast1", "Veldspar", -1.0f, 50, 30),
               "Negative quantity rejected");
    assertTrue(!sys.addResult("e1", "r1", "ast1", "Veldspar", 100, -1.0f, 30),
               "Negative value rejected");
    assertTrue(!sys.addResult("e1", "r1", "ast1", "Veldspar", 100, 50, -1.0f),
               "Negative distance rejected");

    assertTrue(sys.addResult("e1", "r1", "ast1", "Veldspar", 100, 0.0f, 0.0f),
               "Zero value and zero distance allowed");
    assertTrue(!sys.addResult("e1", "r1", "ast1", "Scordite", 200, 100, 50),
               "Duplicate result_id rejected");
    assertTrue(!sys.addResult("missing", "r9", "ast1", "X", 10, 5, 3),
               "Missing entity rejected");
}

static void testSurveyScannerCapacity() {
    std::cout << "\n=== SurveyScanner: Capacity ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxResults("e1", 3);

    assertTrue(sys.addResult("e1", "r1", "a1", "Veldspar", 100, 10, 5), "Add 1");
    assertTrue(sys.addResult("e1", "r2", "a2", "Scordite", 200, 20, 5), "Add 2");
    assertTrue(sys.addResult("e1", "r3", "a3", "Pyroxeres", 300, 30, 5), "Add 3");
    assertTrue(!sys.addResult("e1", "r4", "a4", "Kernite", 400, 40, 5),
               "Add 4 rejected at capacity");
    assertTrue(sys.getResultCount("e1") == 3, "Still 3");
}

static void testSurveyScannerRemoveResult() {
    std::cout << "\n=== SurveyScanner: RemoveResult ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addResult("e1", "r1", "a1", "Veldspar", 100, 10, 5);
    sys.addResult("e1", "r2", "a2", "Scordite", 200, 20, 5);

    assertTrue(sys.removeResult("e1", "r1"), "Remove r1");
    assertTrue(sys.getResultCount("e1") == 1, "1 result left");
    assertTrue(!sys.hasResult("e1", "r1"), "r1 gone");
    assertTrue(sys.hasResult("e1", "r2"), "r2 present");
    assertTrue(!sys.removeResult("e1", "r1"), "Remove already removed fails");
    assertTrue(!sys.removeResult("e1", "unknown"), "Remove unknown fails");
}

static void testSurveyScannerClearResults() {
    std::cout << "\n=== SurveyScanner: ClearResults ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addResult("e1", "r1", "a1", "Veldspar", 100, 10, 5);
    sys.addResult("e1", "r2", "a2", "Scordite", 200, 20, 5);

    // Clear while Complete should reset to Idle
    sys.startScan("e1", "belt_1");
    sys.setScanDuration("e1", 1.0f);
    sys.update(2.0f);
    assertTrue(sys.isScanComplete("e1"), "Scan is Complete");
    assertTrue(sys.clearResults("e1"), "ClearResults succeeds");
    assertTrue(sys.getResultCount("e1") == 0, "0 results after clear");
    assertTrue(!sys.isScanComplete("e1"), "Status reset to Idle after clear");
    assertTrue(approxEqual(sys.getTotalValueScanned("e1"), 30.0f),
               "Total value preserved");
}

static void testSurveyScannerConfiguration() {
    std::cout << "\n=== SurveyScanner: Configuration ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setScanDuration("e1", 12.0f), "Set scan duration");
    assertTrue(approxEqual(sys.getScanDuration("e1"), 12.0f), "Duration is 12");
    assertTrue(!sys.setScanDuration("e1", 0.0f), "Zero duration rejected");
    assertTrue(!sys.setScanDuration("e1", -1.0f), "Negative duration rejected");

    assertTrue(sys.setScanRange("e1", 20000.0f), "Set scan range");
    assertTrue(approxEqual(sys.getScanRange("e1"), 20000.0f), "Range is 20000");
    assertTrue(!sys.setScanRange("e1", 0.0f), "Zero range rejected");

    assertTrue(sys.setMaxResults("e1", 50), "Set max results");
    assertTrue(!sys.setMaxResults("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxResults("e1", -1), "Negative max rejected");

    assertTrue(sys.setScanDeviation("e1", 0.1f), "Set deviation");
    assertTrue(!sys.setScanDeviation("e1", -0.1f), "Negative deviation rejected");
    assertTrue(!sys.setScanDeviation("e1", 1.1f), "Over 1.0 deviation rejected");
    assertTrue(sys.setScanDeviation("e1", 0.0f), "Zero deviation allowed");
    assertTrue(sys.setScanDeviation("e1", 1.0f), "1.0 deviation allowed");
}

static void testSurveyScannerMissing() {
    std::cout << "\n=== SurveyScanner: Missing ===" << std::endl;
    ecs::World world;
    systems::SurveyScannerSystem sys(&world);

    assertTrue(!sys.startScan("none", "belt_1"), "Start fails on missing");
    assertTrue(!sys.cancelScan("none"), "Cancel fails on missing");
    assertTrue(!sys.addResult("none", "r1", "a1", "V", 10, 5, 3),
               "AddResult fails on missing");
    assertTrue(!sys.removeResult("none", "r1"), "RemoveResult fails on missing");
    assertTrue(!sys.clearResults("none"), "ClearResults fails on missing");
    assertTrue(!sys.setScanDuration("none", 5.0f), "SetDuration fails on missing");
    assertTrue(!sys.setScanRange("none", 100.0f), "SetRange fails on missing");
    assertTrue(!sys.setMaxResults("none", 10), "SetMax fails on missing");
    assertTrue(!sys.setScanDeviation("none", 0.1f), "SetDeviation fails on missing");
    assertTrue(sys.getResultCount("none") == 0, "0 count on missing");
    assertTrue(!sys.hasResult("none", "r1"), "No result on missing");
    assertTrue(approxEqual(sys.getScanProgress("none"), 0.0f), "0 progress on missing");
    assertTrue(!sys.isScanning("none"), "Not scanning on missing");
    assertTrue(!sys.isScanComplete("none"), "Not complete on missing");
    assertTrue(sys.getTotalScansCompleted("none") == 0, "0 total on missing");
    assertTrue(approxEqual(sys.getTotalValueScanned("none"), 0.0f), "0 value on missing");
    assertTrue(sys.getTargetBeltId("none") == "", "Empty target on missing");
    assertTrue(approxEqual(sys.getScanDuration("none"), 0.0f), "0 duration on missing");
    assertTrue(approxEqual(sys.getScanRange("none"), 0.0f), "0 range on missing");
    assertTrue(approxEqual(sys.getResultQuantity("none", "r1"), 0.0f), "0 qty on missing");
    assertTrue(sys.getResultOreType("none", "r1") == "", "Empty ore on missing");
}

void run_survey_scanner_system_tests() {
    testSurveyScannerInit();
    testSurveyScannerStartScan();
    testSurveyScannerScanProgress();
    testSurveyScannerCancelScan();
    testSurveyScannerAddResult();
    testSurveyScannerAddResultValidation();
    testSurveyScannerCapacity();
    testSurveyScannerRemoveResult();
    testSurveyScannerClearResults();
    testSurveyScannerConfiguration();
    testSurveyScannerMissing();
}
