// Tests for: Jump Gate Activation System
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/jump_gate_activation_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Jump Gate Activation System Tests ====================

static void testJumpGateActivationCreate() {
    std::cout << "\n=== JumpGateActivation: Create ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    assertTrue(sys.initialize("jga1", "gate_alpha", "sol_b", "gate_beta"), "Init succeeds");
    assertTrue(sys.isReady("jga1"), "Gate is ready initially");
    assertTrue(!sys.isCharging("jga1"), "Not charging initially");
    assertTrue(!sys.isOnCooldown("jga1"), "Not on cooldown initially");
    assertTrue(sys.getTotalJumps("jga1") == 0, "0 jumps initially");
    assertTrue(sys.getCurrentQueue("jga1") == 0, "0 queued initially");
    assertTrue(sys.getDestinationSystem("jga1") == "sol_b", "Destination is sol_b");
    assertTrue(approxEqual(sys.getFuelCost("jga1"), 50.0f), "Default fuel cost 50");
}

static void testJumpGateActivationInitValidation() {
    std::cout << "\n=== JumpGateActivation: InitValidation ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    assertTrue(!sys.initialize("jga1", "", "sol_b", "gate_b"), "Empty gate_id rejected");
    world.createEntity("jga2");
    assertTrue(!sys.initialize("jga2", "gate_a", "", "gate_b"), "Empty dest_system rejected");
    world.createEntity("jga3");
    assertTrue(!sys.initialize("jga3", "gate_a", "sol_b", ""), "Empty dest_gate rejected");
    assertTrue(!sys.initialize("nonexistent", "gate_a", "sol_b", "gate_b"), "Missing entity rejected");
}

static void testJumpGateActivationCharge() {
    std::cout << "\n=== JumpGateActivation: Charge ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    sys.setChargeTime("jga1", 10.0f);
    assertTrue(sys.startCharge("jga1"), "Start charge");
    assertTrue(sys.isCharging("jga1"), "Now charging");
    assertTrue(!sys.isReady("jga1"), "Not ready while charging");
    sys.update(5.0f);
    float progress = sys.getChargeProgress("jga1");
    assertTrue(progress > 0.49f && progress < 0.51f, "50% charged after 5s");
    assertTrue(sys.isCharging("jga1"), "Still charging");
}

static void testJumpGateActivationFullCycle() {
    std::cout << "\n=== JumpGateActivation: FullCycle ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    sys.setChargeTime("jga1", 10.0f);
    sys.setCooldownTime("jga1", 5.0f);
    sys.startCharge("jga1");
    sys.update(11.0f);
    assertTrue(!sys.isCharging("jga1"), "No longer charging");
    assertTrue(sys.isOnCooldown("jga1"), "On cooldown after fire");
    assertTrue(sys.getTotalJumps("jga1") == 1, "1 jump completed");
    sys.update(6.0f);
    assertTrue(!sys.isOnCooldown("jga1"), "Cooldown expired");
    assertTrue(sys.isReady("jga1"), "Ready again");
}

static void testJumpGateActivationCancelCharge() {
    std::cout << "\n=== JumpGateActivation: CancelCharge ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    sys.startCharge("jga1");
    sys.update(3.0f);
    assertTrue(sys.cancelCharge("jga1"), "Cancel charge");
    assertTrue(!sys.isCharging("jga1"), "No longer charging");
    assertTrue(approxEqual(sys.getChargeProgress("jga1"), 0.0f), "Charge reset to 0");
    assertTrue(sys.isReady("jga1"), "Ready after cancel");
    assertTrue(!sys.cancelCharge("jga1"), "Cannot cancel when not charging");
}

static void testJumpGateActivationQueue() {
    std::cout << "\n=== JumpGateActivation: Queue ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    assertTrue(sys.queueShip("jga1"), "Queue ship 1");
    assertTrue(sys.queueShip("jga1"), "Queue ship 2");
    assertTrue(sys.getCurrentQueue("jga1") == 2, "2 ships queued");
    assertTrue(sys.dequeueShip("jga1"), "Dequeue ship");
    assertTrue(sys.getCurrentQueue("jga1") == 1, "1 ship queued");
    for (int i = 1; i < 5; ++i) sys.queueShip("jga1");
    assertTrue(!sys.queueShip("jga1"), "6th ship rejected (max 5)");
}

