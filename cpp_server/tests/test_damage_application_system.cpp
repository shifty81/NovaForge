// Tests for: DamageApplication System Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/damage_application_system.h"

using namespace atlas;

// ==================== DamageApplication System Tests ====================

static void testDamageApplicationCreate() {
    std::cout << "\n=== DamageApplication: Create ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeDamageTracking("ship1"), "Init damage tracking succeeds");
    assertTrue(sys.getPendingCount("ship1") == 0, "No pending damage initially");
    assertTrue(approxEqual(sys.getTotalApplied("ship1"), 0.0f), "0 total applied");
    assertTrue(sys.getHitsProcessed("ship1") == 0, "0 hits processed");
}

static void testDamageApplicationQueue() {
    std::cout << "\n=== DamageApplication: Queue ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    assertTrue(sys.queueDamage("ship1", "attacker1", 100.0f, 2, 1.0f), "Queue kinetic damage");
    assertTrue(sys.getPendingCount("ship1") == 1, "1 pending");
    assertTrue(sys.hasPendingDamage("ship1"), "Has pending damage");
}

static void testDamageApplicationApplyShield() {
    std::cout << "\n=== DamageApplication: ApplyShield ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    auto* entity = world.getEntity("ship1");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 100.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 100.0f;
    health->hull_hp = 100.0f;

    sys.queueDamage("ship1", "attacker1", 50.0f, 2, 1.0f);  // kinetic
    sys.update(0.1f);

    assertTrue(approxEqual(health->shield_hp, 50.0f), "Shield reduced to 50");
    assertTrue(approxEqual(health->armor_hp, 100.0f), "Armor unchanged");
    assertTrue(approxEqual(health->hull_hp, 100.0f), "Hull unchanged");
    assertTrue(sys.getHitsProcessed("ship1") == 1, "1 hit processed");
    assertTrue(approxEqual(sys.getTotalApplied("ship1"), 50.0f), "50 total applied");
}

static void testDamageApplicationResistance() {
    std::cout << "\n=== DamageApplication: Resistance ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    auto* entity = world.getEntity("ship1");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 100.0f;
    health->shield_em_resist = 0.5f;  // 50% EM resist on shields

    sys.queueDamage("ship1", "attacker1", 100.0f, 0, 1.0f);  // EM damage
    sys.update(0.1f);

    // 100 raw EM × (1 - 0.5) = 50 effective → shield takes 50
    assertTrue(approxEqual(health->shield_hp, 50.0f), "Shield takes 50 after 50% resist");
    assertTrue(sys.getTotalMitigated("ship1") > 0.0f, "Some damage mitigated");
}

static void testDamageApplicationOverflow() {
    std::cout << "\n=== DamageApplication: Overflow ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    auto* entity = world.getEntity("ship1");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 30.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 100.0f;
    health->hull_hp = 100.0f;

    sys.queueDamage("ship1", "attacker1", 80.0f, 2, 1.0f);  // kinetic, 80 damage
    sys.update(0.1f);

    // 80 damage: 30 to shield (depleted), 50 overflows to armor
    assertTrue(approxEqual(health->shield_hp, 0.0f), "Shield depleted");
    assertTrue(approxEqual(health->armor_hp, 50.0f), "Armor takes overflow");
    assertTrue(approxEqual(health->hull_hp, 100.0f), "Hull unchanged");
}

static void testDamageApplicationHullDamage() {
    std::cout << "\n=== DamageApplication: HullDamage ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    auto* entity = world.getEntity("ship1");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 0.0f;
    health->armor_hp = 0.0f;
    health->hull_hp = 100.0f;

    sys.queueDamage("ship1", "attacker1", 60.0f, 3, 1.0f);  // explosive
    sys.update(0.1f);

    assertTrue(approxEqual(health->hull_hp, 40.0f), "Hull reduced to 40");
    assertTrue(sys.getHitsProcessed("ship1") == 1, "1 hit processed");
}

static void testDamageApplicationClearPending() {
    std::cout << "\n=== DamageApplication: ClearPending ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    sys.queueDamage("ship1", "a1", 10.0f, 0, 1.0f);
    sys.queueDamage("ship1", "a2", 20.0f, 1, 2.0f);
    assertTrue(sys.getPendingCount("ship1") == 2, "2 pending");
    assertTrue(sys.clearPending("ship1"), "Clear succeeds");
    assertTrue(sys.getPendingCount("ship1") == 0, "0 pending after clear");
}

static void testDamageApplicationMaxPending() {
    std::cout << "\n=== DamageApplication: MaxPending ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    auto* entity = world.getEntity("ship1");
    auto* da = entity->getComponent<components::DamageApplication>();
    da->max_pending = 3;

    sys.queueDamage("ship1", "a1", 10.0f, 0, 1.0f);
    sys.queueDamage("ship1", "a2", 20.0f, 1, 2.0f);
    sys.queueDamage("ship1", "a3", 30.0f, 2, 3.0f);
    assertTrue(!sys.queueDamage("ship1", "a4", 40.0f, 3, 4.0f), "Max pending enforced");
    assertTrue(sys.getPendingCount("ship1") == 3, "Still 3 pending");
}

static void testDamageApplicationMultipleHits() {
    std::cout << "\n=== DamageApplication: MultipleHits ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeDamageTracking("ship1");
    auto* entity = world.getEntity("ship1");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 200.0f;
    health->armor_hp = 100.0f;
    health->hull_hp = 100.0f;

    sys.queueDamage("ship1", "a1", 50.0f, 0, 1.0f);
    sys.queueDamage("ship1", "a2", 50.0f, 1, 2.0f);
    sys.queueDamage("ship1", "a3", 50.0f, 2, 3.0f);
    sys.update(0.1f);

    assertTrue(sys.getHitsProcessed("ship1") == 3, "3 hits processed");
    assertTrue(approxEqual(health->shield_hp, 50.0f), "Shield reduced by 150");
    assertTrue(sys.getPendingCount("ship1") == 0, "Queue cleared after update");
}

static void testDamageApplicationMissing() {
    std::cout << "\n=== DamageApplication: Missing ===" << std::endl;
    ecs::World world;
    systems::DamageApplicationSystem sys(&world);
    assertTrue(!sys.initializeDamageTracking("nonexistent"), "Init fails on missing");
    assertTrue(!sys.queueDamage("nonexistent", "a1", 10.0f, 0, 1.0f), "Queue fails on missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(approxEqual(sys.getTotalApplied("nonexistent"), 0.0f), "0 applied on missing");
    assertTrue(approxEqual(sys.getTotalMitigated("nonexistent"), 0.0f), "0 mitigated on missing");
    assertTrue(sys.getHitsProcessed("nonexistent") == 0, "0 processed on missing");
    assertTrue(!sys.hasPendingDamage("nonexistent"), "No pending on missing");
    assertTrue(!sys.clearPending("nonexistent"), "Clear fails on missing");
}

void run_damage_application_system_tests() {
    testDamageApplicationCreate();
    testDamageApplicationQueue();
    testDamageApplicationApplyShield();
    testDamageApplicationResistance();
    testDamageApplicationOverflow();
    testDamageApplicationHullDamage();
    testDamageApplicationClearPending();
    testDamageApplicationMaxPending();
    testDamageApplicationMultipleHits();
    testDamageApplicationMissing();
}
