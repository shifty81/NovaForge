// Tests for: Warp Charge Sequence System
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_charge_sequence_system.h"

using namespace atlas;

// ==================== Warp Charge Sequence System Tests ====================

static void testWarpChargeCreate() {
    std::cout << "\n=== WarpChargeSequence: Create ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 5.0f, 1.0f), "Init succeeds");
    assertTrue(sys.getChargeProgress("ship1") == 0.0f, "0 charge progress");
    assertTrue(sys.getCooldownRemaining("ship1") == 0.0f, "0 cooldown");
    assertTrue(sys.getDestination("ship1") == "", "No destination");
    assertTrue(sys.getTotalWarpsCompleted("ship1") == 0, "0 warps");
    assertTrue(sys.getTotalDisruptions("ship1") == 0, "0 disruptions");
    assertTrue(!sys.isCharging("ship1"), "Not charging");
    assertTrue(!sys.isOnCooldown("ship1"), "Not on cooldown");

    // Invalid params rejected
    world.createEntity("ship2");
    assertTrue(!sys.initialize("ship2", 0.0f, 1.0f), "Charge time 0 rejected");
    assertTrue(!sys.initialize("ship2", 5.0f, 0.0f), "Mass factor 0 rejected");
}

static void testWarpChargeInitiate() {
    std::cout << "\n=== WarpChargeSequence: Initiate ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5.0f, 1.0f);

    assertTrue(sys.initiateWarp("ship1", "system_b"), "Initiate warp");
    assertTrue(sys.isCharging("ship1"), "Is charging");
    assertTrue(sys.getDestination("ship1") == "system_b", "Destination is system_b");

    // Can't initiate while already charging
    assertTrue(!sys.initiateWarp("ship1", "system_c"), "Can't double-initiate");

    // Empty destination rejected
    world.createEntity("ship2");
    sys.initialize("ship2", 5.0f, 1.0f);
    assertTrue(!sys.initiateWarp("ship2", ""), "Empty destination rejected");
}

static void testWarpChargeProgressNoAlignment() {
    std::cout << "\n=== WarpChargeSequence: ProgressNoAlignment ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5.0f, 1.0f);  // 5s charge
    sys.initiateWarp("ship1", "system_b");

    // Not aligned — no charge progress
    sys.update(3.0f);
    assertTrue(sys.getChargeProgress("ship1") == 0.0f, "0 progress without alignment");
}

static void testWarpChargeProgressWithAlignment() {
    std::cout << "\n=== WarpChargeSequence: ProgressWithAlignment ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5.0f, 1.0f);  // 5s charge
    sys.initiateWarp("ship1", "system_b");
    sys.setAligned("ship1", true);

    // Half charge
    sys.update(2.5f);
    float progress = sys.getChargeProgress("ship1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% at 2.5s");

    // Full charge
    sys.update(2.5f);
    progress = sys.getChargeProgress("ship1");
    assertTrue(progress > 0.99f, "100% at 5.0s");
}

static void testWarpChargeMassFactor() {
    std::cout << "\n=== WarpChargeSequence: MassFactor ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5.0f, 2.0f);  // 5s × 2.0 = 10s charge
    sys.initiateWarp("ship1", "system_b");
    sys.setAligned("ship1", true);

    // At 5s with mass_factor 2.0, should be 50%
    sys.update(5.0f);
    float progress = sys.getChargeProgress("ship1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% with mass_factor 2.0");
}

static void testWarpChargeComplete() {
    std::cout << "\n=== WarpChargeSequence: Complete ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5.0f, 1.0f);
    sys.initiateWarp("ship1", "system_b");
    sys.setAligned("ship1", true);

    // Can't complete before fully charged
    sys.update(2.0f);
    assertTrue(!sys.completeWarp("ship1"), "Can't complete partial charge");

    // Fully charge
    sys.update(3.0f);
    assertTrue(sys.completeWarp("ship1"), "Complete warp");
    assertTrue(!sys.isCharging("ship1"), "Not charging after complete");
    assertTrue(sys.getTotalWarpsCompleted("ship1") == 1, "1 warp completed");
    assertTrue(sys.isOnCooldown("ship1"), "On cooldown");
    assertTrue(sys.getDestination("ship1") == "", "Destination cleared");
}

