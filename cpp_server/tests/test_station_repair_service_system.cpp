// Tests for: StationRepairService System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/station_repair_service_system.h"

using namespace atlas;

// ==================== StationRepairService System Tests ====================

static void testRepairServiceCreate() {
    std::cout << "\n=== StationRepairService: Create ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeRepair("ship1", "station_1", 100.0f, 2.0f), "Init repair succeeds");
    assertTrue(!sys.isRepairing("ship1"), "Not repairing initially");
    assertTrue(!sys.isRepairComplete("ship1"), "Not complete initially");
    assertTrue(sys.getRepairPhase("ship1") == 0, "Phase = Idle (0)");
}

static void testRepairServiceStart() {
    std::cout << "\n=== StationRepairService: Start Repair ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1", 50.0f, 1.5f);

    assertTrue(sys.startRepair("ship1", 100.0f, 200.0f, 50.0f), "Start repair succeeds");
    assertTrue(sys.isRepairing("ship1"), "Ship is being repaired");
    assertTrue(sys.getRepairPhase("ship1") == 1, "Phase = RepairingShield (1)");
}

static void testRepairServiceShieldPhase() {
    std::cout << "\n=== StationRepairService: Shield Phase ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1", 50.0f, 1.0f);
    sys.startRepair("ship1", 100.0f, 0.0f, 0.0f);

    // 50 HP/s * 2s = 100 HP repaired
    for (int i = 0; i < 20; ++i) sys.update(0.1f);

    assertTrue(sys.isRepairComplete("ship1"), "Shield-only repair complete");
    assertTrue(approxEqual(sys.getTotalHPRepaired("ship1"), 100.0f), "100 HP repaired");
    assertTrue(approxEqual(sys.getTotalCost("ship1"), 100.0f), "Cost = 100 * 1.0 = 100");
}

static void testRepairServiceFullRepair() {
    std::cout << "\n=== StationRepairService: Full Repair (All Layers) ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1", 100.0f, 2.0f);
    sys.startRepair("ship1", 50.0f, 100.0f, 50.0f);

    // 100 HP/s: 0.5s shield, 1.0s armor, 0.5s hull = 2.0s total
    for (int i = 0; i < 20; ++i) sys.update(0.1f);

    assertTrue(sys.isRepairComplete("ship1"), "Full repair complete");
    assertTrue(approxEqual(sys.getTotalHPRepaired("ship1"), 200.0f), "200 HP repaired total");
    assertTrue(approxEqual(sys.getTotalCost("ship1"), 400.0f), "Cost = 200 * 2.0 = 400");
}

static void testRepairServiceArmorOnly() {
    std::cout << "\n=== StationRepairService: Armor Only (Skip Shield) ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1", 50.0f, 1.0f);
    sys.startRepair("ship1", 0.0f, 100.0f, 0.0f);

    assertTrue(sys.getRepairPhase("ship1") == 2, "Starts at RepairingArmor (2)");

    for (int i = 0; i < 20; ++i) sys.update(0.1f);

    assertTrue(sys.isRepairComplete("ship1"), "Armor-only repair complete");
    assertTrue(approxEqual(sys.getTotalHPRepaired("ship1"), 100.0f), "100 HP repaired");
}

static void testRepairServiceCancel() {
    std::cout << "\n=== StationRepairService: Cancel ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1");
    sys.startRepair("ship1", 100.0f, 100.0f, 100.0f);

    // Partial repair (1 tick)
    sys.update(0.1f);
    assertTrue(sys.isRepairing("ship1"), "Repairing before cancel");

    assertTrue(sys.cancelRepair("ship1"), "Cancel succeeds");
    assertTrue(!sys.isRepairing("ship1"), "Not repairing after cancel");
    assertTrue(sys.getRepairPhase("ship1") == 0, "Back to Idle");
}

static void testRepairServiceNoDamage() {
    std::cout << "\n=== StationRepairService: No Damage ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1");

    assertTrue(sys.startRepair("ship1", 0.0f, 0.0f, 0.0f), "Start with no damage");
    assertTrue(sys.isRepairComplete("ship1"), "Immediately complete");
}

static void testRepairServiceDuplicateInit() {
    std::cout << "\n=== StationRepairService: Duplicate Init ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeRepair("ship1", "station_1"), "First init succeeds");
    assertTrue(!sys.initializeRepair("ship1", "station_2"), "Duplicate init rejected");
}

static void testRepairServiceMissing() {
    std::cout << "\n=== StationRepairService: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    assertTrue(!sys.initializeRepair("nonexistent", "station_1"), "Init fails for missing entity");
    assertTrue(!sys.isRepairing("nonexistent"), "Not repairing for missing entity");
    assertTrue(sys.getRepairPhase("nonexistent") == -1, "Phase = -1 for missing");
    assertTrue(approxEqual(sys.getTotalCost("nonexistent"), 0.0f), "Cost = 0 for missing");
}

static void testRepairServiceHullOnly() {
    std::cout << "\n=== StationRepairService: Hull Only ===" << std::endl;
    ecs::World world;
    systems::StationRepairServiceSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRepair("ship1", "station_1", 25.0f, 3.0f);
    sys.startRepair("ship1", 0.0f, 0.0f, 50.0f);

    assertTrue(sys.getRepairPhase("ship1") == 3, "Starts at RepairingHull (3)");

    // 25 HP/s * 2s = 50 HP
    for (int i = 0; i < 20; ++i) sys.update(0.1f);

    assertTrue(sys.isRepairComplete("ship1"), "Hull-only repair complete");
    assertTrue(approxEqual(sys.getTotalHPRepaired("ship1"), 50.0f), "50 HP repaired");
    assertTrue(approxEqual(sys.getTotalCost("ship1"), 150.0f), "Cost = 50 * 3.0 = 150");
}

void run_station_repair_service_system_tests() {
    testRepairServiceCreate();
    testRepairServiceStart();
    testRepairServiceShieldPhase();
    testRepairServiceFullRepair();
    testRepairServiceArmorOnly();
    testRepairServiceCancel();
    testRepairServiceNoDamage();
    testRepairServiceDuplicateInit();
    testRepairServiceMissing();
    testRepairServiceHullOnly();
}
