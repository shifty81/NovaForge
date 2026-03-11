// Tests for: CommandBurstSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/command_burst_system.h"

using namespace atlas;
using BT = components::CommandBurstState::BurstType;

// ==================== CommandBurstSystem Tests ====================

static void testBurstInit() {
    std::cout << "\n=== Burst: Init ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    assertTrue(sys.initialize("cmd1", "commander1"), "Init succeeds");
    assertTrue(sys.getBurstCount("cmd1") == 0, "Zero bursts initially");
    assertTrue(sys.getActiveBurstCount("cmd1") == 0, "Zero active initially");
    assertTrue(!sys.isAnyActive("cmd1"), "None active initially");
    assertTrue(sys.getTotalActivations("cmd1") == 0, "Zero activations initially");
    assertTrue(sys.getTotalCycles("cmd1") == 0, "Zero cycles initially");
    assertTrue(sys.getCommanderId("cmd1") == "commander1", "Commander stored");
}

static void testBurstInitFails() {
    std::cout << "\n=== Burst: InitFails ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "cmd1"), "Fails on missing entity");
    world.createEntity("cmd1");
    assertTrue(!sys.initialize("cmd1", ""), "Fails with empty commander_id");
}

static void testBurstAdd() {
    std::cout << "\n=== Burst: Add ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");

    assertTrue(sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, 10.0f),
               "Add shield burst");
    assertTrue(sys.getBurstCount("cmd1") == 1, "One burst");
    assertTrue(sys.hasBurst("cmd1", "b1"), "Burst present");
    assertTrue(!sys.isBurstActive("cmd1", "b1"), "Not active after add");

    assertTrue(sys.addBurst("cmd1", "b2", BT::Navigation, 0.10f, 8000.0f, 8.0f),
               "Add navigation burst");
    assertTrue(sys.getBurstCount("cmd1") == 2, "Two bursts");
}

static void testBurstAddValidation() {
    std::cout << "\n=== Burst: AddValidation ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");

    assertTrue(!sys.addBurst("cmd1", "", BT::Shield, 0.15f, 6000.0f, 10.0f),
               "Empty burst_id rejected");
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Shield, 0.0f, 6000.0f, 10.0f),
               "Zero strength rejected");
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Shield, 1.1f, 6000.0f, 10.0f),
               "Strength > 1 rejected");
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 0.0f, 10.0f),
               "Zero radius rejected");
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, -100.0f, 10.0f),
               "Negative radius rejected");
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, 0.0f),
               "Zero cycle_time rejected");
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, -1.0f),
               "Negative cycle_time rejected");

    // Duplicate
    sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, 10.0f);
    assertTrue(!sys.addBurst("cmd1", "b1", BT::Armor, 0.10f, 6000.0f, 10.0f),
               "Duplicate burst_id rejected");
}

static void testBurstRemove() {
    std::cout << "\n=== Burst: Remove ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");
    sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, 10.0f);
    sys.addBurst("cmd1", "b2", BT::Armor, 0.10f, 6000.0f, 10.0f);

    assertTrue(sys.removeBurst("cmd1", "b1"), "Remove first burst");
    assertTrue(sys.getBurstCount("cmd1") == 1, "One burst remaining");
    assertTrue(!sys.hasBurst("cmd1", "b1"), "Removed burst gone");
    assertTrue(!sys.removeBurst("cmd1", "nonexistent"), "Remove unknown fails");
}

static void testBurstActivate() {
    std::cout << "\n=== Burst: Activate ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");
    sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, 10.0f);
    sys.addBurst("cmd1", "b2", BT::Navigation, 0.10f, 8000.0f, 8.0f);

    assertTrue(sys.activateBurst("cmd1", "b1"), "Activate shield burst");
    assertTrue(sys.isBurstActive("cmd1", "b1"), "Shield burst active");
    assertTrue(sys.isAnyActive("cmd1"), "Some burst active");
    assertTrue(sys.getTotalActivations("cmd1") == 1, "Total activations = 1");
    assertTrue(sys.getActiveBurstCount("cmd1") == 1, "One active burst");

    assertTrue(!sys.activateBurst("cmd1", "b1"), "Cannot activate twice");

    assertTrue(sys.activateBurst("cmd1", "b2"), "Activate navigation burst");
    assertTrue(sys.getActiveBurstCount("cmd1") == 2, "Two active bursts");
    assertTrue(!sys.activateBurst("cmd1", "nonexistent"), "Activate unknown fails");
}

