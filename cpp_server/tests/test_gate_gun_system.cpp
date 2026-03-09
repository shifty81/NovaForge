// Tests for: Gate Gun System
#include "test_log.h"
#include "components/combat_components.h"
#include "systems/gate_gun_system.h"

using namespace atlas;

// ==================== Gate Gun System Tests ====================

static void testGateGunCreate() {
    std::cout << "\n=== GateGun: Create ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    assertTrue(sys.initialize("gate1", "stargate_alpha"), "Init succeeds");
    assertTrue(sys.getTargetCount("gate1") == 0, "No targets initially");
    assertTrue(sys.getTotalShotsFired("gate1") == 0, "0 shots fired");
    assertTrue(approxEqual(sys.getTotalDamageDealt("gate1"), 0.0f), "0 damage dealt");
    assertTrue(sys.getTotalKills("gate1") == 0, "0 kills");
    assertTrue(sys.isOnline("gate1"), "Online by default");
}

static void testGateGunAddTarget() {
    std::cout << "\n=== GateGun: AddTarget ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");
    assertTrue(sys.addTarget("gate1", "pirate1", 5.0f, true), "Add pirate target");
    assertTrue(sys.getTargetCount("gate1") == 1, "1 target");
    assertTrue(sys.addTarget("gate1", "pirate2", 3.0f, true), "Add second target");
    assertTrue(sys.getTargetCount("gate1") == 2, "2 targets");
    assertTrue(!sys.addTarget("gate1", "pirate1", 1.0f, true), "Duplicate rejected");
    assertTrue(sys.getTargetCount("gate1") == 2, "Still 2 targets");
}

static void testGateGunRemoveTarget() {
    std::cout << "\n=== GateGun: RemoveTarget ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");
    sys.addTarget("gate1", "pirate1", 5.0f, true);
    assertTrue(sys.removeTarget("gate1", "pirate1"), "Remove succeeds");
    assertTrue(sys.getTargetCount("gate1") == 0, "0 targets after remove");
    assertTrue(!sys.removeTarget("gate1", "pirate1"), "Double remove fails");
}

static void testGateGunFiring() {
    std::cout << "\n=== GateGun: Firing ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");
    sys.addTarget("gate1", "pirate1", 5.0f, true);
    // Default cycle_time = 3.0, so 3.1s should fire once
    sys.update(3.1f);
    assertTrue(sys.getTotalShotsFired("gate1") == 1, "1 shot after 3.1s");
    assertTrue(approxEqual(sys.getTotalDamageDealt("gate1"), 500.0f), "500 damage dealt");
    // Another cycle
    sys.update(3.0f);
    assertTrue(sys.getTotalShotsFired("gate1") == 2, "2 shots after 6.1s");
}

static void testGateGunMaxTargets() {
    std::cout << "\n=== GateGun: MaxTargets ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");

    auto* entity = world.getEntity("gate1");
    auto* gun = entity->getComponent<components::GateGunState>();
    gun->max_targets = 2;

    sys.addTarget("gate1", "t1", 1.0f, true);
    sys.addTarget("gate1", "t2", 2.0f, true);
    assertTrue(!sys.addTarget("gate1", "t3", 3.0f, true), "3rd target rejected");
    assertTrue(sys.getTargetCount("gate1") == 2, "Still 2 targets");
}

static void testGateGunOnlineOffline() {
    std::cout << "\n=== GateGun: Online/Offline ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");
    sys.addTarget("gate1", "pirate1", 5.0f, true);
    assertTrue(sys.setOnline("gate1", false), "Set offline succeeds");
    assertTrue(!sys.isOnline("gate1"), "Now offline");
    assertTrue(sys.getTargetCount("gate1") == 0, "Targets cleared on offline");
    assertTrue(!sys.addTarget("gate1", "pirate2", 1.0f, true), "Can't add target when offline");
    assertTrue(sys.setOnline("gate1", true), "Set online succeeds");
    assertTrue(sys.isOnline("gate1"), "Now online");
}

static void testGateGunDamageAtRange() {
    std::cout << "\n=== GateGun: DamageAtRange ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");
    // Default optimal=100, falloff=50, damage=500
    assertTrue(approxEqual(sys.getDamageAtRange("gate1", 50.0f), 500.0f), "Full damage within optimal");
    assertTrue(approxEqual(sys.getDamageAtRange("gate1", 100.0f), 500.0f), "Full damage at optimal");
    assertTrue(approxEqual(sys.getDamageAtRange("gate1", 125.0f), 250.0f), "Half damage at mid-falloff");
    assertTrue(approxEqual(sys.getDamageAtRange("gate1", 150.0f), 0.0f), "0 damage at max range");
    assertTrue(approxEqual(sys.getDamageAtRange("gate1", 200.0f), 0.0f), "0 damage beyond max range");
}

static void testGateGunNoTargetNoFire() {
    std::cout << "\n=== GateGun: NoTargetNoFire ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    world.createEntity("gate1");
    sys.initialize("gate1");
    sys.update(10.0f);
    assertTrue(sys.getTotalShotsFired("gate1") == 0, "No shots without targets");
}

static void testGateGunMissing() {
    std::cout << "\n=== GateGun: Missing ===" << std::endl;
    ecs::World world;
    systems::GateGunSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addTarget("nonexistent", "t1"), "Add target fails on missing");
    assertTrue(!sys.removeTarget("nonexistent", "t1"), "Remove target fails on missing");
    assertTrue(sys.getTargetCount("nonexistent") == 0, "0 targets on missing");
    assertTrue(sys.getTotalShotsFired("nonexistent") == 0, "0 shots on missing");
    assertTrue(approxEqual(sys.getTotalDamageDealt("nonexistent"), 0.0f), "0 damage on missing");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(!sys.setOnline("nonexistent", true), "SetOnline fails on missing");
    assertTrue(!sys.isOnline("nonexistent"), "Not online on missing");
    assertTrue(approxEqual(sys.getDamageAtRange("nonexistent", 50.0f), 0.0f), "0 damage on missing");
}

void run_gate_gun_system_tests() {
    testGateGunCreate();
    testGateGunAddTarget();
    testGateGunRemoveTarget();
    testGateGunFiring();
    testGateGunMaxTargets();
    testGateGunOnlineOffline();
    testGateGunDamageAtRange();
    testGateGunNoTargetNoFire();
    testGateGunMissing();
}
