// Tests for: SignatureAnalysisSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/signature_analysis_system.h"

using namespace atlas;
using SigType = components::SignatureAnalysisState::SigType;

static void testSigAnalysisInit() {
    std::cout << "\n=== SigAnalysis: Init ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(sys.getSigCount("e1") == 0, "No sigs after init");
    assertTrue(sys.getTotalIdentified("e1") == 0, "No identified after init");
    assertTrue(sys.getIdentifiedCount("e1") == 0, "Identified count 0 after init");
}

static void testSigAnalysisAddSignature() {
    std::cout << "\n=== SigAnalysis: AddSignature ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    assertTrue(sys.addSignature("e1", "ABC-001", "Wormhole Alpha", SigType::Wormhole, 75.0f, 10.0f), "Add sig");
    assertTrue(sys.getSigCount("e1") == 1, "1 sig count");
    assertTrue(sys.hasSignature("e1", "ABC-001"), "Has sig");
    assertTrue(!sys.isIdentified("e1", "ABC-001"), "Not identified yet");
    assertTrue(approxEqual(sys.getScanProgress("e1", "ABC-001"), 0.0f), "0 progress");
    assertTrue(approxEqual(sys.getStrength("e1", "ABC-001"), 75.0f), "Strength 75");
    assertTrue(!sys.addSignature("e1", "ABC-001", "Dup", SigType::Relic, 50.0f, 5.0f), "Duplicate rejected");
    assertTrue(!sys.addSignature("e1", "", "Name", SigType::Data, 50.0f, 5.0f), "Empty sig_id rejected");
    assertTrue(!sys.addSignature("e1", "XYZ-002", "", SigType::Data, 50.0f, 5.0f), "Empty sig_name rejected");
    assertTrue(!sys.addSignature("e1", "XYZ-002", "Name", SigType::Data, -1.0f, 5.0f), "Negative strength rejected");
    assertTrue(!sys.addSignature("e1", "XYZ-002", "Name", SigType::Data, 101.0f, 5.0f), "Strength>100 rejected");
    assertTrue(!sys.addSignature("e1", "XYZ-002", "Name", SigType::Data, 50.0f, 0.0f), "Zero scan_strength rejected");
    assertTrue(!sys.addSignature("e1", "XYZ-002", "Name", SigType::Data, 50.0f, -1.0f), "Negative scan_strength rejected");
    assertTrue(sys.getSigCount("e1") == 1, "Still 1 after invalid adds");
}

static void testSigAnalysisScanTick() {
    std::cout << "\n=== SigAnalysis: ScanTick ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addSignature("e1", "S1", "Relic Site", SigType::Relic, 80.0f, 10.0f);
    assertTrue(sys.scanTick("e1"), "scanTick returns true");
    assertTrue(approxEqual(sys.getScanProgress("e1", "S1"), 10.0f), "10 progress after 1 tick");
    assertTrue(!sys.isIdentified("e1", "S1"), "Not identified at 10%");
    for (int i = 0; i < 9; i++) sys.scanTick("e1");
    assertTrue(approxEqual(sys.getScanProgress("e1", "S1"), 100.0f), "100 progress after 10 ticks");
    assertTrue(sys.isIdentified("e1", "S1"), "Identified at 100%");
    assertTrue(sys.getTotalIdentified("e1") == 1, "Total identified 1");
    assertTrue(sys.getIdentifiedCount("e1") == 1, "getIdentifiedCount 1");
    sys.scanTick("e1");
    assertTrue(approxEqual(sys.getScanProgress("e1", "S1"), 100.0f), "Progress stays 100");
    assertTrue(sys.getTotalIdentified("e1") == 1, "Total identified stays 1");
}

static void testSigAnalysisScanTickMissing() {
    std::cout << "\n=== SigAnalysis: ScanTick Missing ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    assertTrue(!sys.scanTick("nonexistent"), "scanTick on missing entity fails");
}

static void testSigAnalysisRemoveSignature() {
    std::cout << "\n=== SigAnalysis: RemoveSignature ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addSignature("e1", "S1", "Site1", SigType::Data, 50.0f, 5.0f);
    sys.addSignature("e1", "S2", "Site2", SigType::Gas, 60.0f, 8.0f);
    assertTrue(sys.getSigCount("e1") == 2, "2 sigs");
    assertTrue(sys.removeSignature("e1", "S1"), "Remove S1");
    assertTrue(sys.getSigCount("e1") == 1, "1 sig after remove");
    assertTrue(!sys.hasSignature("e1", "S1"), "S1 gone");
    assertTrue(sys.hasSignature("e1", "S2"), "S2 still present");
    assertTrue(!sys.removeSignature("e1", "S1"), "Remove missing sig fails");
    assertTrue(!sys.removeSignature("nonexistent", "S2"), "Remove on missing entity fails");
}

static void testSigAnalysisClearSignatures() {
    std::cout << "\n=== SigAnalysis: ClearSignatures ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addSignature("e1", "S1", "Site1", SigType::Anomaly, 50.0f, 5.0f);
    sys.addSignature("e1", "S2", "Site2", SigType::Combat, 70.0f, 10.0f);
    assertTrue(sys.getSigCount("e1") == 2, "2 sigs before clear");
    assertTrue(sys.clearSignatures("e1"), "ClearSignatures succeeds");
    assertTrue(sys.getSigCount("e1") == 0, "0 sigs after clear");
    assertTrue(!sys.clearSignatures("nonexistent"), "Clear on missing entity fails");
}

