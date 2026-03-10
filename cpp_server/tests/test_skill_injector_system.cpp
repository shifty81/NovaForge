// Tests for: SkillInjectorSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/skill_injector_system.h"

using namespace atlas;

// ==================== SkillInjectorSystem Tests ====================

static void testInjectorInit() {
    std::cout << "\n=== Injector: Init ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", 1000000, 5000000), "Init succeeds");
    assertTrue(sys.getUnallocatedSP("p1") == 1000000, "Unallocated SP stored");
    assertTrue(sys.getTotalSP("p1") == 5000000, "Total SP stored");
    assertTrue(sys.getInjectorCount("p1") == 0, "Zero injectors initially");
    assertTrue(sys.getTotalExtracted("p1") == 0, "Zero extracted initially");
    assertTrue(sys.getTotalInjected("p1") == 0, "Zero injected initially");
}

static void testInjectorInitFails() {
    std::cout << "\n=== Injector: InitFails ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 0, 0), "Fails on missing entity");
    world.createEntity("p1");
    assertTrue(!sys.initialize("p1", -1, 0), "Fails with negative unallocated");
    assertTrue(!sys.initialize("p1", 0, -1), "Fails with negative total_sp");
}

static void testInjectorExtract() {
    std::cout << "\n=== Injector: Extract ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 2000000, 20000000);

    assertTrue(sys.extractSP("p1", "inj1", "pilot1"), "Extract 500k SP");
    assertTrue(sys.getInjectorCount("p1") == 1, "One injector in inventory");
    assertTrue(sys.getUnallocatedSP("p1") == 1500000, "Unallocated reduced by 500k");
    assertTrue(sys.getTotalExtracted("p1") == 500000, "500k extracted recorded");
    assertTrue(sys.hasInjector("p1", "inj1"), "Injector present");

    // Extract second
    assertTrue(sys.extractSP("p1", "inj2", "pilot1"), "Extract second 500k SP");
    assertTrue(sys.getInjectorCount("p1") == 2, "Two injectors");
    assertTrue(sys.getUnallocatedSP("p1") == 1000000, "Unallocated now 1M");
    assertTrue(sys.getTotalExtracted("p1") == 1000000, "1M extracted total");
}

static void testInjectorExtractFails() {
    std::cout << "\n=== Injector: ExtractFails ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 400000, 5000000);  // < 500k unallocated

    assertTrue(!sys.extractSP("p1", "inj1", "pilot1"), "Insufficient SP fails");
    assertTrue(!sys.extractSP("p1", "", "pilot1"), "Empty injector_id fails");

    // Reset with enough SP but duplicate injector
    world.createEntity("p2");
    sys.initialize("p2", 2000000, 5000000);
    sys.extractSP("p2", "inj1", "pilot1");
    assertTrue(!sys.extractSP("p2", "inj1", "pilot1"), "Duplicate injector_id fails");
}

static void testInjectorInjectFullDose() {
    std::cout << "\n=== Injector: InjectFullDose ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    // total_sp ≤ 5M → full 500k dose
    sys.initialize("p1", 1000000, 3000000);
    sys.extractSP("p1", "inj1", "pilot1");

    assertTrue(sys.computeDose("p1") == 500000, "Full dose at 2.5M SP");

    world.createEntity("p2");
    sys.initialize("p2", 500000, 2000000);
    // Give p2 an injector from p1
    auto* comp1 = world.getEntity("p1")->getComponent<components::SkillInjectorState>();
    auto* comp2 = world.getEntity("p2")->getComponent<components::SkillInjectorState>();
    comp2->injectors.push_back(comp1->injectors.front());
    comp2->total_sp = 2000000;

    assertTrue(sys.injectSP("p2", "inj1"), "Inject full dose");
    assertTrue(sys.getUnallocatedSP("p2") == 1000000, "SP added: 500k + 500k");
    assertTrue(sys.getTotalInjected("p2") == 500000, "500k injected recorded");
    assertTrue(!sys.hasInjector("p2", "inj1"), "Used injector removed");
}

static void testInjectorDiminishingReturns() {
    std::cout << "\n=== Injector: DiminishingReturns ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);

    world.createEntity("p_low");
    sys.initialize("p_low", 0, 4000000);
    assertTrue(sys.computeDose("p_low") == 500000, "≤5M → 500k dose");

    world.createEntity("p_mid1");
    sys.initialize("p_mid1", 0, 10000000);
    assertTrue(sys.computeDose("p_mid1") == 400000, "5–50M → 400k dose");

    world.createEntity("p_mid2");
    sys.initialize("p_mid2", 0, 60000000);
    assertTrue(sys.computeDose("p_mid2") == 300000, "50–80M → 300k dose");

    world.createEntity("p_high");
    sys.initialize("p_high", 0, 100000000);
    assertTrue(sys.computeDose("p_high") == 200000, ">80M → 200k dose");
}

