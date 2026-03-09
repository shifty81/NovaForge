// Tests for: Shield Arc HUD System
#include "test_log.h"
#include "components/ui_components.h"
#include "systems/shield_arc_hud_system.h"

using namespace atlas;

// ==================== Shield Arc HUD System Tests ====================

static void testShieldArcHudCreate() {
    std::cout << "\n=== ShieldArcHud: Create ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(approxEqual(sys.getShieldPercent("ship1"), 100.0f), "Shield starts at 100");
    assertTrue(approxEqual(sys.getArmorPercent("ship1"), 100.0f), "Armor starts at 100");
    assertTrue(approxEqual(sys.getHullPercent("ship1"), 100.0f), "Hull starts at 100");
    assertTrue(!sys.isShieldCritical("ship1"), "Shield not critical");
    assertTrue(!sys.isArmorCritical("ship1"), "Armor not critical");
    assertTrue(!sys.isHullCritical("ship1"), "Hull not critical");
}

static void testShieldArcHudSetPercent() {
    std::cout << "\n=== ShieldArcHud: SetPercent ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setShieldPercent("ship1", 75.0f), "Set shield succeeds");
    assertTrue(approxEqual(sys.getShieldPercent("ship1"), 75.0f), "Shield is 75");
    assertTrue(sys.setArmorPercent("ship1", 50.0f), "Set armor succeeds");
    assertTrue(approxEqual(sys.getArmorPercent("ship1"), 50.0f), "Armor is 50");
    assertTrue(sys.setHullPercent("ship1", 30.0f), "Set hull succeeds");
    assertTrue(approxEqual(sys.getHullPercent("ship1"), 30.0f), "Hull is 30");
}

static void testShieldArcHudClamping() {
    std::cout << "\n=== ShieldArcHud: Clamping ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setShieldPercent("ship1", 150.0f);
    assertTrue(approxEqual(sys.getShieldPercent("ship1"), 100.0f), "Clamped above to 100");
    sys.setShieldPercent("ship1", -10.0f);
    assertTrue(approxEqual(sys.getShieldPercent("ship1"), 0.0f), "Clamped below to 0");
}

static void testShieldArcHudCritical() {
    std::cout << "\n=== ShieldArcHud: Critical ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setShieldPercent("ship1", 20.0f);
    sys.setArmorPercent("ship1", 10.0f);
    sys.setHullPercent("ship1", 5.0f);
    sys.update(0.1f);
    assertTrue(sys.isShieldCritical("ship1"), "Shield critical at 20%");
    assertTrue(sys.isArmorCritical("ship1"), "Armor critical at 10%");
    assertTrue(sys.isHullCritical("ship1"), "Hull critical at 5%");
}

static void testShieldArcHudCriticalThreshold() {
    std::cout << "\n=== ShieldArcHud: CriticalThreshold ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setShieldPercent("ship1", 25.0f);
    sys.update(0.1f);
    assertTrue(!sys.isShieldCritical("ship1"), "25% is not critical (threshold is < 25%)");
    sys.setShieldPercent("ship1", 24.9f);
    sys.update(0.1f);
    assertTrue(sys.isShieldCritical("ship1"), "24.9% is critical");
}

static void testShieldArcHudFlashTimer() {
    std::cout << "\n=== ShieldArcHud: FlashTimer ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setShieldPercent("ship1", 10.0f);
    sys.update(0.3f);
    auto* entity = world.getEntity("ship1");
    auto* arc = entity->getComponent<components::ShieldArcHud>();
    assertTrue(arc->flash_timer > 0.0f, "Flash timer ticking when critical");
    sys.setShieldPercent("ship1", 80.0f);
    sys.update(0.1f);
    assertTrue(approxEqual(arc->flash_timer, 0.0f), "Flash timer reset when not critical");
}

static void testShieldArcHudVisibility() {
    std::cout << "\n=== ShieldArcHud: Visibility ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setVisible("ship1", false), "Set invisible succeeds");
    auto* entity = world.getEntity("ship1");
    auto* arc = entity->getComponent<components::ShieldArcHud>();
    assertTrue(!arc->visible, "Arc is invisible");
    assertTrue(sys.setVisible("ship1", true), "Set visible succeeds");
    assertTrue(arc->visible, "Arc is visible again");
}

static void testShieldArcHudMissing() {
    std::cout << "\n=== ShieldArcHud: Missing ===" << std::endl;
    ecs::World world;
    systems::ShieldArcHudSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.setShieldPercent("nonexistent", 50.0f), "Set fails on missing");
    assertTrue(!sys.setArmorPercent("nonexistent", 50.0f), "Set armor fails on missing");
    assertTrue(!sys.setHullPercent("nonexistent", 50.0f), "Set hull fails on missing");
    assertTrue(approxEqual(sys.getShieldPercent("nonexistent"), 0.0f), "0 shield on missing");
    assertTrue(approxEqual(sys.getArmorPercent("nonexistent"), 0.0f), "0 armor on missing");
    assertTrue(approxEqual(sys.getHullPercent("nonexistent"), 0.0f), "0 hull on missing");
    assertTrue(!sys.isShieldCritical("nonexistent"), "Not critical on missing");
    assertTrue(!sys.isArmorCritical("nonexistent"), "Not critical on missing");
    assertTrue(!sys.isHullCritical("nonexistent"), "Not critical on missing");
    assertTrue(!sys.setVisible("nonexistent", true), "Set visible fails on missing");
}

void run_shield_arc_hud_system_tests() {
    testShieldArcHudCreate();
    testShieldArcHudSetPercent();
    testShieldArcHudClamping();
    testShieldArcHudCritical();
    testShieldArcHudCriticalThreshold();
    testShieldArcHudFlashTimer();
    testShieldArcHudVisibility();
    testShieldArcHudMissing();
}