static void testWarpChargeDisrupt() {
    std::cout << "\n=== WarpChargeSequence: Disrupt ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5.0f, 1.0f);
    sys.initiateWarp("ship1", "system_b");
    sys.setAligned("ship1", true);

    sys.update(2.0f);
    assertTrue(sys.disruptWarp("ship1"), "Disrupt warp");
    assertTrue(!sys.isCharging("ship1"), "Not charging after disruption");
    assertTrue(sys.getChargeProgress("ship1") == 0.0f, "Progress reset");
    assertTrue(sys.getTotalDisruptions("ship1") == 1, "1 disruption");
    assertTrue(sys.getDestination("ship1") == "", "Destination cleared");
    assertTrue(sys.getTotalWarpsCompleted("ship1") == 0, "0 warps completed");

    // Can't disrupt when not charging
    assertTrue(!sys.disruptWarp("ship1"), "Can't disrupt when not charging");
}

static void testWarpChargeCooldown() {
    std::cout << "\n=== WarpChargeSequence: Cooldown ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 2.0f, 1.0f);
    sys.initiateWarp("ship1", "system_b");
    sys.setAligned("ship1", true);

    // Complete warp (charge fully in 2s)
    sys.update(2.0f);
    sys.completeWarp("ship1");
    assertTrue(sys.isOnCooldown("ship1"), "On cooldown after warp");

    // Can't initiate while on cooldown
    assertTrue(!sys.initiateWarp("ship1", "system_c"), "Can't warp on cooldown");

    // Wait out cooldown (default 10s)
    sys.update(10.0f);
    assertTrue(!sys.isOnCooldown("ship1"), "Cooldown expired");

    // Can warp again
    assertTrue(sys.initiateWarp("ship1", "system_c"), "Can warp after cooldown");
}

static void testWarpChargeSetAligned() {
    std::cout << "\n=== WarpChargeSequence: SetAligned ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 4.0f, 1.0f);
    sys.initiateWarp("ship1", "system_b");

    // Align mid-charge
    sys.update(1.0f);
    assertTrue(sys.getChargeProgress("ship1") == 0.0f, "0 without alignment");

    sys.setAligned("ship1", true);
    sys.update(2.0f);
    float progress = sys.getChargeProgress("ship1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% after alignment (2s of 4s)");

    // Unalign stops progress
    sys.setAligned("ship1", false);
    sys.update(1.0f);
    float progress2 = sys.getChargeProgress("ship1");
    assertTrue(progress2 > 0.49f && progress2 < 0.51f, "Progress frozen without alignment");
}

static void testWarpChargeMissing() {
    std::cout << "\n=== WarpChargeSequence: Missing ===" << std::endl;
    ecs::World world;
    systems::WarpChargeSequenceSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 5.0f, 1.0f), "Init fails on missing");
    assertTrue(!sys.initiateWarp("nonexistent", "system_b"), "Initiate fails on missing");
    assertTrue(!sys.disruptWarp("nonexistent"), "Disrupt fails on missing");
    assertTrue(!sys.completeWarp("nonexistent"), "Complete fails on missing");
    assertTrue(!sys.setAligned("nonexistent", true), "SetAligned fails on missing");
    assertTrue(sys.getChargeProgress("nonexistent") == 0.0f, "0 progress on missing");
    assertTrue(sys.getCooldownRemaining("nonexistent") == 0.0f, "0 cooldown on missing");
    assertTrue(sys.getDestination("nonexistent") == "", "Empty dest on missing");
    assertTrue(sys.getTotalWarpsCompleted("nonexistent") == 0, "0 warps on missing");
    assertTrue(sys.getTotalDisruptions("nonexistent") == 0, "0 disruptions on missing");
    assertTrue(!sys.isCharging("nonexistent"), "Not charging on missing");
    assertTrue(!sys.isOnCooldown("nonexistent"), "Not on cooldown on missing");
}

void run_warp_charge_sequence_system_tests() {
    testWarpChargeCreate();
    testWarpChargeInitiate();
    testWarpChargeProgressNoAlignment();
    testWarpChargeProgressWithAlignment();
    testWarpChargeMassFactor();
    testWarpChargeComplete();
    testWarpChargeDisrupt();
    testWarpChargeCooldown();
    testWarpChargeSetAligned();
    testWarpChargeMissing();
}
