// Tests for: SignatureAnalysisSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/signature_analysis_system.h"

using namespace atlas;

// ==================== SignatureAnalysisSystem Tests ====================

static void testSigAnalysisCreate() {
    std::cout << "\n=== SignatureAnalysis: Create ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    assertTrue(sys.initialize("scanner1", 2.0f), "Init succeeds");
    assertTrue(sys.getReadingCount("scanner1") == 0, "Zero readings");
    assertTrue(sys.getIdentifiedCount("scanner1") == 0, "Zero identified");
    assertTrue(approxEqual(sys.getScanPower("scanner1"), 2.0f), "Scan power set");
    assertTrue(sys.getTotalScans("scanner1") == 0, "Zero total scans");
    assertTrue(sys.getTotalIdentified("scanner1") == 0, "Zero total identified");
}

static void testSigAnalysisInvalidInit() {
    std::cout << "\n=== SignatureAnalysis: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    assertTrue(!sys.initialize("missing"), "Missing entity fails");
    world.createEntity("s1");
    assertTrue(!sys.initialize("s1", 0.0f), "Zero scan power rejected");
    assertTrue(!sys.initialize("s1", -1.0f), "Negative scan power rejected");
}

static void testSigAnalysisAddReading() {
    std::cout << "\n=== SignatureAnalysis: AddReading ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    assertTrue(sys.addReading("scanner1", "sig_a", "wormhole", 5.0f), "Add reading A");
    assertTrue(sys.addReading("scanner1", "sig_b", "combat", 3.0f), "Add reading B");
    assertTrue(sys.getReadingCount("scanner1") == 2, "2 readings");

    assertTrue(!sys.addReading("scanner1", "sig_a", "data", 2.0f), "Duplicate rejected");
    assertTrue(!sys.addReading("scanner1", "", "wormhole", 5.0f), "Empty ID rejected");
    assertTrue(!sys.addReading("scanner1", "x", "", 5.0f), "Empty type rejected");
    assertTrue(!sys.addReading("scanner1", "x", "wormhole", 0.0f), "Zero strength rejected");
    assertTrue(!sys.addReading("scanner1", "x", "wormhole", -1.0f), "Negative strength rejected");
    assertTrue(!sys.addReading("nonexistent", "x", "wormhole", 5.0f), "Missing entity rejected");
}

static void testSigAnalysisRemoveReading() {
    std::cout << "\n=== SignatureAnalysis: RemoveReading ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");
    sys.addReading("scanner1", "sig_a", "wormhole", 5.0f);
    sys.addReading("scanner1", "sig_b", "combat", 3.0f);

    assertTrue(sys.removeReading("scanner1", "sig_a"), "Remove sig_a succeeds");
    assertTrue(sys.getReadingCount("scanner1") == 1, "1 reading remaining");
    assertTrue(!sys.removeReading("scanner1", "sig_a"), "Double remove fails");
    assertTrue(!sys.removeReading("scanner1", "nonexistent"), "Remove nonexistent fails");
}

static void testSigAnalysisAnalyze() {
    std::cout << "\n=== SignatureAnalysis: Analyze ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1", 1.0f);
    sys.addReading("scanner1", "sig_a", "wormhole", 5.0f);

    // Resolution starts at 0, so analyze should fail
    assertTrue(!sys.analyzeSignature("scanner1", "sig_a"), "Analyze fails - low resolution");
    assertTrue(sys.getTotalScans("scanner1") == 1, "Scan counted");
    assertTrue(!sys.isIdentified("scanner1", "sig_a"), "Not yet identified");

    // Run update to build resolution: analysis_speed(1) * scan_power(1) * 5s = 5.0
    sys.update(5.0f);
    assertTrue(sys.analyzeSignature("scanner1", "sig_a"), "Analyze succeeds after scanning");
    assertTrue(sys.isIdentified("scanner1", "sig_a"), "Now identified");
    assertTrue(sys.getTotalIdentified("scanner1") == 1, "1 total identified");

    // Can't re-analyze
    assertTrue(!sys.analyzeSignature("scanner1", "sig_a"), "Re-analyze rejected");
}

static void testSigAnalysisInvalidOperations() {
    std::cout << "\n=== SignatureAnalysis: InvalidOperations ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    assertTrue(!sys.analyzeSignature("scanner1", "nonexistent"), "Analyze nonexistent fails");
    assertTrue(!sys.analyzeSignature("missing", "sig_a"), "Analyze missing entity fails");
    assertTrue(!sys.setScanPower("scanner1", 0.0f), "Zero power rejected");
    assertTrue(!sys.setScanPower("scanner1", -1.0f), "Negative power rejected");
    assertTrue(!sys.setAnalysisSpeed("scanner1", 0.0f), "Zero speed rejected");
    assertTrue(!sys.setAnalysisSpeed("missing", 1.0f), "Missing entity rejected");
}

