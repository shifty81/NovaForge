// Tests for: ModuleCascadingFailure System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/module_cascading_failure_system.h"

using namespace atlas;

// ==================== ModuleCascadingFailure System Tests ====================

static void testCascadeCreate() {
    std::cout << "\n=== Cascade: Create ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeShip("ship1"), "Init ship succeeds");
    assertTrue(sys.getModuleCount("ship1") == 0, "No modules initially");
    assertTrue(sys.getTotalFailures("ship1") == 0, "No failures initially");
    assertTrue(sys.getCascadeEvents("ship1") == 0, "No cascades initially");
}

static void testCascadeAddModule() {
    std::cout << "\n=== Cascade: AddModule ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    assertTrue(sys.addModule("ship1", "power_core", "Power", 200.0f), "Add power module");
    assertTrue(sys.addModule("ship1", "weapon_1", "Weapon", 100.0f), "Add weapon module");
    assertTrue(sys.getModuleCount("ship1") == 2, "2 modules added");
    assertTrue(!sys.addModule("ship1", "power_core", "Power", 200.0f), "Duplicate module fails");
    assertTrue(sys.isModuleOnline("ship1", "power_core"), "Power module online");
}

static void testCascadeDamage() {
    std::cout << "\n=== Cascade: Damage ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    assertTrue(sys.damageModule("ship1", "weapon_1", 30.0f), "Damage weapon");
    assertTrue(approxEqual(sys.getModuleHP("ship1", "weapon_1"), 70.0f), "HP reduced to 70");
    assertTrue(sys.isModuleOnline("ship1", "weapon_1"), "Still online at 70 HP");
    assertTrue(!sys.isModuleDestroyed("ship1", "weapon_1"), "Not destroyed at 70 HP");
}

static void testCascadeDestroy() {
    std::cout << "\n=== Cascade: Destroy ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    sys.damageModule("ship1", "weapon_1", 100.0f);
    assertTrue(sys.isModuleDestroyed("ship1", "weapon_1"), "Weapon destroyed at 0 HP");
    assertTrue(!sys.isModuleOnline("ship1", "weapon_1"), "Destroyed module offline");
    assertTrue(sys.getTotalFailures("ship1") == 1, "1 failure recorded");
    assertTrue(!sys.damageModule("ship1", "weapon_1", 10.0f), "Can't damage destroyed module");
}

static void testCascadeRepair() {
    std::cout << "\n=== Cascade: Repair ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    sys.damageModule("ship1", "weapon_1", 100.0f);
    assertTrue(sys.isModuleDestroyed("ship1", "weapon_1"), "Module destroyed");
    sys.repairModule("ship1", "weapon_1", 50.0f);
    assertTrue(!sys.isModuleDestroyed("ship1", "weapon_1"), "Module repaired (not destroyed)");
    assertTrue(sys.isModuleOnline("ship1", "weapon_1"), "Repaired module comes online");
    assertTrue(approxEqual(sys.getModuleHP("ship1", "weapon_1"), 50.0f), "HP is 50 after repair");
}

static void testCascadeDependency() {
    std::cout << "\n=== Cascade: Dependency ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "power_core", "Power", 200.0f);
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    assertTrue(sys.addDependency("ship1", "weapon_1", "power_core"), "Add dependency succeeds");
    assertTrue(!sys.addDependency("ship1", "weapon_1", "power_core"), "Duplicate dependency fails");
    assertTrue(!sys.addDependency("ship1", "weapon_1", "nonexistent"), "Dependency on missing fails");
}

static void testCascadePropagation() {
    std::cout << "\n=== Cascade: Propagation ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "power_core", "Power", 200.0f);
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    sys.addModule("ship1", "shield_1", "Shield", 150.0f);
    sys.addDependency("ship1", "weapon_1", "power_core");
    sys.addDependency("ship1", "shield_1", "power_core");
    
    // Destroy power core -> weapons and shields should go offline
    sys.damageModule("ship1", "power_core", 200.0f);
    sys.update(0.0f);
    assertTrue(!sys.isModuleOnline("ship1", "weapon_1"), "Weapon offline after power loss");
    assertTrue(!sys.isModuleOnline("ship1", "shield_1"), "Shield offline after power loss");
    assertTrue(sys.getCascadeEvents("ship1") == 2, "2 cascade events");
}

static void testCascadeChain() {
    std::cout << "\n=== Cascade: Chain ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "reactor", "Power", 200.0f);
    sys.addModule("ship1", "distributor", "Power", 150.0f);
    sys.addModule("ship1", "turret", "Weapon", 80.0f);
    sys.addDependency("ship1", "distributor", "reactor");
    sys.addDependency("ship1", "turret", "distributor");
    
    // Destroy reactor -> distributor offline -> turret offline (chain cascade)
    sys.damageModule("ship1", "reactor", 200.0f);
    sys.update(0.0f);
    assertTrue(!sys.isModuleOnline("ship1", "distributor"), "Distributor offline (direct cascade)");
    assertTrue(!sys.isModuleOnline("ship1", "turret"), "Turret offline (chain cascade)");
    assertTrue(sys.getCascadeEvents("ship1") == 2, "2 cascade events in chain");
}

static void testCascadeOnlineCount() {
    std::cout << "\n=== Cascade: OnlineCount ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "mod1", "Weapon", 100.0f);
    sys.addModule("ship1", "mod2", "Shield", 100.0f);
    sys.addModule("ship1", "mod3", "Engine", 100.0f);
    assertTrue(sys.getOnlineModuleCount("ship1") == 3, "3 modules online");
    sys.damageModule("ship1", "mod1", 100.0f);
    assertTrue(sys.getOnlineModuleCount("ship1") == 2, "2 modules online after destroy");
}

static void testCascadeMissing() {
    std::cout << "\n=== Cascade: Missing ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    assertTrue(!sys.initializeShip("nonexistent"), "Init missing entity fails");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "Module count 0 on missing");
    assertTrue(sys.getTotalFailures("nonexistent") == 0, "Failures 0 on missing");
    assertTrue(!sys.addModule("nonexistent", "mod1", "Weapon", 100.0f), "Add module to missing fails");
    assertTrue(approxEqual(sys.getModuleHP("nonexistent", "mod1"), 0.0f), "HP 0 on missing");
}


void run_module_cascading_failure_system_tests() {
    testCascadeCreate();
    testCascadeAddModule();
    testCascadeDamage();
    testCascadeDestroy();
    testCascadeRepair();
    testCascadeDependency();
    testCascadePropagation();
    testCascadeChain();
    testCascadeOnlineCount();
    testCascadeMissing();
}
