// Tests for: InertiaModifierSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/inertia_modifier_system.h"

using namespace atlas;

// ==================== InertiaModifierSystem Tests ====================

static void testInertiaInit() {
    std::cout << "\n=== Inertia: Init ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    assertTrue(sys.initialize("s1", 1.5f, 12.0f), "Init succeeds");
    assertTrue(approxEqual(sys.getBaseInertia("s1"), 1.5f), "Base inertia = 1.5");
    assertTrue(approxEqual(sys.getBaseAlignTime("s1"), 12.0f), "Base align time = 12.0");
    assertTrue(approxEqual(sys.getEffectiveInertia("s1"), 1.5f), "Effective = base initially");
    assertTrue(approxEqual(sys.getEffectiveAlignTime("s1"), 12.0f), "Effective align = base");
    assertTrue(sys.getModuleCount("s1") == 0, "Zero modules initially");
    assertTrue(sys.getTotalModifications("s1") == 0, "Zero modifications initially");
}

static void testInertiaInitFails() {
    std::cout << "\n=== Inertia: InitFails ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 1.0f, 10.0f), "Init fails on missing entity");
    world.createEntity("s1");
    assertTrue(!sys.initialize("s1", 0.0f, 10.0f), "Init fails with zero inertia");
    assertTrue(!sys.initialize("s1", -1.0f, 10.0f), "Init fails with negative inertia");
    assertTrue(!sys.initialize("s1", 1.0f, 0.0f), "Init fails with zero align time");
    assertTrue(!sys.initialize("s1", 1.0f, -5.0f), "Init fails with negative align time");
}

static void testInertiaAddModule() {
    std::cout << "\n=== Inertia: AddModule ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 2.0f, 10.0f);

    assertTrue(sys.addModule("s1", "nano1", "Nanofiber I", 0.15f), "Add first module");
    assertTrue(sys.getModuleCount("s1") == 1, "One module stored");
    assertTrue(sys.getTotalModifications("s1") == 1, "One modification");
    // Effective inertia should be reduced
    assertTrue(sys.getEffectiveInertia("s1") < 2.0f, "Inertia reduced after adding module");

    assertTrue(sys.addModule("s1", "nano2", "Nanofiber II", 0.20f), "Add second module");
    assertTrue(sys.getModuleCount("s1") == 2, "Two modules");

    assertTrue(!sys.addModule("s1", "nano1", "Duplicate", 0.10f), "Duplicate module_id rejected");
    assertTrue(!sys.addModule("s1", "", "Empty ID", 0.10f), "Empty module_id rejected");
    assertTrue(!sys.addModule("s1", "x", "", 0.10f), "Empty name rejected");
    assertTrue(!sys.addModule("s1", "x", "Bad", 0.0f), "Zero reduction rejected");
    assertTrue(!sys.addModule("s1", "x", "Bad", 1.0f), "reduction >= 1.0 rejected");
    assertTrue(!sys.addModule("s1", "x", "Bad", -0.5f), "Negative reduction rejected");
    assertTrue(sys.getModuleCount("s1") == 2, "Count unchanged after rejections");
}

static void testInertiaRemoveModule() {
    std::cout << "\n=== Inertia: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 2.0f, 10.0f);
    sys.addModule("s1", "nano1", "Nanofiber I", 0.15f);
    sys.addModule("s1", "nano2", "Nanofiber II", 0.20f);

    assertTrue(sys.removeModule("s1", "nano1"), "Remove existing module");
    assertTrue(sys.getModuleCount("s1") == 1, "Count decremented");
    assertTrue(!sys.removeModule("s1", "nano1"), "Remove nonexistent fails");
    assertTrue(!sys.removeModule("s1", "nonexistent"), "Remove unknown fails");

    // After removing all modules, effective should return to base
    sys.removeModule("s1", "nano2");
    assertTrue(approxEqual(sys.getEffectiveInertia("s1"), 2.0f), "Back to base after removing all");
    assertTrue(approxEqual(sys.getEffectiveAlignTime("s1"), 10.0f), "Align back to base");
}

static void testInertiaActivateDeactivate() {
    std::cout << "\n=== Inertia: ActivateDeactivate ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 2.0f, 10.0f);
    sys.addModule("s1", "nano1", "Nanofiber I", 0.15f);

    float with_active = sys.getEffectiveInertia("s1");
    assertTrue(sys.isModuleActive("s1", "nano1"), "Module active by default");
    assertTrue(!sys.activateModule("s1", "nano1"), "Cannot double-activate");

    assertTrue(sys.deactivateModule("s1", "nano1"), "Deactivate succeeds");
    assertTrue(!sys.isModuleActive("s1", "nano1"), "Module is inactive");
    assertTrue(!sys.deactivateModule("s1", "nano1"), "Cannot double-deactivate");
    assertTrue(approxEqual(sys.getEffectiveInertia("s1"), 2.0f), "Effective = base when deactivated");

    assertTrue(sys.activateModule("s1", "nano1"), "Reactivate succeeds");
    assertTrue(approxEqual(sys.getEffectiveInertia("s1"), with_active), "Effective matches prior active");
    assertTrue(!sys.activateModule("s1", "nonexistent"), "Activate nonexistent fails");
    assertTrue(!sys.deactivateModule("s1", "nonexistent"), "Deactivate nonexistent fails");
}

