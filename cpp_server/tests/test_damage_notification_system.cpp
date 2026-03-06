// Tests for: DamageNotificationSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/damage_notification_system.h"

using namespace atlas;

// ==================== DamageNotificationSystem Tests ====================

static void testDamageRecordIncoming() {
    std::cout << "\n=== DamageNotification: Record Incoming ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    assertTrue(sys.recordIncoming("ship_1", "pirate_1", 150.0f, 0, "Laser", false), "Record incoming succeeds");
    assertTrue(sys.getIncomingCount("ship_1") == 1, "1 incoming entry");
    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 150.0f), "Total taken is 150");
    assertTrue(sys.getCritsTaken("ship_1") == 0, "No crits taken");
}

static void testDamageRecordOutgoing() {
    std::cout << "\n=== DamageNotification: Record Outgoing ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    assertTrue(sys.recordOutgoing("ship_1", "target_1", 200.0f, 2, "Railgun", false), "Record outgoing succeeds");
    assertTrue(sys.getOutgoingCount("ship_1") == 1, "1 outgoing entry");
    assertTrue(approxEqual(sys.getTotalDamageDealt("ship_1"), 200.0f), "Total dealt is 200");
}

static void testDamageCritTracking() {
    std::cout << "\n=== DamageNotification: Crit Tracking ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    sys.recordIncoming("ship_1", "pirate_1", 100.0f, 0, "Laser", true);
    sys.recordIncoming("ship_1", "pirate_1", 50.0f, 1, "Missile", false);
    sys.recordOutgoing("ship_1", "target_1", 300.0f, 3, "Torpedo", true);

    assertTrue(sys.getCritsTaken("ship_1") == 1, "1 crit taken");
    assertTrue(sys.getCritsDealt("ship_1") == 1, "1 crit dealt");
    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 150.0f), "Total taken is 150");
    assertTrue(approxEqual(sys.getTotalDamageDealt("ship_1"), 300.0f), "Total dealt is 300");
}

static void testDamageEntryExpiry() {
    std::cout << "\n=== DamageNotification: Entry Expiry ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* notif = addComp<components::DamageNotification>(e);
    notif->entry_lifetime = 5.0f;

    sys.recordIncoming("ship_1", "pirate_1", 100.0f, 0, "Laser", false);
    assertTrue(sys.getIncomingCount("ship_1") == 1, "1 entry before expiry");

    sys.update(6.0f);  // Past lifetime
    assertTrue(sys.getIncomingCount("ship_1") == 0, "0 entries after expiry");
    // Totals should still accumulate
    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 100.0f), "Total damage still tracked");
}

static void testDamageMaxEntries() {
    std::cout << "\n=== DamageNotification: Max Entries ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* notif = addComp<components::DamageNotification>(e);
    notif->max_entries = 3;

    sys.recordIncoming("ship_1", "a", 10.0f, 0, "L", false);
    sys.recordIncoming("ship_1", "b", 20.0f, 0, "L", false);
    sys.recordIncoming("ship_1", "c", 30.0f, 0, "L", false);
    sys.recordIncoming("ship_1", "d", 40.0f, 0, "L", false);

    assertTrue(sys.getIncomingCount("ship_1") == 3, "Max entries enforced");
    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 100.0f), "Total damage still correct");
}

static void testDamageRecentDPS() {
    std::cout << "\n=== DamageNotification: Recent DPS ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    sys.update(1.0f);  // Advance time
    sys.recordOutgoing("ship_1", "t1", 100.0f, 0, "L", false);
    sys.recordOutgoing("ship_1", "t2", 200.0f, 0, "L", false);

    float dps = sys.getRecentDPS("ship_1", 5.0f);
    assertTrue(approxEqual(dps, 60.0f), "DPS is 300/5 = 60");
}

static void testDamageClearNotifications() {
    std::cout << "\n=== DamageNotification: Clear ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    sys.recordIncoming("ship_1", "a", 100.0f, 0, "L", false);
    sys.recordOutgoing("ship_1", "b", 200.0f, 0, "L", false);

    assertTrue(sys.clearNotifications("ship_1"), "Clear succeeds");
    assertTrue(sys.getIncomingCount("ship_1") == 0, "Incoming cleared");
    assertTrue(sys.getOutgoingCount("ship_1") == 0, "Outgoing cleared");
    // Totals are NOT cleared (they accumulate lifetime)
    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 100.0f), "Total taken preserved");
}

