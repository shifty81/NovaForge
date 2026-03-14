// Tests for: FighterSquadronSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/fighter_squadron_system.h"

using namespace atlas;

// ==================== FighterSquadronSystem Tests ====================

static void testFighterSquadronInit() {
    std::cout << "\n=== FighterSquadron: Init ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    assertTrue(sys.initialize("c1"), "Init succeeds");
    assertTrue(sys.getSquadronCount("c1") == 0, "Zero squadrons");
    assertTrue(sys.getLaunchedCount("c1") == 0, "Zero launched");
    assertTrue(sys.getTotalLaunched("c1") == 0, "Zero total launched");
    assertTrue(sys.getTotalRecalled("c1") == 0, "Zero total recalled");
    assertTrue(sys.getTotalKills("c1") == 0, "Zero total kills");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFighterSquadronAdd() {
    std::cout << "\n=== FighterSquadron: Add ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    assertTrue(sys.addSquadron("c1", "sq1", "Dragonfly I", ST::Light, 80, 60),
               "Add light squad");
    assertTrue(sys.addSquadron("c1", "sq2", "Siren I", ST::Support, 100, 50),
               "Add support squad");
    assertTrue(sys.addSquadron("c1", "sq3", "Cyclops I", ST::Heavy, 200, 40),
               "Add heavy squad");
    assertTrue(sys.getSquadronCount("c1") == 3, "3 squadrons");
    assertTrue(sys.hasSquadron("c1", "sq1"), "Has sq1");
    assertTrue(sys.hasSquadron("c1", "sq2"), "Has sq2");
    assertTrue(sys.hasSquadron("c1", "sq3"), "Has sq3");
    assertTrue(!sys.hasSquadron("c1", "sq4"), "No sq4");
}

static void testFighterSquadronAddValidation() {
    std::cout << "\n=== FighterSquadron: AddValidation ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    assertTrue(!sys.addSquadron("c1", "", "Name", ST::Light), "Empty id rejected");
    assertTrue(!sys.addSquadron("c1", "sq1", "", ST::Light), "Empty name rejected");
    assertTrue(!sys.addSquadron("c1", "sq1", "Name", ST::Light, 0, 50),
               "Zero health rejected");
    assertTrue(!sys.addSquadron("c1", "sq1", "Name", ST::Light, -10, 50),
               "Negative health rejected");
    assertTrue(!sys.addSquadron("c1", "sq1", "Name", ST::Light, 100, -1),
               "Negative ammo rejected");
    assertTrue(sys.addSquadron("c1", "sq1", "Name", ST::Light, 100, 0),
               "Zero ammo allowed");
    assertTrue(!sys.addSquadron("c1", "sq1", "Dup", ST::Light),
               "Duplicate id rejected");
}

static void testFighterSquadronMaxCap() {
    std::cout << "\n=== FighterSquadron: MaxCap ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    for (int i = 0; i < 5; i++) {
        std::string id = "sq" + std::to_string(i);
        assertTrue(sys.addSquadron("c1", id, "Squad", ST::Light),
                   "Squadron added within limit");
    }
    assertTrue(!sys.addSquadron("c1", "sq5", "Over", ST::Light),
               "Blocked at max cap");
    assertTrue(sys.getSquadronCount("c1") == 5, "5 squadrons at cap");
}

static void testFighterSquadronLaunchRecall() {
    std::cout << "\n=== FighterSquadron: LaunchRecall ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light);
    sys.addSquadron("c1", "sq2", "Support", ST::Support);

    assertTrue(sys.launchSquadron("c1", "sq1"), "Launch sq1");
    assertTrue(sys.isLaunched("c1", "sq1"), "sq1 is launched");
    assertTrue(!sys.isLaunched("c1", "sq2"), "sq2 not launched");
    assertTrue(sys.getLaunchedCount("c1") == 1, "1 launched");
    assertTrue(sys.getTotalLaunched("c1") == 1, "1 total launched");

    assertTrue(!sys.launchSquadron("c1", "sq1"), "Double launch rejected");
    assertTrue(!sys.launchSquadron("c1", "unknown"), "Launch unknown fails");

    assertTrue(sys.recallSquadron("c1", "sq1"), "Recall sq1");
    assertTrue(!sys.isLaunched("c1", "sq1"), "sq1 recalled");
    assertTrue(sys.getLaunchedCount("c1") == 0, "0 launched");
    assertTrue(sys.getTotalRecalled("c1") == 1, "1 total recalled");

    assertTrue(!sys.recallSquadron("c1", "sq1"), "Double recall rejected");
    assertTrue(!sys.recallSquadron("c1", "unknown"), "Recall unknown fails");
}

