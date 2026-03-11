// Tests for: StasisWebSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/stasis_web_system.h"

using namespace atlas;

// ==================== StasisWebSystem Tests ====================

static void testWebInit() {
    std::cout << "\n=== Web: Init ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 500.0f), "Init succeeds");
    assertTrue(sys.getWebCount("ship1") == 0, "Zero webs initially");
    assertTrue(sys.getBaseVelocity("ship1") == 500.0f, "Base velocity stored");
    assertTrue(sys.getEffectiveVelocity("ship1") == 500.0f, "Effective velocity equals base");
    assertTrue(!sys.isWebbed("ship1"), "Not webbed initially");
    assertTrue(sys.getTotalWebsApplied("ship1") == 0, "Zero webs applied initially");
}

static void testWebInitFails() {
    std::cout << "\n=== Web: InitFails ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 500.0f), "Fails on missing entity");
    world.createEntity("ship1");
    assertTrue(!sys.initialize("ship1", 0.0f), "Fails with zero velocity");
    assertTrue(!sys.initialize("ship1", -1.0f), "Fails with negative velocity");
}

static void testWebApply() {
    std::cout << "\n=== Web: Apply ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    assertTrue(sys.applyWeb("ship1", "web1", "src1", 0.6f, 5.0f), "Apply first web");
    assertTrue(sys.getWebCount("ship1") == 1, "One web");
    assertTrue(sys.isWebbed("ship1"), "Is webbed after apply");
    assertTrue(sys.getTotalWebsApplied("ship1") == 1, "Total applied incremented");

    assertTrue(sys.applyWeb("ship1", "web2", "src2", 0.5f, 5.0f), "Apply second web");
    assertTrue(sys.getWebCount("ship1") == 2, "Two webs");
    assertTrue(sys.getTotalWebsApplied("ship1") == 2, "Total applied = 2");
}

static void testWebApplyValidation() {
    std::cout << "\n=== Web: ApplyValidation ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    assertTrue(!sys.applyWeb("ship1", "", "src1", 0.6f, 5.0f), "Empty web_id rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "", 0.6f, 5.0f), "Empty source_id rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "src1", 0.0f, 5.0f), "Zero strength rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "src1", 1.0f, 5.0f), "Strength=1 rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "src1", 1.1f, 5.0f), "Strength>1 rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "src1", -0.1f, 5.0f), "Negative strength rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "src1", 0.6f, 0.0f), "Zero cycle_time rejected");
    assertTrue(!sys.applyWeb("ship1", "web1", "src1", 0.6f, -1.0f), "Negative cycle_time rejected");

    // Duplicate
    sys.applyWeb("ship1", "web1", "src1", 0.6f, 5.0f);
    assertTrue(!sys.applyWeb("ship1", "web1", "src2", 0.5f, 5.0f), "Duplicate web_id rejected");
}

static void testWebRemove() {
    std::cout << "\n=== Web: Remove ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);
    sys.applyWeb("ship1", "web1", "src1", 0.6f, 5.0f);
    sys.applyWeb("ship1", "web2", "src2", 0.5f, 5.0f);

    assertTrue(sys.removeWeb("ship1", "web1"), "Remove first web");
    assertTrue(sys.getWebCount("ship1") == 1, "One web remaining");
    assertTrue(sys.isWebbed("ship1"), "Still webbed");
    assertTrue(!sys.removeWeb("ship1", "nonexistent"), "Remove unknown fails");

    assertTrue(sys.removeWeb("ship1", "web2"), "Remove last web");
    assertTrue(sys.getWebCount("ship1") == 0, "Zero webs");
    assertTrue(!sys.isWebbed("ship1"), "Not webbed after all removed");
}

static void testWebVelocityCalc() {
    std::cout << "\n=== Web: VelocityCalc ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    // Single 60% web → effective = 1000 * 0.4 = 400
    sys.applyWeb("ship1", "web1", "src1", 0.6f, 5.0f);
    assertTrue(approxEqual(sys.getEffectiveVelocity("ship1"), 400.0f, 1.0f), "60% web: 400 m/s");

    // Second 50% web → effective = 1000 * 0.4 * 0.5 = 200
    sys.applyWeb("ship1", "web2", "src2", 0.5f, 5.0f);
    assertTrue(approxEqual(sys.getEffectiveVelocity("ship1"), 200.0f, 1.0f), "Stacked webs: 200 m/s");

    // Remove first → effective = 1000 * 0.5 = 500
    sys.removeWeb("ship1", "web1");
    assertTrue(approxEqual(sys.getEffectiveVelocity("ship1"), 500.0f, 1.0f), "After remove: 500 m/s");
}

