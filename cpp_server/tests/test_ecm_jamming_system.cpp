// Tests for: EcmJammingSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/ecm_jamming_system.h"

using namespace atlas;

// ==================== EcmJammingSystem Tests ====================

static void testEcmJammingInit() {
    std::cout << "\n=== EcmJamming: Init ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 15.0f), "Init succeeds");
    assertTrue(sys.getJammerCount("ship1") == 0, "Zero jammers initially");
    assertTrue(!sys.isJammed("ship1"), "Not jammed initially");
    assertTrue(approxEqual(sys.getSensorStrength("ship1"), 15.0f), "Sensor strength set");
    assertTrue(approxEqual(sys.getTotalJamStrength("ship1"), 0.0f), "Zero jam strength");
    assertTrue(sys.getTotalJamsApplied("ship1") == 0, "Zero jams applied");
    assertTrue(sys.getTotalJamAttempts("ship1") == 0, "Zero jam attempts");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testEcmJammingApplyJammer() {
    std::cout << "\n=== EcmJamming: ApplyJammer ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);

    assertTrue(sys.applyJammer("ship1", "enemy1", 5.0f, 5.0f), "Apply jammer succeeds");
    assertTrue(sys.getJammerCount("ship1") == 1, "1 jammer after apply");
    assertTrue(approxEqual(sys.getTotalJamStrength("ship1"), 5.0f), "Jam strength 5");

    // Duplicate source rejected
    assertTrue(!sys.applyJammer("ship1", "enemy1", 3.0f, 5.0f), "Duplicate source rejected");
    assertTrue(sys.getJammerCount("ship1") == 1, "Still 1 jammer after duplicate");

    // Multiple jammers stack
    assertTrue(sys.applyJammer("ship1", "enemy2", 3.0f, 5.0f), "Second jammer applied");
    assertTrue(sys.getJammerCount("ship1") == 2, "2 jammers after second apply");
    assertTrue(approxEqual(sys.getTotalJamStrength("ship1"), 8.0f), "Combined jam strength 8");
}

static void testEcmJammingValidation() {
    std::cout << "\n=== EcmJamming: Validation ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);

    assertTrue(!sys.applyJammer("ship1", "", 5.0f, 5.0f),   "Empty source_id rejected");
    assertTrue(!sys.applyJammer("ship1", "e1", 0.0f, 5.0f), "Zero jam strength rejected");
    assertTrue(!sys.applyJammer("ship1", "e1", 5.0f, 0.0f), "Zero cycle time rejected");
    assertTrue(!sys.applyJammer("ship1", "e1", -1.0f, 5.0f),"Negative strength rejected");
    assertTrue(sys.getJammerCount("ship1") == 0, "No jammers after failed applies");
}

static void testEcmJammingRemoveJammer() {
    std::cout << "\n=== EcmJamming: RemoveJammer ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);
    sys.applyJammer("ship1", "enemy1", 5.0f, 5.0f);
    sys.applyJammer("ship1", "enemy2", 3.0f, 5.0f);

    assertTrue(sys.removeJammer("ship1", "enemy1"), "Remove jammer succeeds");
    assertTrue(sys.getJammerCount("ship1") == 1, "1 jammer remaining");
    assertTrue(approxEqual(sys.getTotalJamStrength("ship1"), 3.0f), "Jam strength 3 after remove");

    assertTrue(!sys.removeJammer("ship1", "enemy1"), "Remove nonexistent jammer fails");
    assertTrue(!sys.removeJammer("ship1", "nonexistent"), "Remove unknown source fails");
}

static void testEcmJammingClearJammers() {
    std::cout << "\n=== EcmJamming: ClearJammers ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);
    sys.applyJammer("ship1", "e1", 5.0f, 5.0f);
    sys.applyJammer("ship1", "e2", 3.0f, 5.0f);
    assertTrue(sys.getJammerCount("ship1") == 2, "2 jammers before clear");
    assertTrue(sys.clearJammers("ship1"), "Clear succeeds");
    assertTrue(sys.getJammerCount("ship1") == 0, "0 jammers after clear");
    assertTrue(!sys.isJammed("ship1"), "Not jammed after clear");
}

static void testEcmJammingSetSensorStrength() {
    std::cout << "\n=== EcmJamming: SetSensorStrength ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);

    assertTrue(sys.setSensorStrength("ship1", 20.0f), "Set sensor strength succeeds");
    assertTrue(approxEqual(sys.getSensorStrength("ship1"), 20.0f), "Sensor strength updated");

    assertTrue(!sys.setSensorStrength("ship1", 0.0f),   "Zero strength rejected");
    assertTrue(!sys.setSensorStrength("ship1", -5.0f),  "Negative strength rejected");
    assertTrue(approxEqual(sys.getSensorStrength("ship1"), 20.0f), "Strength unchanged after rejection");
}

static void testEcmJammingMaxJammers() {
    std::cout << "\n=== EcmJamming: MaxJammers ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);

    auto* comp = world.getEntity("ship1")->getComponent<components::EcmJammingState>();
    comp->max_jammers = 3;

    assertTrue(sys.applyJammer("ship1", "e1", 1.0f, 5.0f), "First jammer applied");
    assertTrue(sys.applyJammer("ship1", "e2", 1.0f, 5.0f), "Second jammer applied");
    assertTrue(sys.applyJammer("ship1", "e3", 1.0f, 5.0f), "Third jammer applied");
    assertTrue(!sys.applyJammer("ship1", "e4", 1.0f, 5.0f), "Fourth jammer rejected at max");
    assertTrue(sys.getJammerCount("ship1") == 3, "Jammer count capped at 3");
}

static void testEcmJammingCycleCounters() {
    std::cout << "\n=== EcmJamming: CycleCounters ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);
    sys.applyJammer("ship1", "enemy1", 5.0f, 2.0f); // 2s cycle

    // Tick past one full cycle
    sys.update(2.1f);
    assertTrue(sys.getTotalJamAttempts("ship1") == 1, "1 jam attempt after one cycle");
}

static void testEcmJammingJamStateAfterRemove() {
    std::cout << "\n=== EcmJamming: JamStateAfterRemove ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);
    sys.applyJammer("ship1", "enemy1", 5.0f, 5.0f);

    // Force jam state manually
    auto* comp = world.getEntity("ship1")->getComponent<components::EcmJammingState>();
    comp->jammers[0].currently_jamming = true;
    comp->is_jammed = true;

    sys.removeJammer("ship1", "enemy1");
    assertTrue(!sys.isJammed("ship1"), "Not jammed after all jammers removed");
    assertTrue(sys.getJammerCount("ship1") == 0, "0 jammers after remove");
}

static void testEcmJammingTotalJamStrength() {
    std::cout << "\n=== EcmJamming: TotalJamStrength ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10.0f);

    sys.applyJammer("ship1", "e1", 2.5f, 5.0f);
    sys.applyJammer("ship1", "e2", 3.5f, 5.0f);
    sys.applyJammer("ship1", "e3", 4.0f, 5.0f);
    assertTrue(approxEqual(sys.getTotalJamStrength("ship1"), 10.0f), "Total jam strength 10");
}

static void testEcmJammingMissing() {
    std::cout << "\n=== EcmJamming: Missing ===" << std::endl;
    ecs::World world;
    systems::EcmJammingSystem sys(&world);

    assertTrue(!sys.applyJammer("nx", "e1", 5.0f, 5.0f), "ApplyJammer fails on missing entity");
    assertTrue(!sys.removeJammer("nx", "e1"),             "RemoveJammer fails on missing entity");
    assertTrue(!sys.clearJammers("nx"),                   "ClearJammers fails on missing entity");
    assertTrue(!sys.setSensorStrength("nx", 10.0f),       "SetSensorStrength fails on missing");
    assertTrue(!sys.isJammed("nx"),                       "isJammed returns false on missing");
    assertTrue(sys.getJammerCount("nx") == 0,             "0 jammers on missing entity");
    assertTrue(approxEqual(sys.getSensorStrength("nx"), 0.0f), "0 sensor strength on missing");
    assertTrue(approxEqual(sys.getTotalJamStrength("nx"), 0.0f), "0 jam strength on missing");
    assertTrue(sys.getTotalJamsApplied("nx") == 0,        "0 jams applied on missing");
    assertTrue(sys.getTotalJamAttempts("nx") == 0,        "0 jam attempts on missing");
    assertTrue(sys.getTotalLockBreaks("nx") == 0,         "0 lock breaks on missing");
}

void run_ecm_jamming_system_tests() {
    testEcmJammingInit();
    testEcmJammingApplyJammer();
    testEcmJammingValidation();
    testEcmJammingRemoveJammer();
    testEcmJammingClearJammers();
    testEcmJammingSetSensorStrength();
    testEcmJammingMaxJammers();
    testEcmJammingCycleCounters();
    testEcmJammingJamStateAfterRemove();
    testEcmJammingTotalJamStrength();
    testEcmJammingMissing();
}
