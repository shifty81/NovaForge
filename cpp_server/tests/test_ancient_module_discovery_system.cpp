// Tests for: AncientModuleDiscoverySystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/ancient_module_discovery_system.h"

using namespace atlas;

// ==================== AncientModuleDiscoverySystem Tests ====================

static void testAncientModuleInitSite() {
    std::cout << "\n=== AncientModuleDiscovery: InitSite ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");

    assertTrue(sys.initializeSite("site1", "ruins_alpha", "explorer_1", 75.0f), "Init site succeeds");
    assertTrue(sys.getScanRange("site1") == 75.0f, "Scan range is 75");
    assertTrue(sys.getTotalModules("site1") == 0, "Zero modules initially");
    assertTrue(sys.getDiscoveredCount("site1") == 0, "Zero discovered initially");
    assertTrue(sys.getExtractedCount("site1") == 0, "Zero extracted initially");
}

static void testAncientModuleInitInvalid() {
    std::cout << "\n=== AncientModuleDiscovery: InitInvalid ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);

    assertTrue(!sys.initializeSite("missing", "s", "e"), "Missing entity fails");

    world.createEntity("site1");
    assertTrue(sys.initializeSite("site1", "s", "e"), "First init succeeds");
    assertTrue(!sys.initializeSite("site1", "s2", "e2"), "Double init fails");
}

static void testAncientModuleAddHidden() {
    std::cout << "\n=== AncientModuleDiscovery: AddHiddenModule ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");
    sys.initializeSite("site1", "ruins", "explorer");

    assertTrue(sys.addHiddenModule("site1", "mod_a", "precursor_shield", 0.5f, 10.0f, 5000.0f), "Add mod_a");
    assertTrue(sys.getTotalModules("site1") == 1, "1 module added");
    assertTrue(sys.addHiddenModule("site1", "mod_b", "ancient_drive", 0.8f, 15.0f, 8000.0f), "Add mod_b");
    assertTrue(sys.getTotalModules("site1") == 2, "2 modules added");

    // Duplicate rejected
    assertTrue(!sys.addHiddenModule("site1", "mod_a", "precursor_shield", 0.5f, 10.0f, 5000.0f), "Duplicate rejected");
    assertTrue(sys.getTotalModules("site1") == 2, "Still 2 modules");
}

static void testAncientModuleMaxModules() {
    std::cout << "\n=== AncientModuleDiscovery: MaxModules ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");
    sys.initializeSite("site1", "ruins", "explorer");

    // Default max_modules is 10
    for (int i = 0; i < 10; i++) {
        std::string id = "mod_" + std::to_string(i);
        assertTrue(sys.addHiddenModule("site1", id, "tech", 0.5f, 5.0f, 100.0f),
                   ("Add " + id).c_str());
    }
    assertTrue(sys.getTotalModules("site1") == 10, "10 modules (max)");
    assertTrue(!sys.addHiddenModule("site1", "mod_extra", "tech", 0.5f, 5.0f, 100.0f),
               "Max modules rejects extra");
}

static void testAncientModuleScanProgress() {
    std::cout << "\n=== AncientModuleDiscovery: ScanProgress ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");
    sys.initializeSite("site1", "ruins", "explorer");
    sys.addHiddenModule("site1", "mod_a", "tech", 0.5f, 10.0f, 100.0f);

    // Module starts Undiscovered (0)
    assertTrue(sys.getModuleState("site1", "mod_a") == 0, "Initial state is Undiscovered");

    assertTrue(sys.beginScan("site1", "mod_a"), "Begin scan succeeds");
    assertTrue(sys.getModuleState("site1", "mod_a") == 1, "State is Scanning");

    // Cannot scan again
    assertTrue(!sys.beginScan("site1", "mod_a"), "Double scan fails");

    // Activate and update
    sys.setActive("site1", true);
    sys.update(0.5f);
    assertTrue(sys.getModuleState("site1", "mod_a") == 1, "Still scanning at 0.5s");

    sys.update(0.6f); // total 1.1 >= 1.0
    assertTrue(sys.getModuleState("site1", "mod_a") == 2, "Transitioned to Discovered");
    assertTrue(sys.getDiscoveredCount("site1") == 1, "1 discovered");
}