static void testSigAnalysisUpdateTick() {
    std::cout << "\n=== SignatureAnalysis: UpdateTick ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1", 2.0f);
    sys.setAnalysisSpeed("scanner1", 3.0f);
    sys.addReading("scanner1", "sig_a", "wormhole", 12.0f);

    // After 2s: resolution = analysis_speed(3) * scan_power(2) * 2 = 12.0
    sys.update(2.0f);
    assertTrue(sys.analyzeSignature("scanner1", "sig_a"), "Identified after 2s at high power");
    assertTrue(sys.isIdentified("scanner1", "sig_a"), "Confirmed identified");
}

static void testSigAnalysisMultipleReadings() {
    std::cout << "\n=== SignatureAnalysis: MultipleReadings ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1", 1.0f);

    for (int i = 0; i < 5; i++) {
        std::string id = "sig_" + std::to_string(i);
        assertTrue(sys.addReading("scanner1", id, "type", 2.0f), ("Add " + id).c_str());
    }
    assertTrue(sys.getReadingCount("scanner1") == 5, "5 readings added");

    // Update enough for resolution to meet strength
    sys.update(2.0f);
    for (int i = 0; i < 5; i++) {
        std::string id = "sig_" + std::to_string(i);
        assertTrue(sys.analyzeSignature("scanner1", id), ("Analyze " + id).c_str());
    }
    assertTrue(sys.getIdentifiedCount("scanner1") == 5, "All 5 identified");
    assertTrue(sys.getTotalIdentified("scanner1") == 5, "5 total identified");
    assertTrue(sys.getTotalScans("scanner1") == 5, "5 total scans");
}

static void testSigAnalysisBoundary() {
    std::cout << "\n=== SignatureAnalysis: Boundary ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1");

    assertTrue(sys.setScanPower("scanner1", 100.0f), "High scan power accepted");
    assertTrue(approxEqual(sys.getScanPower("scanner1"), 100.0f), "Power is 100");
    assertTrue(sys.setAnalysisSpeed("scanner1", 0.001f), "Very low speed accepted");
}

static void testSigAnalysisMissing() {
    std::cout << "\n=== SignatureAnalysis: Missing ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    assertTrue(sys.getReadingCount("x") == 0, "Default readings on missing");
    assertTrue(sys.getIdentifiedCount("x") == 0, "Default identified on missing");
    assertTrue(approxEqual(sys.getScanPower("x"), 0.0f), "Default power on missing");
    assertTrue(approxEqual(sys.getSignalStrength("x", "s"), 0.0f), "Default strength on missing");
    assertTrue(!sys.isIdentified("x", "s"), "Default identified flag on missing");
    assertTrue(sys.getTotalScans("x") == 0, "Default scans on missing");
    assertTrue(sys.getTotalIdentified("x") == 0, "Default total identified on missing");
}

static void testSigAnalysisStats() {
    std::cout << "\n=== SignatureAnalysis: Stats ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1", 1.0f);
    sys.addReading("scanner1", "sig_a", "wormhole", 1.0f);
    sys.addReading("scanner1", "sig_b", "combat", 100.0f);

    sys.update(1.0f);
    sys.analyzeSignature("scanner1", "sig_a"); // succeeds
    sys.analyzeSignature("scanner1", "sig_b"); // fails - signal too strong
    assertTrue(sys.getTotalScans("scanner1") == 2, "2 scan attempts");
    assertTrue(sys.getTotalIdentified("scanner1") == 1, "1 identified");
}

static void testSigAnalysisCombined() {
    std::cout << "\n=== SignatureAnalysis: Combined ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("scanner1");
    sys.initialize("scanner1", 1.0f);

    sys.addReading("scanner1", "sig_a", "wormhole", 3.0f);
    sys.addReading("scanner1", "sig_b", "combat", 6.0f);
    sys.setScanPower("scanner1", 2.0f);
    sys.setAnalysisSpeed("scanner1", 1.5f);

    // After 1s: resolution = 1.5 * 2.0 * 1.0 = 3.0
    sys.update(1.0f);
    assertTrue(sys.analyzeSignature("scanner1", "sig_a"), "sig_a identified at 3.0");
    assertTrue(!sys.analyzeSignature("scanner1", "sig_b"), "sig_b not yet (need 6.0)");

    // After another 1s: resolution = 3.0 + 3.0 = 6.0
    sys.update(1.0f);
    assertTrue(sys.analyzeSignature("scanner1", "sig_b"), "sig_b now identified");

    sys.removeReading("scanner1", "sig_a");
    assertTrue(sys.getReadingCount("scanner1") == 1, "1 reading after remove");
    assertTrue(approxEqual(sys.getSignalStrength("scanner1", "sig_b"), 6.0f), "sig_b strength");
}

void run_signature_analysis_system_tests() {
    testSigAnalysisCreate();
    testSigAnalysisInvalidInit();
    testSigAnalysisAddReading();
    testSigAnalysisRemoveReading();
    testSigAnalysisAnalyze();
    testSigAnalysisInvalidOperations();
    testSigAnalysisUpdateTick();
    testSigAnalysisMultipleReadings();
    testSigAnalysisBoundary();
    testSigAnalysisMissing();
    testSigAnalysisStats();
    testSigAnalysisCombined();
}
