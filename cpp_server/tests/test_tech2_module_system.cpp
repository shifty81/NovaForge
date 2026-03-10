// Tests for: Tech2ModuleSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/tech2_module_system.h"

using namespace atlas;
using Cat = components::Tech2ModuleState::ModuleCategory;

// ==================== Tech2ModuleSystem Tests ====================

static void testTech2ModuleInit() {
    std::cout << "\n=== Tech2Module: Init ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initialize("player1", "capsuleer_001"), "Init succeeds");
    assertTrue(sys.getModuleCount("player1") == 0, "0 modules initially");
    assertTrue(sys.getTotalTech2Acquired("player1") == 0, "0 T2 acquired");
    assertTrue(sys.getTotalFactionAcquired("player1") == 0, "0 faction acquired");
    assertTrue(sys.getTotalDeadspaceAcquired("player1") == 0, "0 deadspace acquired");
}

static void testTech2ModuleRegister() {
    std::cout << "\n=== Tech2Module: Register ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");

    assertTrue(sys.registerModule("player1", "t2_shield_hardener",
                                  "shield_hardener", Cat::Tech2, 5, 1.25f),
               "Register T2 module");
    assertTrue(sys.getModuleCount("player1") == 1, "1 module registered");
}

static void testTech2ModuleRegisterDuplicate() {
    std::cout << "\n=== Tech2Module: RegisterDuplicate ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");

    sys.registerModule("player1", "t2_ab", "afterburner", Cat::Tech2, 5, 1.2f);
    assertTrue(!sys.registerModule("player1", "t2_ab", "afterburner",
                                   Cat::Tech2, 5, 1.2f),
               "Duplicate registration rejected");
    assertTrue(sys.getModuleCount("player1") == 1, "Still 1 module");
}

static void testTech2ModuleAcquire() {
    std::cout << "\n=== Tech2Module: Acquire ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");
    sys.registerModule("player1", "t2_shield_hardener",
                       "shield_hardener", Cat::Tech2, 5, 1.25f);

    assertTrue(sys.acquireModule("player1", "t2_shield_hardener"),
               "Acquire T2 module");
    assertTrue(sys.getTotalTech2Acquired("player1") == 1, "1 T2 acquired");
    assertTrue(sys.getOwnedCount("player1", "t2_shield_hardener") == 1,
               "1 owned");

    // Acquire again
    sys.acquireModule("player1", "t2_shield_hardener");
    assertTrue(sys.getTotalTech2Acquired("player1") == 2, "2 T2 acquired");
    assertTrue(sys.getOwnedCount("player1", "t2_shield_hardener") == 2,
               "2 owned");
}

static void testTech2ModuleAcquireFaction() {
    std::cout << "\n=== Tech2Module: AcquireFaction ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");
    sys.registerModule("player1", "dread_guristas_bcs",
                       "ballistic_control", Cat::Faction, 6, 1.35f);

    assertTrue(sys.acquireModule("player1", "dread_guristas_bcs"),
               "Acquire faction module");
    assertTrue(sys.getTotalFactionAcquired("player1") == 1, "1 faction acquired");
    assertTrue(sys.getTotalTech2Acquired("player1") == 0, "0 T2 acquired");
}

static void testTech2ModuleAcquireDeadspace() {
    std::cout << "\n=== Tech2Module: AcquireDeadspace ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");
    sys.registerModule("player1", "centii_c_type_shield_hardener",
                       "shield_hardener", Cat::Deadspace, 7, 1.45f);

    assertTrue(sys.acquireModule("player1", "centii_c_type_shield_hardener"),
               "Acquire deadspace module");
    assertTrue(sys.getTotalDeadspaceAcquired("player1") == 1,
               "1 deadspace acquired");
}

static void testTech2ModuleAcquireUnknown() {
    std::cout << "\n=== Tech2Module: AcquireUnknown ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");

    assertTrue(!sys.acquireModule("player1", "nonexistent_module"),
               "Acquire unknown fails");
    assertTrue(sys.getTotalTech2Acquired("player1") == 0, "0 T2 acquired");
}

static void testTech2ModuleLootTable() {
    std::cout << "\n=== Tech2Module: LootTable ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");
    sys.registerModule("player1", "faction_mod_x", "mod_x",
                       Cat::Faction, 6, 1.3f);
    sys.addLootEntry("player1", "faction_npc", "faction_mod_x", 0.5f);

    // roll below threshold → drop
    std::string drop = sys.rollLoot("player1", "faction_npc", 0.3f);
    assertTrue(drop == "faction_mod_x", "Drop below threshold");

    // roll above threshold → no drop
    drop = sys.rollLoot("player1", "faction_npc", 0.8f);
    assertTrue(drop.empty(), "No drop above threshold");
}

static void testTech2ModuleLootWrongSite() {
    std::cout << "\n=== Tech2Module: LootWrongSite ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");
    sys.addLootEntry("player1", "deadspace_pocket", "deadspace_mod_y", 1.0f);

    std::string drop = sys.rollLoot("player1", "anomaly", 0.0f);
    assertTrue(drop.empty(), "Wrong site type gives no drop");
}

static void testTech2ModuleMultipleCategories() {
    std::cout << "\n=== Tech2Module: MultipleCategories ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "capsuleer_001");

    sys.registerModule("player1", "t2_mwd", "mwd", Cat::Tech2, 5, 1.2f);
    sys.registerModule("player1", "faction_mwd", "mwd", Cat::Faction, 6, 1.3f);
    sys.registerModule("player1", "deadspace_mwd", "mwd", Cat::Deadspace, 7, 1.4f);

    assertTrue(sys.getModuleCount("player1") == 3, "3 modules registered");
    sys.acquireModule("player1", "t2_mwd");
    sys.acquireModule("player1", "faction_mwd");
    sys.acquireModule("player1", "deadspace_mwd");

    assertTrue(sys.getTotalTech2Acquired("player1") == 1, "1 T2");
    assertTrue(sys.getTotalFactionAcquired("player1") == 1, "1 faction");
    assertTrue(sys.getTotalDeadspaceAcquired("player1") == 1, "1 deadspace");
}

static void testTech2ModuleMissing() {
    std::cout << "\n=== Tech2Module: Missing ===" << std::endl;
    ecs::World world;
    systems::Tech2ModuleSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "owner_x"),
               "Init fails on missing entity");
    assertTrue(!sys.registerModule("nonexistent", "mod_x", "base_mod",
                                   Cat::Tech2, 5, 1.2f),
               "Register fails on missing");
    assertTrue(!sys.addLootEntry("nonexistent", "site", "mod_x", 0.5f),
               "AddLoot fails on missing");
    assertTrue(sys.rollLoot("nonexistent", "site", 0.0f).empty(),
               "RollLoot empty on missing");
    assertTrue(!sys.acquireModule("nonexistent", "mod_x"),
               "Acquire fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getTotalTech2Acquired("nonexistent") == 0, "0 on missing");
}

void run_tech2_module_system_tests() {
    testTech2ModuleInit();
    testTech2ModuleRegister();
    testTech2ModuleRegisterDuplicate();
    testTech2ModuleAcquire();
    testTech2ModuleAcquireFaction();
    testTech2ModuleAcquireDeadspace();
    testTech2ModuleAcquireUnknown();
    testTech2ModuleLootTable();
    testTech2ModuleLootWrongSite();
    testTech2ModuleMultipleCategories();
    testTech2ModuleMissing();
}