static void testFighterSquadronRecallAll() {
    std::cout << "\n=== FighterSquadron: RecallAll ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light);
    sys.addSquadron("c1", "sq2", "Support", ST::Support);
    sys.addSquadron("c1", "sq3", "Heavy", ST::Heavy);
    sys.launchSquadron("c1", "sq1");
    sys.launchSquadron("c1", "sq2");
    sys.launchSquadron("c1", "sq3");

    assertTrue(sys.getLaunchedCount("c1") == 3, "3 launched before recall");
    assertTrue(sys.recallAll("c1"), "RecallAll succeeds");
    assertTrue(sys.getLaunchedCount("c1") == 0, "0 launched after recall all");
    assertTrue(sys.getTotalRecalled("c1") == 3, "3 total recalled");
}

static void testFighterSquadronCombat() {
    std::cout << "\n=== FighterSquadron: Combat ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light, 100, 50);
    sys.launchSquadron("c1", "sq1");

    assertTrue(sys.recordKill("c1", "sq1"), "Record kill");
    assertTrue(sys.recordKill("c1", "sq1"), "Record second kill");
    assertTrue(sys.getTotalKills("c1") == 2, "2 total kills");
    assertTrue(!sys.recordKill("c1", "sq_unlaunched"), "Kill on unlaunched fails");

    assertTrue(sys.applyDamage("c1", "sq1", 30), "Apply 30 damage");
    assertTrue(sys.getHealth("c1", "sq1") == 70, "Health is 70");
    assertTrue(!sys.applyDamage("c1", "sq1", 0), "Zero damage rejected");
    assertTrue(!sys.applyDamage("c1", "sq1", -5), "Negative damage rejected");

    // Kill squadron via damage
    assertTrue(sys.applyDamage("c1", "sq1", 100), "Apply lethal damage");
    assertTrue(sys.getHealth("c1", "sq1") == 0, "Health is 0");
    assertTrue(!sys.isLaunched("c1", "sq1"), "Auto-recalled on death");
    assertTrue(sys.getTotalRecalled("c1") == 1, "1 recall from death");
}

static void testFighterSquadronMaintenance() {
    std::cout << "\n=== FighterSquadron: Maintenance ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light, 100, 80);
    sys.launchSquadron("c1", "sq1");
    sys.applyDamage("c1", "sq1", 110); // kills it, health=0, auto-recalled

    // Repair while docked
    assertTrue(sys.repairSquadron("c1", "sq1", 50), "Repair 50 HP");
    assertTrue(sys.getHealth("c1", "sq1") == 50, "Health is 50");
    assertTrue(sys.repairSquadron("c1", "sq1", 200), "Repair capped at max");
    assertTrue(sys.getHealth("c1", "sq1") == 100, "Health capped at 100");
    assertTrue(!sys.repairSquadron("c1", "sq1", 0), "Zero repair rejected");
    assertTrue(!sys.repairSquadron("c1", "sq1", -10), "Negative repair rejected");

    // Resupply ammo while docked
    // First use some ammo by reducing via component access (simulate usage)
    // For test purposes we just test the resupply interface
    assertTrue(sys.getAmmo("c1", "sq1") == 80, "Ammo starts at 80");
    assertTrue(sys.resupplyAmmo("c1", "sq1", 50), "Resupply capped at max");
    assertTrue(sys.getAmmo("c1", "sq1") == 80, "Ammo still 80 (was full)");

    // Launch blocks maintenance
    sys.launchSquadron("c1", "sq1");
    assertTrue(!sys.repairSquadron("c1", "sq1", 10), "Repair while launched rejected");
    assertTrue(!sys.resupplyAmmo("c1", "sq1", 10), "Resupply while launched rejected");
}

static void testFighterSquadronRemove() {
    std::cout << "\n=== FighterSquadron: Remove ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light);
    sys.addSquadron("c1", "sq2", "Support", ST::Support);

    assertTrue(sys.removeSquadron("c1", "sq1"), "Remove sq1");
    assertTrue(sys.getSquadronCount("c1") == 1, "1 squadron left");
    assertTrue(!sys.hasSquadron("c1", "sq1"), "sq1 removed");
    assertTrue(sys.hasSquadron("c1", "sq2"), "sq2 present");
    assertTrue(!sys.removeSquadron("c1", "sq1"), "Remove nonexistent fails");
}