static void testWebSetBaseVelocity() {
    std::cout << "\n=== Web: SetBaseVelocity ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);
    sys.applyWeb("ship1", "web1", "src1", 0.5f, 5.0f);

    assertTrue(sys.setBaseVelocity("ship1", 2000.0f), "Set base velocity");
    assertTrue(approxEqual(sys.getBaseVelocity("ship1"), 2000.0f), "Base updated");
    assertTrue(approxEqual(sys.getEffectiveVelocity("ship1"), 1000.0f, 1.0f), "Effective recomputed");

    assertTrue(!sys.setBaseVelocity("ship1", 0.0f), "Zero velocity rejected");
    assertTrue(!sys.setBaseVelocity("ship1", -100.0f), "Negative velocity rejected");
}

static void testWebClearAll() {
    std::cout << "\n=== Web: ClearAll ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);
    sys.applyWeb("ship1", "web1", "src1", 0.6f, 5.0f);
    sys.applyWeb("ship1", "web2", "src2", 0.5f, 5.0f);
    sys.applyWeb("ship1", "web3", "src3", 0.4f, 5.0f);

    assertTrue(sys.getWebCount("ship1") == 3, "Three webs before clear");
    assertTrue(sys.clearWebs("ship1"), "Clear succeeds");
    assertTrue(sys.getWebCount("ship1") == 0, "Zero webs after clear");
    assertTrue(!sys.isWebbed("ship1"), "Not webbed after clear");
    assertTrue(approxEqual(sys.getEffectiveVelocity("ship1"), 1000.0f), "Velocity restored to base");
}

static void testWebCapacity() {
    std::cout << "\n=== Web: Capacity ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);
    auto* comp = world.getEntity("ship1")->getComponent<components::StasisWebState>();
    comp->max_webs = 2;

    assertTrue(sys.applyWeb("ship1", "w1", "s1", 0.5f, 5.0f), "First web ok");
    assertTrue(sys.applyWeb("ship1", "w2", "s2", 0.5f, 5.0f), "Second web ok");
    assertTrue(!sys.applyWeb("ship1", "w3", "s3", 0.5f, 5.0f), "Third web rejected at cap");
    assertTrue(sys.getWebCount("ship1") == 2, "Still at 2");
}

static void testWebTick() {
    std::cout << "\n=== Web: Tick ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);
    sys.applyWeb("ship1", "web1", "src1", 0.6f, 5.0f);

    sys.update(3.0f);
    auto* comp = world.getEntity("ship1")->getComponent<components::StasisWebState>();
    assertTrue(comp != nullptr, "Component exists");
    assertTrue(approxEqual(comp->webs[0].cycle_elapsed, 3.0f), "Cycle elapsed after 3s");

    sys.update(3.0f);
    assertTrue(approxEqual(comp->webs[0].cycle_elapsed, 1.0f), "Elapsed wraps after full cycle (6-5=1)");
}

static void testWebMissing() {
    std::cout << "\n=== Web: Missing ===" << std::endl;
    ecs::World world;
    systems::StasisWebSystem sys(&world);

    assertTrue(!sys.applyWeb("nonexistent", "w1", "s1", 0.5f, 5.0f), "Apply fails on missing");
    assertTrue(!sys.removeWeb("nonexistent", "w1"), "Remove fails on missing");
    assertTrue(!sys.clearWebs("nonexistent"), "Clear fails on missing");
    assertTrue(!sys.setBaseVelocity("nonexistent", 500.0f), "SetBase fails on missing");
    assertTrue(sys.getEffectiveVelocity("nonexistent") == 0.0f, "Zero velocity on missing");
    assertTrue(sys.getBaseVelocity("nonexistent") == 0.0f, "Zero base on missing");
    assertTrue(sys.getWebCount("nonexistent") == 0, "Zero webs on missing");
    assertTrue(sys.getActiveWebCount("nonexistent") == 0, "Zero active on missing");
    assertTrue(!sys.isWebbed("nonexistent"), "Not webbed on missing");
    assertTrue(sys.getTotalWebsApplied("nonexistent") == 0, "Zero total on missing");
}

void run_stasis_web_system_tests() {
    testWebInit();
    testWebInitFails();
    testWebApply();
    testWebApplyValidation();
    testWebRemove();
    testWebVelocityCalc();
    testWebSetBaseVelocity();
    testWebClearAll();
    testWebCapacity();
    testWebTick();
    testWebMissing();
}
