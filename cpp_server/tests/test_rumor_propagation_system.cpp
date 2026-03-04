// Tests for: RumorPropagation System Tests
#include "test_log.h"
#include "components/narrative_components.h"
#include "ecs/system.h"
#include "systems/rumor_propagation_system.h"

using namespace atlas;

// ===== RumorPropagation System Tests =====

static void testRumorPropCreate() {
    std::cout << "\n=== RumorProp: Create ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    assertTrue(sys.initializeNetwork("network1"), "Init network succeeds");
    assertTrue(sys.getRumorCount("network1") == 0, "No rumors initially");
    assertTrue(sys.getConfirmedCount("network1") == 0, "No confirmed initially");
    assertTrue(sys.getExpiredCount("network1") == 0, "No expired initially");
}

static void testRumorPropCreateRumor() {
    std::cout << "\n=== RumorProp: CreateRumor ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    assertTrue(sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.8f), "Create rumor succeeds");
    assertTrue(sys.getRumorCount("network1") == 1, "Rumor count is 1");
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.8f), "Accuracy is 0.8");
    assertTrue(sys.isRumorActive("network1", "rumor_1"), "Rumor is active");
    assertTrue(!sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.5f), "Duplicate rumor fails");
}

static void testRumorPropSpread() {
    std::cout << "\n=== RumorProp: Spread ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "PirateActivity", 0.9f);
    assertTrue(sys.spreadRumor("network1", "rumor_1", "system_alpha"), "Spread succeeds");
    assertTrue(sys.getSpreadCount("network1", "rumor_1") == 1, "Spread count is 1");
    // Accuracy reduced by spread (0.9 * 0.9 = 0.81)
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.81f), "Accuracy decayed after spread");
    assertTrue(!sys.spreadRumor("network1", "rumor_1", "system_alpha"), "Duplicate spread fails");
}

static void testRumorPropConfirm() {
    std::cout << "\n=== RumorProp: Confirm ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TradeShift", 0.6f);
    assertTrue(sys.confirmRumor("network1", "rumor_1"), "Confirm succeeds");
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 1.0f), "Confirmed accuracy is 1.0");
    assertTrue(sys.getConfirmedCount("network1") == 1, "Confirmed count is 1");
    assertTrue(!sys.confirmRumor("network1", "rumor_1"), "Re-confirm fails");
}

static void testRumorPropDecay() {
    std::cout << "\n=== RumorProp: Decay ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "FactionConflict", 0.5f);
    sys.update(10.0f); // decay: 0.5 - 0.02*10 = 0.3
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.3f), "Accuracy decayed to 0.3");
    assertTrue(sys.isRumorActive("network1", "rumor_1"), "Rumor still active");
}

static void testRumorPropExpiry() {
    std::cout << "\n=== RumorProp: Expiry ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.1f);
    sys.update(5.0f); // decay: 0.1 - 0.02*5 = 0.0, below 0.05 threshold
    assertTrue(!sys.isRumorActive("network1", "rumor_1"), "Rumor expired");
    assertTrue(sys.getExpiredCount("network1") == 1, "Expired count is 1");
}

static void testRumorPropConfirmedNoDecay() {
    std::cout << "\n=== RumorProp: ConfirmedNoDecay ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "PirateActivity", 0.8f);
    sys.confirmRumor("network1", "rumor_1");
    sys.update(100.0f); // confirmed rumors don't decay
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 1.0f), "Confirmed rumor stays at 1.0");
}

static void testRumorPropMultipleRumors() {
    std::cout << "\n=== RumorProp: Multiple ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.9f);
    sys.createRumor("network1", "rumor_2", "PirateActivity", 0.7f);
    sys.createRumor("network1", "rumor_3", "TradeShift", 0.5f);
    assertTrue(sys.getRumorCount("network1") == 3, "3 rumors created");
    sys.confirmRumor("network1", "rumor_2");
    assertTrue(sys.getConfirmedCount("network1") == 1, "1 confirmed");
}

static void testRumorPropMultiSpread() {
    std::cout << "\n=== RumorProp: MultiSpread ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TitanAssembly", 1.0f);
    sys.spreadRumor("network1", "rumor_1", "system_a");
    sys.spreadRumor("network1", "rumor_1", "system_b");
    sys.spreadRumor("network1", "rumor_1", "system_c");
    assertTrue(sys.getSpreadCount("network1", "rumor_1") == 3, "Spread to 3 systems");
    // Accuracy: 1.0 * 0.9 * 0.9 * 0.9 = 0.729
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.729f), "Accuracy decayed through 3 spreads");
}

static void testRumorPropMissing() {
    std::cout << "\n=== RumorProp: Missing ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    assertTrue(!sys.initializeNetwork("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.createRumor("nonexistent", "r1", "Test", 0.5f), "Create fails on missing");
    assertTrue(!sys.spreadRumor("nonexistent", "r1", "sys1"), "Spread fails on missing");
    assertTrue(!sys.confirmRumor("nonexistent", "r1"), "Confirm fails on missing");
    assertTrue(approxEqual(sys.getRumorAccuracy("nonexistent", "r1"), 0.0f), "0 accuracy on missing");
    assertTrue(sys.getRumorCount("nonexistent") == 0, "0 rumors on missing");
    assertTrue(sys.getConfirmedCount("nonexistent") == 0, "0 confirmed on missing");
    assertTrue(sys.getExpiredCount("nonexistent") == 0, "0 expired on missing");
    assertTrue(sys.getSpreadCount("nonexistent", "r1") == 0, "0 spread on missing");
    assertTrue(!sys.isRumorActive("nonexistent", "r1"), "Not active on missing");
}


void run_rumor_propagation_system_tests() {
    testRumorPropCreate();
    testRumorPropCreateRumor();
    testRumorPropSpread();
    testRumorPropConfirm();
    testRumorPropDecay();
    testRumorPropExpiry();
    testRumorPropConfirmedNoDecay();
    testRumorPropMultipleRumors();
    testRumorPropMultiSpread();
    testRumorPropMissing();
}