static void testDamageMissingEntity() {
    std::cout << "\n=== DamageNotification: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    assertTrue(!sys.recordIncoming("nope", "a", 100.0f, 0, "L", false), "Record fails for missing");
    assertTrue(!sys.recordOutgoing("nope", "a", 100.0f, 0, "L", false), "Record fails for missing");
    assertTrue(sys.getIncomingCount("nope") == 0, "0 incoming");
    assertTrue(sys.getOutgoingCount("nope") == 0, "0 outgoing");
    assertTrue(approxEqual(sys.getTotalDamageTaken("nope"), 0.0f), "0 damage taken");
    assertTrue(approxEqual(sys.getTotalDamageDealt("nope"), 0.0f), "0 damage dealt");
    assertTrue(sys.getCritsTaken("nope") == 0, "0 crits taken");
    assertTrue(sys.getCritsDealt("nope") == 0, "0 crits dealt");
    assertTrue(approxEqual(sys.getRecentDPS("nope", 5.0f), 0.0f), "0 DPS");
}

static void testDamageNegativeAmount() {
    std::cout << "\n=== DamageNotification: Negative Amount ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    assertTrue(!sys.recordIncoming("ship_1", "a", -50.0f, 0, "L", false), "Negative rejected");
    assertTrue(!sys.recordIncoming("ship_1", "a", 0.0f, 0, "L", false), "Zero rejected");
    assertTrue(!sys.recordOutgoing("ship_1", "a", -50.0f, 0, "L", false), "Negative out rejected");
    assertTrue(sys.getIncomingCount("ship_1") == 0, "No entries recorded");
}

static void testDamageMultipleTypes() {
    std::cout << "\n=== DamageNotification: Multiple Types ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    sys.recordIncoming("ship_1", "a", 100.0f, 0, "EM Laser", false);      // EM
    sys.recordIncoming("ship_1", "b", 200.0f, 1, "Heat Beam", false);     // Thermal
    sys.recordIncoming("ship_1", "c", 300.0f, 2, "Railgun", false);       // Kinetic
    sys.recordIncoming("ship_1", "d", 400.0f, 3, "Torpedo", false);       // Explosive

    assertTrue(sys.getIncomingCount("ship_1") == 4, "4 entries recorded");
    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 1000.0f), "Total is 1000");
}

static void testDamageSummaryAccumulation() {
    std::cout << "\n=== DamageNotification: Summary Accumulation ===" << std::endl;
    ecs::World world;
    systems::DamageNotificationSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::DamageNotification>(e);

    for (int i = 0; i < 5; i++) {
        sys.recordIncoming("ship_1", "pirate", 50.0f, 0, "Laser", i == 2);
        sys.recordOutgoing("ship_1", "target", 75.0f, 2, "Rail", i == 4);
    }

    assertTrue(approxEqual(sys.getTotalDamageTaken("ship_1"), 250.0f), "Total taken = 5×50");
    assertTrue(approxEqual(sys.getTotalDamageDealt("ship_1"), 375.0f), "Total dealt = 5×75");
    assertTrue(sys.getCritsTaken("ship_1") == 1, "1 crit taken");
    assertTrue(sys.getCritsDealt("ship_1") == 1, "1 crit dealt");

    auto* notif = e->getComponent<components::DamageNotification>();
    assertTrue(notif->hits_taken == 5, "5 hits taken");
    assertTrue(notif->hits_dealt == 5, "5 hits dealt");
}

void run_damage_notification_system_tests() {
    testDamageRecordIncoming();
    testDamageRecordOutgoing();
    testDamageCritTracking();
    testDamageEntryExpiry();
    testDamageMaxEntries();
    testDamageRecentDPS();
    testDamageClearNotifications();
    testDamageMissingEntity();
    testDamageNegativeAmount();
    testDamageMultipleTypes();
    testDamageSummaryAccumulation();
}
