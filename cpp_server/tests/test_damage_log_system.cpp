// Tests for: DamageLogSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/damage_log_system.h"

using namespace atlas;

using DT = components::DamageLog::DamageType;

// ==================== DamageLogSystem Tests ====================

static void testDamageLogInit() {
    std::cout << "\n=== DamageLog: Init ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getEntryCount("ship1") == 0, "Zero entries initially");
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 0.0f), "Zero outgoing initially");
    assertTrue(approxEqual(sys.getTotalIncoming("ship1"), 0.0f), "Zero incoming initially");
    assertTrue(sys.getTotalMisses("ship1") == 0, "Zero misses initially");
    assertTrue(sys.getTotalShots("ship1") == 0, "Zero shots initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testDamageLogOutgoing() {
    std::cout << "\n=== DamageLog: Outgoing ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.logOutgoing("ship1", "enemy1", DT::Kinetic, 500.0f, "AutoCannon"),
               "Log outgoing hit succeeds");
    assertTrue(sys.getEntryCount("ship1") == 1, "1 entry after log");
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 500.0f), "500 outgoing damage");
    assertTrue(sys.getTotalShots("ship1") == 1, "1 shot taken");

    auto entry = sys.getMostRecentEntry("ship1");
    assertTrue(entry.attacker_id == "ship1", "Attacker is ship1");
    assertTrue(entry.defender_id == "enemy1", "Defender is enemy1");
    assertTrue(entry.damage_type == DT::Kinetic, "Damage type Kinetic");
    assertTrue(approxEqual(entry.amount, 500.0f), "Amount 500");
    assertTrue(entry.weapon == "AutoCannon", "Weapon stored");
    assertTrue(entry.hit, "Entry marked as hit");
}

static void testDamageLogIncoming() {
    std::cout << "\n=== DamageLog: Incoming ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.logIncoming("ship1", "attacker1", DT::Thermal, 800.0f, "Beam Laser"),
               "Log incoming hit succeeds");
    assertTrue(approxEqual(sys.getTotalIncoming("ship1"), 800.0f), "800 incoming damage");
    assertTrue(sys.getTotalShots("ship1") == 1, "1 shot received");

    auto entry = sys.getMostRecentEntry("ship1");
    assertTrue(entry.attacker_id == "attacker1", "Attacker stored");
    assertTrue(entry.defender_id == "ship1", "Defender is ship1");
    assertTrue(entry.damage_type == DT::Thermal, "Damage type Thermal");
    assertTrue(approxEqual(entry.amount, 800.0f), "Amount 800");
}

static void testDamageLogMiss() {
    std::cout << "\n=== DamageLog: Miss ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.logOutgoing("ship1", "enemy1", DT::Explosive, 300.0f, "Rocket", false),
               "Log miss succeeds");
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 0.0f), "No outgoing damage on miss");
    assertTrue(sys.getTotalMisses("ship1") == 1, "1 miss recorded");
    assertTrue(sys.getTotalShots("ship1") == 1, "Shot still counted");

    auto entry = sys.getMostRecentEntry("ship1");
    assertTrue(!entry.hit, "Entry marked as miss");
    assertTrue(approxEqual(entry.amount, 0.0f), "Amount is 0 on miss");
}

static void testDamageLogMultipleTypes() {
    std::cout << "\n=== DamageLog: MultipleTypes ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.logOutgoing("ship1", "e1", DT::EM,       200.0f, "Pulse Laser");
    sys.logOutgoing("ship1", "e2", DT::Thermal,  300.0f, "Beam Laser");
    sys.logOutgoing("ship1", "e3", DT::Kinetic,  400.0f, "Railgun");
    sys.logOutgoing("ship1", "e4", DT::Explosive, 500.0f, "Torpedo");

    assertTrue(sys.getEntryCount("ship1") == 4, "4 entries logged");
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 1400.0f), "1400 total outgoing");

    auto latest = sys.getMostRecentEntry("ship1");
    assertTrue(latest.damage_type == DT::Explosive, "Most recent is Explosive");
    assertTrue(approxEqual(latest.amount, 500.0f), "Most recent amount 500");
}

static void testDamageLogMaxEntries() {
    std::cout << "\n=== DamageLog: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    auto* comp = world.getEntity("ship1")->getComponent<components::DamageLog>();
    comp->max_entries = 3;

    for (int i = 0; i < 5; ++i) {
        std::string defender = "enemy" + std::to_string(i);
        sys.logOutgoing("ship1", defender, DT::Kinetic, 100.0f * (i + 1), "Cannon");
    }

    assertTrue(sys.getEntryCount("ship1") == 3, "Entries capped at max_entries");
    assertTrue(sys.getTotalShots("ship1") == 5, "Total shots uncapped");
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 1500.0f), "Total outgoing uncapped");

    // Oldest entry evicted — latest should be enemy4
    auto latest = sys.getMostRecentEntry("ship1");
    assertTrue(latest.defender_id == "enemy4", "Most recent is enemy4");
    assertTrue(comp->entries[0].defender_id == "enemy2", "Oldest remaining is enemy2");
}

