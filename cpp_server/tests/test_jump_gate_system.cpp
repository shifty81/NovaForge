// Tests for: JumpGate System Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "pcg/star_system_generator.h"
#include "systems/jump_gate_system.h"

using namespace atlas;

// ==================== JumpGate System Tests ====================

static void testJumpGateCreate() {
    std::cout << "\n=== JumpGate: Create ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    assertTrue(sys.initializeGateNetwork("gate1", "system_jita"), "Init gate network succeeds");
    assertTrue(sys.getGateCount("gate1") == 0, "No gates initially");
    assertTrue(sys.getTotalJumps("gate1") == 0, "No jumps initially");
}

static void testJumpGateAdd() {
    std::cout << "\n=== JumpGate: Add ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    assertTrue(sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f), "Add gate succeeds");
    assertTrue(sys.getGateCount("gate1") == 1, "1 gate");
    assertTrue(sys.getOnlineGateCount("gate1") == 1, "1 online gate");
}

static void testJumpGateDuplicate() {
    std::cout << "\n=== JumpGate: Duplicate ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    assertTrue(!sys.addGate("gate1", "g1", "system_dodixie", "g_dod1", 60.0f, 0.7f), "Duplicate rejected");
    assertTrue(sys.getGateCount("gate1") == 1, "Still 1 gate");
}

static void testJumpGateActivate() {
    std::cout << "\n=== JumpGate: Activate ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    assertTrue(sys.isGateReady("gate1", "g1"), "Gate ready before activation");
    assertTrue(sys.activateGate("gate1", "g1"), "Activation succeeds");
    assertTrue(!sys.isGateReady("gate1", "g1"), "Gate not ready during activation");
    assertTrue(!sys.activateGate("gate1", "g1"), "Double activation rejected");
}

static void testJumpGateJump() {
    std::cout << "\n=== JumpGate: Jump ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    sys.activateGate("gate1", "g1");
    // Advance time to complete activation (default 10s)
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getActivationProgress("gate1", "g1"), 1.0f), "Activation complete");
    assertTrue(sys.completeJump("gate1", "g1"), "Jump completes");
    assertTrue(sys.getTotalJumps("gate1") == 1, "1 total jump");
}

static void testJumpGateCooldown() {
    std::cout << "\n=== JumpGate: Cooldown ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    sys.activateGate("gate1", "g1");
    sys.update(10.0f); // complete activation
    sys.completeJump("gate1", "g1");
    assertTrue(!sys.isGateReady("gate1", "g1"), "Gate on cooldown");
    assertTrue(sys.getCooldownRemaining("gate1", "g1") > 0.0f, "Cooldown > 0");
    assertTrue(!sys.activateGate("gate1", "g1"), "Can't activate on cooldown");
    sys.update(30.0f); // wait out cooldown
    assertTrue(sys.isGateReady("gate1", "g1"), "Gate ready after cooldown");
}

static void testJumpGateOffline() {
    std::cout << "\n=== JumpGate: Offline ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    assertTrue(sys.setGateOnline("gate1", "g1", false), "Set offline succeeds");
    assertTrue(!sys.activateGate("gate1", "g1"), "Can't activate offline gate");
    assertTrue(sys.getOnlineGateCount("gate1") == 0, "0 online gates");
    assertTrue(sys.setGateOnline("gate1", "g1", true), "Set online succeeds");
    assertTrue(sys.getOnlineGateCount("gate1") == 1, "1 online gate");
}

static void testJumpGateRemove() {
    std::cout << "\n=== JumpGate: Remove ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    sys.addGate("gate1", "g2", "system_dodixie", "g_dod1", 60.0f, 0.7f);
    assertTrue(sys.removeGate("gate1", "g1"), "Remove succeeds");
    assertTrue(sys.getGateCount("gate1") == 1, "1 gate remaining");
    assertTrue(!sys.removeGate("gate1", "g1"), "Remove nonexistent fails");
}

static void testJumpGateMaxLimit() {
    std::cout << "\n=== JumpGate: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    auto* entity = world.getEntity("gate1");
    auto* jg = entity->getComponent<components::JumpGate>();
    jg->max_gates = 2;
    sys.addGate("gate1", "g1", "sys_a", "ga", 50.0f, 1.0f);
    sys.addGate("gate1", "g2", "sys_b", "gb", 50.0f, 1.0f);
    assertTrue(!sys.addGate("gate1", "g3", "sys_c", "gc", 50.0f, 1.0f), "Max limit enforced");
    assertTrue(sys.getGateCount("gate1") == 2, "Still 2 gates");
}

static void testJumpGateMissing() {
    std::cout << "\n=== JumpGate: Missing ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    assertTrue(!sys.initializeGateNetwork("nonexistent", "s1"), "Init fails on missing");
    assertTrue(!sys.addGate("nonexistent", "g1", "s", "g", 50.0f, 1.0f), "Add fails on missing");
    assertTrue(!sys.removeGate("nonexistent", "g1"), "Remove fails on missing");
    assertTrue(!sys.activateGate("nonexistent", "g1"), "Activate fails on missing");
    assertTrue(!sys.completeJump("nonexistent", "g1"), "Jump fails on missing");
    assertTrue(sys.getGateCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getOnlineGateCount("nonexistent") == 0, "0 online on missing");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "0 jumps on missing");
    assertTrue(!sys.isGateReady("nonexistent", "g1"), "Not ready on missing");
    assertTrue(approxEqual(sys.getActivationProgress("nonexistent", "g1"), 0.0f), "0 progress on missing");
}


void run_jump_gate_system_tests() {
    testJumpGateCreate();
    testJumpGateAdd();
    testJumpGateDuplicate();
    testJumpGateActivate();
    testJumpGateJump();
    testJumpGateCooldown();
    testJumpGateOffline();
    testJumpGateRemove();
    testJumpGateMaxLimit();
    testJumpGateMissing();
}