static void testJumpGateActivationChargeTimeClamp() {
    std::cout << "\n=== JumpGateActivation: ChargeTimeClamp ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    sys.setChargeTime("jga1", 0.1f);
    sys.startCharge("jga1");
    sys.update(1.5f);
    assertTrue(sys.getTotalJumps("jga1") == 1, "Jump completed with clamped charge time");
    sys.setCooldownTime("jga1", 1000.0f);
    sys.update(301.0f);
    assertTrue(sys.isReady("jga1"), "Cooldown expired at clamped 300s");
}

static void testJumpGateActivationFuelCost() {
    std::cout << "\n=== JumpGateActivation: FuelCost ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    assertTrue(sys.setFuelCost("jga1", 100.0f), "Set fuel cost");
    assertTrue(approxEqual(sys.getFuelCost("jga1"), 100.0f), "Fuel cost is 100");
    sys.setFuelCost("jga1", -10.0f);
    assertTrue(approxEqual(sys.getFuelCost("jga1"), 0.0f), "Fuel cost clamped to 0");
}

static void testJumpGateActivationCannotChargeDuringCooldown() {
    std::cout << "\n=== JumpGateActivation: CannotChargeDuringCooldown ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    world.createEntity("jga1");
    sys.initialize("jga1", "gate_a", "sol_b", "gate_b");
    sys.setChargeTime("jga1", 1.0f);
    sys.setCooldownTime("jga1", 10.0f);
    sys.startCharge("jga1");
    sys.update(2.0f);
    assertTrue(sys.isOnCooldown("jga1"), "On cooldown");
    assertTrue(!sys.startCharge("jga1"), "Cannot charge during cooldown");
}

static void testJumpGateActivationMissing() {
    std::cout << "\n=== JumpGateActivation: Missing ===" << std::endl;
    ecs::World world;
    systems::JumpGateActivationSystem sys(&world);
    assertTrue(!sys.startCharge("nonexistent"), "Charge fails on missing");
    assertTrue(!sys.cancelCharge("nonexistent"), "Cancel fails on missing");
    assertTrue(!sys.queueShip("nonexistent"), "Queue fails on missing");
    assertTrue(!sys.dequeueShip("nonexistent"), "Dequeue fails on missing");
    assertTrue(!sys.setChargeTime("nonexistent", 5.0f), "Charge time fails on missing");
    assertTrue(!sys.setCooldownTime("nonexistent", 5.0f), "Cooldown time fails on missing");
    assertTrue(!sys.setFuelCost("nonexistent", 100.0f), "Fuel cost fails on missing");
    assertTrue(approxEqual(sys.getChargeProgress("nonexistent"), 0.0f), "0 charge on missing");
    assertTrue(approxEqual(sys.getRemainingCooldown("nonexistent"), 0.0f), "0 cooldown on missing");
    assertTrue(approxEqual(sys.getFuelCost("nonexistent"), 0.0f), "0 fuel on missing");
    assertTrue(!sys.isReady("nonexistent"), "Not ready on missing");
    assertTrue(!sys.isCharging("nonexistent"), "Not charging on missing");
    assertTrue(!sys.isOnCooldown("nonexistent"), "Not cooling on missing");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "0 jumps on missing");
    assertTrue(sys.getCurrentQueue("nonexistent") == 0, "0 queue on missing");
    assertTrue(sys.getDestinationSystem("nonexistent").empty(), "Empty dest on missing");
}

void run_jump_gate_activation_system_tests() {
    testJumpGateActivationCreate();
    testJumpGateActivationInitValidation();
    testJumpGateActivationCharge();
    testJumpGateActivationFullCycle();
    testJumpGateActivationCancelCharge();
    testJumpGateActivationQueue();
    testJumpGateActivationChargeTimeClamp();
    testJumpGateActivationFuelCost();
    testJumpGateActivationCannotChargeDuringCooldown();
    testJumpGateActivationMissing();
}
