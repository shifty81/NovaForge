// Tests for: ModuleCapabilitySystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/module_capability_system.h"

using namespace atlas;

// ==================== ModuleCapabilitySystem Tests ====================

static void testModCapInit() {
    std::cout << "\n=== ModuleCap: Init ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_module_count("e1") == 0, "No modules initially");
    assertTrue(sys.get_total_capabilities_registered("e1") == 0, "Zero caps registered");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testModCapAddRemoveModule() {
    std::cout << "\n=== ModuleCap: AddRemoveModule ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.add_module("e1", "mod1", "weapon", 2), "Add mod1");
    assertTrue(sys.add_module("e1", "mod2", "shield", 3), "Add mod2");
    assertTrue(sys.get_module_count("e1") == 2, "Two modules");
    assertTrue(!sys.add_module("e1", "mod1", "armor", 1), "Duplicate rejected");
    assertTrue(!sys.add_module("e1", "", "armor", 1), "Empty id rejected");

    assertTrue(sys.remove_module("e1", "mod1"), "Remove mod1");
    assertTrue(sys.get_module_count("e1") == 1, "One module left");
    assertTrue(!sys.remove_module("e1", "mod1"), "Remove nonexistent fails");
}

static void testModCapAddCapability() {
    std::cout << "\n=== ModuleCap: AddCapability ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);

    assertTrue(sys.add_capability("e1", "mod1", "cap1", "dps_boost", 10.0f), "Add cap1");
    assertTrue(sys.add_capability("e1", "mod1", "cap2", "tracking", 5.0f), "Add cap2");
    assertTrue(sys.get_capability_count("e1", "mod1") == 2, "Two capabilities");
    assertTrue(sys.get_total_capabilities_registered("e1") == 2, "Two total registered");

    // Duplicate
    assertTrue(!sys.add_capability("e1", "mod1", "cap1", "dps_boost", 1.0f), "Dup cap rejected");

    // Add to nonexistent module
    assertTrue(!sys.add_capability("e1", "ghost", "cap3", "x", 1.0f), "Nonexistent module fails");
}

static void testModCapRemoveCapability() {
    std::cout << "\n=== ModuleCap: RemoveCapability ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.add_capability("e1", "mod1", "cap1", "dps_boost", 10.0f);

    assertTrue(sys.remove_capability("e1", "mod1", "cap1"), "Remove cap1");
    assertTrue(sys.get_capability_count("e1", "mod1") == 0, "Zero capabilities");
    assertTrue(!sys.remove_capability("e1", "mod1", "cap1"), "Remove again fails");
    assertTrue(!sys.remove_capability("e1", "mod1", "ghost"), "Remove nonexistent fails");
}

static void testModCapHasType() {
    std::cout << "\n=== ModuleCap: HasType ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.add_module("e1", "mod2", "shield", 3);
    sys.add_capability("e1", "mod1", "cap1", "dps_boost", 10.0f);
    sys.add_capability("e1", "mod2", "cap2", "shield_boost", 20.0f);

    assertTrue(sys.has_capability_type("e1", "dps_boost"), "Has dps_boost");
    assertTrue(sys.has_capability_type("e1", "shield_boost"), "Has shield_boost");
    assertTrue(!sys.has_capability_type("e1", "ecm_jam"), "No ecm_jam");
}

static void testModCapTotalStrength() {
    std::cout << "\n=== ModuleCap: TotalStrength ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.add_module("e1", "mod2", "weapon", 2);
    sys.add_capability("e1", "mod1", "c1", "dps_boost", 10.0f);
    sys.add_capability("e1", "mod1", "c2", "dps_boost", 5.0f);
    sys.add_capability("e1", "mod2", "c3", "dps_boost", 7.0f);

    assertTrue(approxEqual(sys.get_total_capability_strength("e1", "dps_boost"), 22.0f),
               "Total DPS boost is 22");
    assertTrue(approxEqual(sys.get_total_capability_strength("e1", "shield_boost"), 0.0f),
               "No shield_boost strength");
}

static void testModCapEnabled() {
    std::cout << "\n=== ModuleCap: Enabled ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.add_capability("e1", "mod1", "cap1", "dps_boost", 10.0f);

    assertTrue(sys.is_capability_enabled("e1", "mod1", "cap1"), "Enabled by default");
    assertTrue(sys.set_capability_enabled("e1", "mod1", "cap1", false), "Disable cap1");
    assertTrue(!sys.is_capability_enabled("e1", "mod1", "cap1"), "Now disabled");

    // Disabled caps don't count in has/strength
    assertTrue(!sys.has_capability_type("e1", "dps_boost"), "Disabled not found");
    assertTrue(approxEqual(sys.get_total_capability_strength("e1", "dps_boost"), 0.0f),
               "Disabled strength is 0");

    // Re-enable
    assertTrue(sys.set_capability_enabled("e1", "mod1", "cap1", true), "Re-enable");
    assertTrue(sys.has_capability_type("e1", "dps_boost"), "Found again");
    assertTrue(approxEqual(sys.get_total_capability_strength("e1", "dps_boost"), 10.0f),
               "Strength restored");
}

static void testModCapClear() {
    std::cout << "\n=== ModuleCap: Clear ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.add_capability("e1", "mod1", "cap1", "dps_boost", 10.0f);

    assertTrue(sys.clear_modules("e1"), "Clear modules");
    assertTrue(sys.get_module_count("e1") == 0, "Zero modules");
}

static void testModCapMaxCap() {
    std::cout << "\n=== ModuleCap: MaxCap ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    for (int i = 0; i < 30; i++) {
        std::string id = "mod_" + std::to_string(i);
        assertTrue(sys.add_module("e1", id, "type", 1), "Add module within limit");
    }
    assertTrue(!sys.add_module("e1", "mod_30", "type", 1), "Blocked at max");
    assertTrue(sys.get_module_count("e1") == 30, "At max capacity");
}

