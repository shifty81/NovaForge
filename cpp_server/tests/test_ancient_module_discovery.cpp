// Tests for: AncientModuleDiscovery Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/ancient_module_discovery_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== AncientModuleDiscovery Tests ====================

static void testAncientDiscoveryInit() {
    std::cout << "\n=== AncientModuleDiscovery: Init ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    assertTrue(sys.initializeSite("ruin_1", "ancient_site_01", "player_1", 75.0f), "Site initialized");
    assertTrue(approxEqual(sys.getScanRange("ruin_1"), 75.0f), "Scan range set");
    assertTrue(sys.getTotalModules("ruin_1") == 0, "No modules initially");
    assertTrue(!sys.initializeSite("ruin_1", "s", "p"), "Duplicate init fails");
}

static void testAncientDiscoveryAddModule() {
    std::cout << "\n=== AncientModuleDiscovery: Add Module ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    assertTrue(sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.7f, 10.0f, 5000.0f), "Module added");
    assertTrue(sys.getTotalModules("ruin_1") == 1, "1 module total");
    assertTrue(!sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.5f, 5.0f, 1000.0f), "Duplicate rejected");
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 0, "Module is undiscovered");
}

static void testAncientDiscoveryScan() {
    std::cout << "\n=== AncientModuleDiscovery: Scan ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "power_core", 0.5f, 8.0f, 3000.0f);
    assertTrue(sys.beginScan("ruin_1", "mod_1"), "Scan started");
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 1, "Module is scanning");
    assertTrue(!sys.beginScan("ruin_1", "mod_1"), "Can't re-scan");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 2, "Module discovered after scan");
    assertTrue(sys.getDiscoveredCount("ruin_1") == 1, "1 discovered");
}

static void testAncientDiscoveryExtract() {
    std::cout << "\n=== AncientModuleDiscovery: Extract ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "weapon_array", 0.8f, 5.0f, 8000.0f);
    assertTrue(!sys.beginExtraction("ruin_1", "mod_1"), "Can't extract undiscovered");
    sys.beginScan("ruin_1", "mod_1");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f); // Complete scan
    assertTrue(sys.beginExtraction("ruin_1", "mod_1"), "Extraction started");
    for (int i = 0; i < 10; i++) sys.update(1.0f); // Complete extraction (5.0s required)
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 4, "Module extracted");
    assertTrue(sys.getExtractedCount("ruin_1") == 1, "1 extracted");
}

static void testAncientDiscoveryAnalyze() {
    std::cout << "\n=== AncientModuleDiscovery: Analyze ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "nav_system", 0.3f, 3.0f, 2000.0f);
    sys.beginScan("ruin_1", "mod_1");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    sys.beginExtraction("ruin_1", "mod_1");
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.analyzeModule("ruin_1", "mod_1"), "Module analyzed");
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 5, "Module in analyzed state");
    assertTrue(!sys.analyzeModule("ruin_1", "mod_1"), "Can't re-analyze");
}

static void testAncientDiscoveryMultiple() {
    std::cout << "\n=== AncientModuleDiscovery: Multiple Modules ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.5f, 5.0f, 3000.0f);
    sys.addHiddenModule("ruin_1", "mod_2", "power_core", 0.6f, 8.0f, 5000.0f);
    sys.addHiddenModule("ruin_1", "mod_3", "nav_comp", 0.4f, 4.0f, 2000.0f);
    assertTrue(sys.getTotalModules("ruin_1") == 3, "3 modules total");
    sys.beginScan("ruin_1", "mod_1");
    sys.beginScan("ruin_1", "mod_2");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.getDiscoveredCount("ruin_1") == 2, "2 discovered");
}

static void testAncientDiscoveryMaxModules() {
    std::cout << "\n=== AncientModuleDiscovery: Max Modules ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    for (int i = 0; i < 10; i++) {
        std::string id = "mod_" + std::to_string(i);
        assertTrue(sys.addHiddenModule("ruin_1", id, "tech", 0.5f, 5.0f, 1000.0f),
            ("Module " + std::to_string(i) + " added").c_str());
    }
    assertTrue(!sys.addHiddenModule("ruin_1", "mod_extra", "tech", 0.5f, 5.0f, 1000.0f),
        "11th module rejected (max 10)");
}

static void testAncientDiscoveryActive() {
    std::cout << "\n=== AncientModuleDiscovery: Active State ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.5f, 5.0f, 3000.0f);
    sys.beginScan("ruin_1", "mod_1");
    sys.update(2.0f); // Not active, shouldn't advance
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 1, "Still scanning when inactive");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 2, "Discovered when active");
}

static void testAncientDiscoveryStateNames() {
    std::cout << "\n=== AncientModuleDiscovery: State Names ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    assertTrue(sys.stateName(0) == "undiscovered", "State 0 name");
    assertTrue(sys.stateName(1) == "scanning", "State 1 name");
    assertTrue(sys.stateName(2) == "discovered", "State 2 name");
    assertTrue(sys.stateName(5) == "analyzed", "State 5 name");
}

static void testAncientDiscoveryMissing() {
    std::cout << "\n=== AncientModuleDiscovery: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    assertTrue(!sys.initializeSite("nonexistent", "s", "p"), "Init fails on missing");
    assertTrue(!sys.addHiddenModule("nonexistent", "m", "t", 0.5f, 5.0f, 100.0f), "Add fails on missing");
    assertTrue(sys.getDiscoveredCount("nonexistent") == 0, "0 discovered on missing");
    assertTrue(sys.getTotalModules("nonexistent") == 0, "0 modules on missing");
}


void run_ancient_module_discovery_tests() {
    testAncientDiscoveryInit();
    testAncientDiscoveryAddModule();
    testAncientDiscoveryScan();
    testAncientDiscoveryExtract();
    testAncientDiscoveryAnalyze();
    testAncientDiscoveryMultiple();
    testAncientDiscoveryMaxModules();
    testAncientDiscoveryActive();
    testAncientDiscoveryStateNames();
    testAncientDiscoveryMissing();
}