static void testFighterSquadronClear() {
    std::cout << "\n=== FighterSquadron: Clear ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light);
    sys.addSquadron("c1", "sq2", "Support", ST::Support);

    assertTrue(sys.clearSquadrons("c1"), "Clear succeeds");
    assertTrue(sys.getSquadronCount("c1") == 0, "0 squadrons");
    assertTrue(!sys.hasSquadron("c1", "sq1"), "sq1 gone");
    assertTrue(!sys.hasSquadron("c1", "sq2"), "sq2 gone");
}

static void testFighterSquadronTypes() {
    std::cout << "\n=== FighterSquadron: Types ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light1", ST::Light);
    sys.addSquadron("c1", "sq2", "Light2", ST::Light);
    sys.addSquadron("c1", "sq3", "Support1", ST::Support);
    sys.addSquadron("c1", "sq4", "Heavy1", ST::Heavy);

    assertTrue(sys.getCountByType("c1", ST::Light) == 2, "2 light");
    assertTrue(sys.getCountByType("c1", ST::Support) == 1, "1 support");
    assertTrue(sys.getCountByType("c1", ST::Heavy) == 1, "1 heavy");
}

static void testFighterSquadronDamageNotLaunched() {
    std::cout << "\n=== FighterSquadron: DamageNotLaunched ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using ST = components::FighterSquadronState::SquadronType;
    sys.addSquadron("c1", "sq1", "Light", ST::Light, 100, 50);
    // Not launched — damage and kill should fail
    assertTrue(!sys.applyDamage("c1", "sq1", 10), "Damage on docked rejected");
    assertTrue(!sys.recordKill("c1", "sq1"), "Kill on docked rejected");
}

static void testFighterSquadronMissing() {
    std::cout << "\n=== FighterSquadron: Missing ===" << std::endl;
    ecs::World world;
    systems::FighterSquadronSystem sys(&world);

    using ST = components::FighterSquadronState::SquadronType;
    assertTrue(!sys.addSquadron("none", "sq1", "Name", ST::Light),
               "Add fails on missing");
    assertTrue(!sys.removeSquadron("none", "sq1"), "Remove fails on missing");
    assertTrue(!sys.clearSquadrons("none"), "Clear fails on missing");
    assertTrue(!sys.launchSquadron("none", "sq1"), "Launch fails on missing");
    assertTrue(!sys.recallSquadron("none", "sq1"), "Recall fails on missing");
    assertTrue(!sys.recallAll("none"), "RecallAll fails on missing");
    assertTrue(!sys.recordKill("none", "sq1"), "RecordKill fails on missing");
    assertTrue(!sys.applyDamage("none", "sq1", 10), "ApplyDamage fails on missing");
    assertTrue(!sys.resupplyAmmo("none", "sq1", 10), "Resupply fails on missing");
    assertTrue(!sys.repairSquadron("none", "sq1", 10), "Repair fails on missing");
    assertTrue(sys.getSquadronCount("none") == 0, "0 count on missing");
    assertTrue(sys.getLaunchedCount("none") == 0, "0 launched on missing");
    assertTrue(!sys.isLaunched("none", "sq1"), "Not launched on missing");
    assertTrue(sys.getHealth("none", "sq1") == 0, "0 health on missing");
    assertTrue(sys.getAmmo("none", "sq1") == 0, "0 ammo on missing");
    assertTrue(sys.getTotalLaunched("none") == 0, "0 total launched on missing");
    assertTrue(sys.getTotalRecalled("none") == 0, "0 total recalled on missing");
    assertTrue(sys.getTotalKills("none") == 0, "0 total kills on missing");
    assertTrue(!sys.hasSquadron("none", "sq1"), "No squadron on missing");
    assertTrue(sys.getCountByType("none", ST::Light) == 0,
               "0 type count on missing");
}

void run_fighter_squadron_system_tests() {
    testFighterSquadronInit();
    testFighterSquadronAdd();
    testFighterSquadronAddValidation();
    testFighterSquadronMaxCap();
    testFighterSquadronLaunchRecall();
    testFighterSquadronRecallAll();
    testFighterSquadronCombat();
    testFighterSquadronMaintenance();
    testFighterSquadronRemove();
    testFighterSquadronClear();
    testFighterSquadronTypes();
    testFighterSquadronDamageNotLaunched();
    testFighterSquadronMissing();
}