static void testInjectorInjectReducedDose() {
    std::cout << "\n=== Injector: InjectReducedDose ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    // total_sp 50–80M → 300k dose
    sys.initialize("p1", 1000000, 60000000);
    sys.extractSP("p1", "inj1", "p1");

    // Put injector into a fresh entity at high SP tier for injection test
    world.createEntity("p2");
    sys.initialize("p2", 0, 60000000);
    auto* comp2 = world.getEntity("p2")->getComponent<components::SkillInjectorState>();
    auto* comp1 = world.getEntity("p1")->getComponent<components::SkillInjectorState>();
    comp2->injectors.push_back(comp1->injectors.front());

    assertTrue(sys.injectSP("p2", "inj1"), "Inject at 60M SP");
    assertTrue(sys.getUnallocatedSP("p2") == 300000, "300k SP added at 60M tier");
    assertTrue(sys.getTotalSP("p2") == 60300000, "Total SP updated");
}

static void testInjectorCapacity() {
    std::cout << "\n=== Injector: Capacity ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 20000000, 5000000);
    auto* comp = world.getEntity("p1")->getComponent<components::SkillInjectorState>();
    comp->max_injectors = 3;

    sys.extractSP("p1", "i1", "src");
    sys.extractSP("p1", "i2", "src");
    sys.extractSP("p1", "i3", "src");
    assertTrue(sys.getInjectorCount("p1") == 3, "At capacity");
    assertTrue(!sys.extractSP("p1", "i4", "src"), "Fourth extraction rejected");
    assertTrue(sys.getInjectorCount("p1") == 3, "Still at 3");
}

static void testInjectorAddUnallocated() {
    std::cout << "\n=== Injector: AddUnallocated ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 0, 0);

    assertTrue(sys.addUnallocatedSP("p1", 1000000), "Add 1M unallocated");
    assertTrue(sys.getUnallocatedSP("p1") == 1000000, "1M unallocated");
    assertTrue(sys.getTotalSP("p1") == 1000000, "Total SP updated");
    assertTrue(!sys.addUnallocatedSP("p1", 0), "Zero amount rejected");
    assertTrue(!sys.addUnallocatedSP("p1", -1), "Negative amount rejected");
}

static void testInjectorSetTotalSP() {
    std::cout << "\n=== Injector: SetTotalSP ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 0, 0);

    assertTrue(sys.setTotalSP("p1", 50000000), "Set total SP");
    assertTrue(sys.getTotalSP("p1") == 50000000, "Total SP set");
    assertTrue(!sys.setTotalSP("p1", -1), "Negative total_sp rejected");
}

static void testInjectorMissing() {
    std::cout << "\n=== Injector: Missing ===" << std::endl;
    ecs::World world;
    systems::SkillInjectorSystem sys(&world);

    assertTrue(!sys.extractSP("nonexistent", "inj1", "src"), "Extract fails on missing");
    assertTrue(!sys.injectSP("nonexistent", "inj1"), "Inject fails on missing");
    assertTrue(!sys.addUnallocatedSP("nonexistent", 100), "AddSP fails on missing");
    assertTrue(!sys.setTotalSP("nonexistent", 100), "SetTotal fails on missing");
    assertTrue(sys.getInjectorCount("nonexistent") == 0, "Zero injectors on missing");
    assertTrue(sys.getUnallocatedSP("nonexistent") == 0, "Zero unallocated on missing");
    assertTrue(sys.getTotalSP("nonexistent") == 0, "Zero total on missing");
    assertTrue(sys.getTotalExtracted("nonexistent") == 0, "Zero extracted on missing");
    assertTrue(sys.getTotalInjected("nonexistent") == 0, "Zero injected on missing");
    assertTrue(!sys.hasInjector("nonexistent", "inj1"), "HasInjector false on missing");
    assertTrue(sys.computeDose("nonexistent") == 0, "Zero dose on missing");
}

void run_skill_injector_system_tests() {
    testInjectorInit();
    testInjectorInitFails();
    testInjectorExtract();
    testInjectorExtractFails();
    testInjectorInjectFullDose();
    testInjectorDiminishingReturns();
    testInjectorInjectReducedDose();
    testInjectorCapacity();
    testInjectorAddUnallocated();
    testInjectorSetTotalSP();
    testInjectorMissing();
}