static void testBurstDeactivate() {
    std::cout << "\n=== Burst: Deactivate ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");
    sys.addBurst("cmd1", "b1", BT::Shield, 0.15f, 6000.0f, 10.0f);
    sys.activateBurst("cmd1", "b1");

    assertTrue(sys.isBurstActive("cmd1", "b1"), "Active before deactivate");
    assertTrue(sys.deactivateBurst("cmd1", "b1"), "Deactivate succeeds");
    assertTrue(!sys.isBurstActive("cmd1", "b1"), "Inactive after deactivate");
    assertTrue(!sys.isAnyActive("cmd1"), "None active");

    assertTrue(!sys.deactivateBurst("cmd1", "b1"), "Cannot deactivate twice");
    assertTrue(!sys.deactivateBurst("cmd1", "nonexistent"), "Deactivate unknown fails");
}

static void testBurstCycleTick() {
    std::cout << "\n=== Burst: CycleTick ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");
    sys.addBurst("cmd1", "b1", BT::Mining, 0.20f, 5000.0f, 10.0f);
    sys.activateBurst("cmd1", "b1");

    sys.update(5.0f);
    assertTrue(sys.isBurstActive("cmd1", "b1"), "Still active at 5s");
    assertTrue(sys.getTotalCycles("cmd1") == 0, "No cycles completed");

    sys.update(6.0f);
    assertTrue(!sys.isBurstActive("cmd1", "b1"), "Deactivated after cycle completes");
    assertTrue(sys.getTotalCycles("cmd1") == 1, "One cycle completed");
}

static void testBurstCapacity() {
    std::cout << "\n=== Burst: Capacity ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");
    auto* comp = world.getEntity("cmd1")->getComponent<components::CommandBurstState>();
    comp->max_bursts = 2;

    assertTrue(sys.addBurst("cmd1", "b1", BT::Shield, 0.1f, 5000.0f, 10.0f), "First ok");
    assertTrue(sys.addBurst("cmd1", "b2", BT::Armor, 0.1f, 5000.0f, 10.0f), "Second ok");
    assertTrue(!sys.addBurst("cmd1", "b3", BT::Navigation, 0.1f, 5000.0f, 10.0f),
               "Third rejected at cap");
    assertTrue(sys.getBurstCount("cmd1") == 2, "Still at 2");
}

static void testBurstAllTypes() {
    std::cout << "\n=== Burst: AllTypes ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);
    world.createEntity("cmd1");
    sys.initialize("cmd1", "commander1");

    assertTrue(sys.addBurst("cmd1", "b_shd", BT::Shield,     0.1f, 6000.0f, 10.0f), "Shield type ok");
    assertTrue(sys.addBurst("cmd1", "b_arm", BT::Armor,      0.1f, 6000.0f, 10.0f), "Armor type ok");
    assertTrue(sys.addBurst("cmd1", "b_nav", BT::Navigation, 0.1f, 6000.0f, 10.0f), "Navigation type ok");
    assertTrue(sys.addBurst("cmd1", "b_sen", BT::Sensor,     0.1f, 6000.0f, 10.0f), "Sensor type ok");
    assertTrue(sys.addBurst("cmd1", "b_min", BT::Mining,     0.1f, 6000.0f, 10.0f), "Mining type ok");
    assertTrue(sys.getBurstCount("cmd1") == 5, "All 5 burst types fitted");
}

static void testBurstMissing() {
    std::cout << "\n=== Burst: Missing ===" << std::endl;
    ecs::World world;
    systems::CommandBurstSystem sys(&world);

    assertTrue(!sys.addBurst("nonexistent", "b1", BT::Shield, 0.1f, 5000.0f, 10.0f),
               "Add fails on missing");
    assertTrue(!sys.removeBurst("nonexistent", "b1"), "Remove fails on missing");
    assertTrue(!sys.activateBurst("nonexistent", "b1"), "Activate fails on missing");
    assertTrue(!sys.deactivateBurst("nonexistent", "b1"), "Deactivate fails on missing");
    assertTrue(sys.getBurstCount("nonexistent") == 0, "Zero bursts on missing");
    assertTrue(sys.getActiveBurstCount("nonexistent") == 0, "Zero active on missing");
    assertTrue(!sys.hasBurst("nonexistent", "b1"), "HasBurst false on missing");
    assertTrue(!sys.isBurstActive("nonexistent", "b1"), "Not active on missing");
    assertTrue(!sys.isAnyActive("nonexistent"), "None active on missing");
    assertTrue(sys.getTotalActivations("nonexistent") == 0, "Zero activations on missing");
    assertTrue(sys.getTotalCycles("nonexistent") == 0, "Zero cycles on missing");
    assertTrue(sys.getCommanderId("nonexistent").empty(), "Empty commander on missing");
}

void run_command_burst_system_tests() {
    testBurstInit();
    testBurstInitFails();
    testBurstAdd();
    testBurstAddValidation();
    testBurstRemove();
    testBurstActivate();
    testBurstDeactivate();
    testBurstCycleTick();
    testBurstCapacity();
    testBurstAllTypes();
    testBurstMissing();
}
