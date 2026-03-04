// Tests for: Mod Manifest System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/mod_manifest_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mod Manifest System Tests ====================

static void testModManifestDefaults() {
    std::cout << "\n=== Mod Manifest Defaults ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);
    assertTrue(modSys.getModCount() == 0, "No mods registered initially");
    assertTrue(modSys.getEnabledModCount() == 0, "No mods enabled initially");
    assertTrue(modSys.validateAll(), "Empty registry is valid");
}

static void testModManifestRegister() {
    std::cout << "\n=== Mod Manifest Register ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    assertTrue(modSys.registerMod("core_expansion", "Core Expansion", "1.0.0", "NovaForge Team"), "Register succeeds");
    assertTrue(modSys.isModRegistered("core_expansion"), "Mod is registered");
    assertTrue(modSys.getModCount() == 1, "One mod registered");
    assertTrue(modSys.isModEnabled("core_expansion"), "Mod enabled by default");
    assertTrue(modSys.getModVersion("core_expansion") == "1.0.0", "Version matches");
}

static void testModManifestDuplicate() {
    std::cout << "\n=== Mod Manifest Duplicate ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("mod_a", "Mod A", "1.0.0", "Author A");
    assertTrue(!modSys.registerMod("mod_a", "Mod A Dupe", "2.0.0", "Author B"), "Duplicate mod ID rejected");
    assertTrue(modSys.getModCount() == 1, "Still one mod");
}

static void testModManifestUnregister() {
    std::cout << "\n=== Mod Manifest Unregister ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("temp_mod", "Temp Mod", "0.1.0", "Tester");
    assertTrue(modSys.unregisterMod("temp_mod"), "Unregister succeeds");
    assertTrue(!modSys.isModRegistered("temp_mod"), "Mod no longer registered");
    assertTrue(modSys.getModCount() == 0, "Zero mods after unregister");
}

static void testModManifestDependencies() {
    std::cout << "\n=== Mod Manifest Dependencies ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("base_lib", "Base Library", "1.0.0", "Core Team");
    modSys.registerMod("addon_pack", "Addon Pack", "1.0.0", "Addon Team", {"base_lib"});

    assertTrue(modSys.areDependenciesMet("addon_pack"), "Dependencies met when base_lib present");
    assertTrue(modSys.validateAll(), "All valid with dependencies satisfied");

    // Register mod with unmet dependency
    modSys.registerMod("broken_mod", "Broken Mod", "1.0.0", "Nobody", {"missing_lib"});
    assertTrue(!modSys.areDependenciesMet("broken_mod"), "Unmet dependency detected");
    assertTrue(!modSys.validateAll(), "Validation fails with unmet dependency");
}

static void testModManifestLoadOrder() {
    std::cout << "\n=== Mod Manifest Load Order ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("foundation", "Foundation", "1.0.0", "Core");
    modSys.registerMod("graphics", "Graphics Pack", "1.0.0", "Art", {"foundation"});
    modSys.registerMod("gameplay", "Gameplay Mod", "1.0.0", "Design", {"foundation"});

    auto order = modSys.getLoadOrder();
    assertTrue(order.size() == 3, "All 3 mods in load order");
    // foundation must come before graphics and gameplay
    int foundIdx = -1, gfxIdx = -1, gameIdx = -1;
    for (size_t i = 0; i < order.size(); i++) {
        if (order[i] == "foundation") foundIdx = static_cast<int>(i);
        if (order[i] == "graphics") gfxIdx = static_cast<int>(i);
        if (order[i] == "gameplay") gameIdx = static_cast<int>(i);
    }
    assertTrue(foundIdx < gfxIdx, "Foundation loads before Graphics");
    assertTrue(foundIdx < gameIdx, "Foundation loads before Gameplay");
}

static void testModManifestEnableDisable() {
    std::cout << "\n=== Mod Manifest Enable/Disable ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("toggle_mod", "Toggle Mod", "1.0.0", "Dev");
    assertTrue(modSys.isModEnabled("toggle_mod"), "Enabled by default");

    assertTrue(modSys.setModEnabled("toggle_mod", false), "Disable succeeds");
    assertTrue(!modSys.isModEnabled("toggle_mod"), "Mod is disabled");
    assertTrue(modSys.getEnabledModCount() == 0, "Zero enabled mods");

    assertTrue(modSys.setModEnabled("toggle_mod", true), "Re-enable succeeds");
    assertTrue(modSys.isModEnabled("toggle_mod"), "Mod is enabled again");
}

static void testModManifestEmptyId() {
    std::cout << "\n=== Mod Manifest Empty ID ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    assertTrue(!modSys.registerMod("", "No ID", "1.0.0", "Nobody"), "Empty mod ID rejected");
    assertTrue(modSys.getModCount() == 0, "No mods registered with empty ID");
}


void run_mod_manifest_system_tests() {
    testModManifestDefaults();
    testModManifestRegister();
    testModManifestDuplicate();
    testModManifestUnregister();
    testModManifestDependencies();
    testModManifestLoadOrder();
    testModManifestEnableDisable();
    testModManifestEmptyId();
}