static void testAncientModuleExtraction() {
    std::cout << "\n=== AncientModuleDiscovery: Extraction ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");
    sys.initializeSite("site1", "ruins", "explorer");
    sys.addHiddenModule("site1", "mod_a", "tech", 0.5f, 2.0f, 100.0f);
    sys.setActive("site1", true);

    // Scan to Discovered
    sys.beginScan("site1", "mod_a");
    sys.update(1.5f);
    assertTrue(sys.getModuleState("site1", "mod_a") == 2, "Discovered after scan");

    // Cannot extract from wrong state
    assertTrue(!sys.beginExtraction("site1", "nonexistent"), "Extract nonexistent fails");

    // Begin extraction
    assertTrue(sys.beginExtraction("site1", "mod_a"), "Begin extraction succeeds");
    assertTrue(sys.getModuleState("site1", "mod_a") == 3, "State is Extracting");

    // Cannot extract again
    assertTrue(!sys.beginExtraction("site1", "mod_a"), "Double extract fails");

    // Progress extraction (extract_required=2.0)
    sys.update(1.0f);
    assertTrue(sys.getModuleState("site1", "mod_a") == 3, "Still extracting at 1.0s");
    assertTrue(sys.getExtractedCount("site1") == 0, "Not extracted yet");

    sys.update(1.5f); // total 2.5 >= 2.0
    assertTrue(sys.getModuleState("site1", "mod_a") == 4, "Transitioned to Extracted");
    assertTrue(sys.getExtractedCount("site1") == 1, "1 extracted");
}

static void testAncientModuleAnalysis() {
    std::cout << "\n=== AncientModuleDiscovery: Analysis ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");
    sys.initializeSite("site1", "ruins", "explorer");
    sys.addHiddenModule("site1", "mod_a", "tech", 0.5f, 1.0f, 100.0f);
    sys.setActive("site1", true);

    // Scan → Discovered → Extract → Extracted
    sys.beginScan("site1", "mod_a");
    sys.update(1.5f);
    sys.beginExtraction("site1", "mod_a");
    sys.update(1.5f);
    assertTrue(sys.getModuleState("site1", "mod_a") == 4, "Extracted state");

    // Cannot analyze from wrong state
    assertTrue(!sys.analyzeModule("site1", "nonexistent"), "Analyze nonexistent fails");

    // Analyze
    assertTrue(sys.analyzeModule("site1", "mod_a"), "Analyze succeeds");
    assertTrue(sys.getModuleState("site1", "mod_a") == 5, "State is Analyzed");

    // Cannot analyze twice
    assertTrue(!sys.analyzeModule("site1", "mod_a"), "Double analyze fails");
}

static void testAncientModuleStateNames() {
    std::cout << "\n=== AncientModuleDiscovery: StateNames ===" << std::endl;
    assertTrue(systems::AncientModuleDiscoverySystem::stateName(0) == "undiscovered", "State 0 name");
    assertTrue(systems::AncientModuleDiscoverySystem::stateName(1) == "scanning", "State 1 name");
    assertTrue(systems::AncientModuleDiscoverySystem::stateName(2) == "discovered", "State 2 name");
    assertTrue(systems::AncientModuleDiscoverySystem::stateName(3) == "extracting", "State 3 name");
    assertTrue(systems::AncientModuleDiscoverySystem::stateName(4) == "extracted", "State 4 name");
    assertTrue(systems::AncientModuleDiscoverySystem::stateName(5) == "analyzed", "State 5 name");
}

static void testAncientModuleInactive() {
    std::cout << "\n=== AncientModuleDiscovery: Inactive ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("site1");
    sys.initializeSite("site1", "ruins", "explorer");
    sys.addHiddenModule("site1", "mod_a", "tech", 0.5f, 10.0f, 100.0f);
    sys.beginScan("site1", "mod_a");

    // Inactive by default — update should not progress
    sys.update(5.0f);
    assertTrue(sys.getModuleState("site1", "mod_a") == 1, "Still scanning when inactive");

    sys.setActive("site1", true);
    sys.update(1.5f);
    assertTrue(sys.getModuleState("site1", "mod_a") == 2, "Discovered after activation");
}

static void testAncientModuleMissing() {
    std::cout << "\n=== AncientModuleDiscovery: Missing ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);

    assertTrue(sys.getTotalModules("x") == 0, "Total modules on missing");
    assertTrue(sys.getDiscoveredCount("x") == 0, "Discovered count on missing");
    assertTrue(sys.getExtractedCount("x") == 0, "Extracted count on missing");
    assertTrue(sys.getScanRange("x") == 0.0f, "Scan range on missing");
    assertTrue(sys.getModuleState("x", "m") == 0, "Module state on missing");
    assertTrue(!sys.addHiddenModule("x", "m", "t", 0.5f, 5.0f, 100.0f), "Add module on missing");
    assertTrue(!sys.beginScan("x", "m"), "Scan on missing");
    assertTrue(!sys.beginExtraction("x", "m"), "Extract on missing");
    assertTrue(!sys.analyzeModule("x", "m"), "Analyze on missing");
    assertTrue(!sys.setActive("x", true), "SetActive on missing");
}

void run_ancient_module_discovery_system_tests() {
    testAncientModuleInitSite();
    testAncientModuleInitInvalid();
    testAncientModuleAddHidden();
    testAncientModuleMaxModules();
    testAncientModuleScanProgress();
    testAncientModuleExtraction();
    testAncientModuleAnalysis();
    testAncientModuleStateNames();
    testAncientModuleInactive();
    testAncientModuleMissing();
}