static void testSigAnalysisSetScanContribution() {
    std::cout << "\n=== SigAnalysis: SetScanContribution ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addSignature("e1", "S1", "Site1", SigType::Wormhole, 50.0f, 5.0f);
    assertTrue(sys.setScanContribution("e1", 50.0f), "Set contribution succeeds");
    sys.scanTick("e1");
    assertTrue(approxEqual(sys.getScanProgress("e1", "S1"), 50.0f), "50 progress with 50 contribution");
    sys.scanTick("e1");
    assertTrue(approxEqual(sys.getScanProgress("e1", "S1"), 100.0f), "100 progress after 2 ticks with 50 contrib");
    assertTrue(sys.isIdentified("e1", "S1"), "Identified");
    assertTrue(!sys.setScanContribution("nonexistent", 10.0f), "Set contribution on missing entity fails");
}

static void testSigAnalysisGetCountByType() {
    std::cout << "\n=== SigAnalysis: GetCountByType ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addSignature("e1", "S1", "Worm1", SigType::Wormhole, 50.0f, 5.0f);
    sys.addSignature("e1", "S2", "Worm2", SigType::Wormhole, 60.0f, 5.0f);
    sys.addSignature("e1", "S3", "Relic1", SigType::Relic, 70.0f, 5.0f);
    sys.addSignature("e1", "S4", "Data1", SigType::Data, 80.0f, 5.0f);
    assertTrue(sys.getCountByType("e1", SigType::Wormhole) == 2, "2 wormholes");
    assertTrue(sys.getCountByType("e1", SigType::Relic) == 1, "1 relic");
    assertTrue(sys.getCountByType("e1", SigType::Data) == 1, "1 data");
    assertTrue(sys.getCountByType("e1", SigType::Gas) == 0, "0 gas");
    assertTrue(sys.getCountByType("e1", SigType::Anomaly) == 0, "0 anomaly");
    assertTrue(sys.getCountByType("e1", SigType::Combat) == 0, "0 combat");
    assertTrue(sys.getCountByType("nonexistent", SigType::Wormhole) == 0, "0 for missing entity");
}

static void testSigAnalysisMaxCap() {
    std::cout << "\n=== SigAnalysis: MaxCap ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    for (int i = 0; i < 50; i++) {
        std::string id = "S" + std::to_string(i);
        std::string name = "Site" + std::to_string(i);
        assertTrue(sys.addSignature("e1", id, name, SigType::Anomaly, 50.0f, 5.0f), "Add sig " + id);
    }
    assertTrue(sys.getSigCount("e1") == 50, "50 sigs at cap");
    assertTrue(!sys.addSignature("e1", "S_over", "Over", SigType::Anomaly, 50.0f, 5.0f), "51st sig rejected");
}

static void testSigAnalysisMissing() {
    std::cout << "\n=== SigAnalysis: Missing entity ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    const std::string missing = "nonexistent";
    assertTrue(!sys.addSignature(missing, "S1", "Site", SigType::Anomaly, 50.0f, 5.0f), "addSignature missing entity");
    assertTrue(!sys.removeSignature(missing, "S1"), "removeSignature missing entity");
    assertTrue(!sys.clearSignatures(missing), "clearSignatures missing entity");
    assertTrue(!sys.scanTick(missing), "scanTick missing entity");
    assertTrue(!sys.setScanContribution(missing, 10.0f), "setScanContribution missing entity");
    assertTrue(sys.getSigCount(missing) == 0, "getSigCount missing entity");
    assertTrue(sys.getIdentifiedCount(missing) == 0, "getIdentifiedCount missing entity");
    assertTrue(!sys.isIdentified(missing, "S1"), "isIdentified missing entity");
    assertTrue(approxEqual(sys.getScanProgress(missing, "S1"), 0.0f), "getScanProgress missing entity");
    assertTrue(approxEqual(sys.getStrength(missing, "S1"), 0.0f), "getStrength missing entity");
    assertTrue(!sys.hasSignature(missing, "S1"), "hasSignature missing entity");
    assertTrue(sys.getTotalIdentified(missing) == 0, "getTotalIdentified missing entity");
    assertTrue(sys.getCountByType(missing, SigType::Anomaly) == 0, "getCountByType missing entity");
}

static void testSigAnalysisMultipleSigsScanTick() {
    std::cout << "\n=== SigAnalysis: MultipleSigsScanTick ===" << std::endl;
    ecs::World world;
    systems::SignatureAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setScanContribution("e1", 25.0f);
    sys.addSignature("e1", "S1", "Site1", SigType::Data, 50.0f, 5.0f);
    sys.addSignature("e1", "S2", "Site2", SigType::Gas, 60.0f, 5.0f);
    sys.addSignature("e1", "S3", "Site3", SigType::Combat, 70.0f, 5.0f);
    for (int i = 0; i < 4; i++) sys.scanTick("e1");
    assertTrue(sys.isIdentified("e1", "S1"), "S1 identified");
    assertTrue(sys.isIdentified("e1", "S2"), "S2 identified");
    assertTrue(sys.isIdentified("e1", "S3"), "S3 identified");
    assertTrue(sys.getTotalIdentified("e1") == 3, "3 total identified");
    assertTrue(sys.getIdentifiedCount("e1") == 3, "getIdentifiedCount 3");
}

void run_signature_analysis_system_tests() {
    testSigAnalysisInit();
    testSigAnalysisAddSignature();
    testSigAnalysisScanTick();
    testSigAnalysisScanTickMissing();
    testSigAnalysisRemoveSignature();
    testSigAnalysisClearSignatures();
    testSigAnalysisSetScanContribution();
    testSigAnalysisGetCountByType();
    testSigAnalysisMaxCap();
    testSigAnalysisMissing();
    testSigAnalysisMultipleSigsScanTick();
}