static void testDamageLogClearEntries() {
    std::cout << "\n=== DamageLog: ClearEntries ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.logOutgoing("ship1", "e1", DT::Kinetic, 100.0f, "Gun");
    sys.logIncoming("ship1", "a1", DT::Thermal, 200.0f, "Laser");
    assertTrue(sys.getEntryCount("ship1") == 2, "2 entries before clear");

    assertTrue(sys.clearEntries("ship1"), "Clear succeeds");
    assertTrue(sys.getEntryCount("ship1") == 0, "0 entries after clear");

    // Totals preserved after clear
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 100.0f), "Outgoing total preserved");
    assertTrue(approxEqual(sys.getTotalIncoming("ship1"), 200.0f), "Incoming total preserved");
}

static void testDamageLogTimestamps() {
    std::cout << "\n=== DamageLog: Timestamps ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.update(5.0f);
    sys.logOutgoing("ship1", "enemy1", DT::Kinetic, 100.0f, "Cannon");
    auto entry = sys.getMostRecentEntry("ship1");
    assertTrue(entry.timestamp >= 4.9f && entry.timestamp <= 5.1f,
               "Timestamp recorded at ~5s elapsed");
}

static void testDamageLogNoEntries() {
    std::cout << "\n=== DamageLog: NoEntries ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    auto entry = sys.getMostRecentEntry("ship1");
    assertTrue(entry.attacker_id.empty(), "Empty attacker on no entries");
    assertTrue(entry.weapon.empty(), "Empty weapon on no entries");
    assertTrue(approxEqual(entry.amount, 0.0f), "Zero amount on no entries");
}

static void testDamageLogMixed() {
    std::cout << "\n=== DamageLog: Mixed ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.logOutgoing("ship1", "e1", DT::EM,      300.0f, "Pulse",   true);
    sys.logOutgoing("ship1", "e1", DT::Kinetic,  200.0f, "Cannon",  false); // miss
    sys.logIncoming("ship1", "a1", DT::Thermal,  500.0f, "Beam",    true);
    sys.logIncoming("ship1", "a1", DT::Explosive, 150.0f, "Missile", false); // miss

    assertTrue(sys.getTotalShots("ship1") == 4, "4 total shots");
    assertTrue(sys.getTotalMisses("ship1") == 2, "2 misses");
    assertTrue(approxEqual(sys.getTotalOutgoing("ship1"), 300.0f), "300 outgoing (hit only)");
    assertTrue(approxEqual(sys.getTotalIncoming("ship1"), 500.0f), "500 incoming (hit only)");
    assertTrue(sys.getEntryCount("ship1") == 4, "4 entries");
}

static void testDamageLogMissing() {
    std::cout << "\n=== DamageLog: Missing ===" << std::endl;
    ecs::World world;
    systems::DamageLogSystem sys(&world);

    assertTrue(!sys.logOutgoing("nx", "e1", DT::Kinetic, 100.0f, "Gun"),
               "LogOutgoing fails on missing entity");
    assertTrue(!sys.logIncoming("nx", "a1", DT::Thermal, 100.0f, "Laser"),
               "LogIncoming fails on missing entity");
    assertTrue(!sys.clearEntries("nx"), "ClearEntries fails on missing entity");
    assertTrue(sys.getEntryCount("nx") == 0, "0 entries on missing");
    assertTrue(approxEqual(sys.getTotalOutgoing("nx"), 0.0f), "0 outgoing on missing");
    assertTrue(approxEqual(sys.getTotalIncoming("nx"), 0.0f), "0 incoming on missing");
    assertTrue(sys.getTotalMisses("nx") == 0, "0 misses on missing");
    assertTrue(sys.getTotalShots("nx") == 0, "0 shots on missing");
    auto entry = sys.getMostRecentEntry("nx");
    assertTrue(entry.attacker_id.empty(), "Empty entry on missing");
}

void run_damage_log_system_tests() {
    testDamageLogInit();
    testDamageLogOutgoing();
    testDamageLogIncoming();
    testDamageLogMiss();
    testDamageLogMultipleTypes();
    testDamageLogMaxEntries();
    testDamageLogClearEntries();
    testDamageLogTimestamps();
    testDamageLogNoEntries();
    testDamageLogMixed();
    testDamageLogMissing();
}