static void testModCapUpdate() {
    std::cout << "\n=== ModuleCap: Update ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.update(0.016f);
    assertTrue(sys.get_module_count("e1") == 1, "Module persists after update");
}

static void testModCapRemoveModuleWithCaps() {
    std::cout << "\n=== ModuleCap: RemoveModuleWithCaps ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);
    sys.add_capability("e1", "mod1", "cap1", "dps_boost", 10.0f);
    sys.add_capability("e1", "mod1", "cap2", "tracking", 5.0f);

    assertTrue(sys.remove_module("e1", "mod1"), "Remove module with caps");
    assertTrue(sys.get_module_count("e1") == 0, "Zero modules");
    assertTrue(!sys.has_capability_type("e1", "dps_boost"), "Caps gone with module");
}

static void testModCapMissing() {
    std::cout << "\n=== ModuleCap: Missing ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);

    assertTrue(!sys.add_module("no", "m", "t", 1), "add_module fails");
    assertTrue(!sys.remove_module("no", "m"), "remove_module fails");
    assertTrue(!sys.clear_modules("no"), "clear_modules fails");
    assertTrue(!sys.add_capability("no", "m", "c", "t", 1.0f), "add_cap fails");
    assertTrue(!sys.remove_capability("no", "m", "c"), "remove_cap fails");
    assertTrue(!sys.set_capability_enabled("no", "m", "c", true), "set_enabled fails");
    assertTrue(!sys.has_capability_type("no", "x"), "has_type default");
    assertTrue(approxEqual(sys.get_total_capability_strength("no", "x"), 0.0f), "strength default");
    assertTrue(sys.get_module_count("no") == 0, "module_count default");
    assertTrue(sys.get_capability_count("no", "m") == 0, "cap_count default");
    assertTrue(!sys.is_capability_enabled("no", "m", "c"), "is_enabled default");
    assertTrue(sys.get_total_capabilities_registered("no") == 0, "total_caps default");
}

static void testModCapSetEnabledNonexistent() {
    std::cout << "\n=== ModuleCap: SetEnabledNonexistent ===" << std::endl;
    ecs::World world;
    systems::ModuleCapabilitySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_module("e1", "mod1", "weapon", 2);

    assertTrue(!sys.set_capability_enabled("e1", "mod1", "ghost", true), "Nonexistent cap");
    assertTrue(!sys.set_capability_enabled("e1", "ghost_mod", "ghost", true), "Nonexistent mod");
}

void run_module_capability_system_tests() {
    testModCapInit();
    testModCapAddRemoveModule();
    testModCapAddCapability();
    testModCapRemoveCapability();
    testModCapHasType();
    testModCapTotalStrength();
    testModCapEnabled();
    testModCapClear();
    testModCapMaxCap();
    testModCapUpdate();
    testModCapRemoveModuleWithCaps();
    testModCapMissing();
    testModCapSetEnabledNonexistent();
}