static void testInertiaStackingPenalty() {
    std::cout << "\n=== Inertia: StackingPenalty ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 1.0f, 10.0f);

    // Add 4 identical modules with 0.10 reduction each
    sys.addModule("s1", "m1", "Nano I", 0.10f);
    float after_one = sys.getEffectiveInertia("s1");
    sys.addModule("s1", "m2", "Nano II", 0.10f);
    float after_two = sys.getEffectiveInertia("s1");
    sys.addModule("s1", "m3", "Nano III", 0.10f);
    float after_three = sys.getEffectiveInertia("s1");
    sys.addModule("s1", "m4", "Nano IV", 0.10f);
    float after_four = sys.getEffectiveInertia("s1");

    // Each additional module should have less effect due to stacking penalty
    float delta1 = 1.0f - after_one;        // first module's full effect
    float delta2 = after_one - after_two;    // second module's marginal effect
    float delta3 = after_two - after_three;  // third
    float delta4 = after_three - after_four; // fourth

    assertTrue(delta1 > delta2, "Second module less effective than first (stacking)");
    assertTrue(delta2 > delta3, "Third module less effective than second");
    assertTrue(delta3 > delta4, "Fourth module less effective than third");
    assertTrue(after_four > 0.0f, "Effective inertia still positive");
}

static void testInertiaSetBase() {
    std::cout << "\n=== Inertia: SetBase ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 2.0f, 10.0f);
    sys.addModule("s1", "nano1", "Nanofiber I", 0.15f);

    assertTrue(sys.setBaseInertia("s1", 3.0f), "Set base inertia");
    assertTrue(approxEqual(sys.getBaseInertia("s1"), 3.0f), "Base updated");
    assertTrue(sys.getEffectiveInertia("s1") < 3.0f, "Effective still reduced");
    assertTrue(!sys.setBaseInertia("s1", 0.0f), "Zero base rejected");
    assertTrue(!sys.setBaseInertia("s1", -1.0f), "Negative base rejected");

    assertTrue(sys.setBaseAlignTime("s1", 15.0f), "Set base align time");
    assertTrue(approxEqual(sys.getBaseAlignTime("s1"), 15.0f), "Align time updated");
    assertTrue(!sys.setBaseAlignTime("s1", 0.0f), "Zero align rejected");
    assertTrue(!sys.setBaseAlignTime("s1", -1.0f), "Negative align rejected");
}

static void testInertiaMaxModules() {
    std::cout << "\n=== Inertia: MaxModules ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 1.0f, 10.0f);

    // Set small max for testing
    auto* comp = world.getEntity("s1")->getComponent<components::InertiaModifierState>();
    comp->max_modules = 3;

    sys.addModule("s1", "m1", "Mod1", 0.10f);
    sys.addModule("s1", "m2", "Mod2", 0.10f);
    sys.addModule("s1", "m3", "Mod3", 0.10f);
    assertTrue(sys.getModuleCount("s1") == 3, "Three modules at capacity");
    assertTrue(!sys.addModule("s1", "m4", "Mod4", 0.10f),
               "Fourth module rejected at capacity");
    assertTrue(sys.getModuleCount("s1") == 3, "Count still 3");
}

static void testInertiaAlignTimeReduction() {
    std::cout << "\n=== Inertia: AlignTimeReduction ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);
    world.createEntity("s1");
    sys.initialize("s1", 1.0f, 10.0f);

    sys.addModule("s1", "nano1", "Nanofiber I", 0.15f);
    float eff_align = sys.getEffectiveAlignTime("s1");
    float eff_inertia = sys.getEffectiveInertia("s1");

    // Both should be reduced proportionally
    assertTrue(eff_align < 10.0f, "Align time reduced");
    assertTrue(eff_inertia < 1.0f, "Inertia reduced");
    // Ratio should be same
    assertTrue(approxEqual(eff_align / 10.0f, eff_inertia / 1.0f),
               "Align time and inertia reduced proportionally");
}

static void testInertiaMissing() {
    std::cout << "\n=== Inertia: Missing ===" << std::endl;
    ecs::World world;
    systems::InertiaModifierSystem sys(&world);

    assertTrue(!sys.addModule("nonexistent", "m", "M", 0.10f), "AddModule fails on missing");
    assertTrue(!sys.removeModule("nonexistent", "m"), "RemoveModule fails on missing");
    assertTrue(!sys.activateModule("nonexistent", "m"), "Activate fails on missing");
    assertTrue(!sys.deactivateModule("nonexistent", "m"), "Deactivate fails on missing");
    assertTrue(!sys.setBaseInertia("nonexistent", 1.0f), "SetBase fails on missing");
    assertTrue(!sys.setBaseAlignTime("nonexistent", 1.0f), "SetAlign fails on missing");
    assertTrue(approxEqual(sys.getEffectiveInertia("nonexistent"), 0.0f), "Zero eff inertia on missing");
    assertTrue(approxEqual(sys.getEffectiveAlignTime("nonexistent"), 0.0f), "Zero eff align on missing");
    assertTrue(approxEqual(sys.getBaseInertia("nonexistent"), 0.0f), "Zero base inertia on missing");
    assertTrue(approxEqual(sys.getBaseAlignTime("nonexistent"), 0.0f), "Zero base align on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "Zero modules on missing");
    assertTrue(!sys.isModuleActive("nonexistent", "m"), "Not active on missing");
    assertTrue(sys.getTotalModifications("nonexistent") == 0, "Zero mods on missing");
}

void run_inertia_modifier_system_tests() {
    testInertiaInit();
    testInertiaInitFails();
    testInertiaAddModule();
    testInertiaRemoveModule();
    testInertiaActivateDeactivate();
    testInertiaStackingPenalty();
    testInertiaSetBase();
    testInertiaMaxModules();
    testInertiaAlignTimeReduction();
    testInertiaMissing();
}
