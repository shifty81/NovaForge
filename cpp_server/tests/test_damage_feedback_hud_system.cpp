// Tests for: Damage Feedback HUD System
#include "test_log.h"
#include "components/ui_components.h"
#include "systems/damage_feedback_hud_system.h"

using namespace atlas;

// ==================== Damage Feedback HUD System Tests ====================

static void testDamageFeedbackHudCreate() {
    std::cout << "\n=== DamageFeedbackHud: Create ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 0.0f), "No shield feedback");
    assertTrue(approxEqual(sys.getArmorIntensity("ship1"), 0.0f), "No armor feedback");
    assertTrue(approxEqual(sys.getHullIntensity("ship1"), 0.0f), "No hull feedback");
    assertTrue(approxEqual(sys.getScreenShake("ship1"), 0.0f), "No screen shake");
    assertTrue(sys.getTotalShieldHits("ship1") == 0, "0 shield hits");
    assertTrue(sys.getTotalArmorHits("ship1") == 0, "0 armor hits");
    assertTrue(sys.getTotalHullHits("ship1") == 0, "0 hull hits");
}

static void testDamageFeedbackHudShieldHit() {
    std::cout << "\n=== DamageFeedbackHud: ShieldHit ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.triggerShieldHit("ship1", 0.8f), "Shield hit succeeds");
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 0.8f), "Shield intensity 0.8");
    assertTrue(sys.getTotalShieldHits("ship1") == 1, "1 shield hit");
}

static void testDamageFeedbackHudArmorHit() {
    std::cout << "\n=== DamageFeedbackHud: ArmorHit ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.triggerArmorHit("ship1", 0.6f), "Armor hit succeeds");
    assertTrue(approxEqual(sys.getArmorIntensity("ship1"), 0.6f), "Armor intensity 0.6");
    assertTrue(sys.getTotalArmorHits("ship1") == 1, "1 armor hit");
}

static void testDamageFeedbackHudHullHit() {
    std::cout << "\n=== DamageFeedbackHud: HullHit ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.triggerHullHit("ship1", 0.9f), "Hull hit succeeds");
    assertTrue(approxEqual(sys.getHullIntensity("ship1"), 0.9f), "Hull intensity 0.9");
    assertTrue(approxEqual(sys.getScreenShake("ship1"), 0.9f), "Screen shake from hull hit");
    assertTrue(sys.getTotalHullHits("ship1") == 1, "1 hull hit");
}

static void testDamageFeedbackHudDecay() {
    std::cout << "\n=== DamageFeedbackHud: Decay ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.triggerShieldHit("ship1", 1.0f);
    sys.update(0.25f); // decay_rate=2.0 → 1.0 - 2.0*0.25 = 0.5
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 0.5f), "Shield decayed to 0.5");
    sys.update(0.25f); // 0.5 - 2.0*0.25 = 0.0
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 0.0f), "Shield fully decayed");
}

static void testDamageFeedbackHudScreenShakeDecay() {
    std::cout << "\n=== DamageFeedbackHud: ScreenShakeDecay ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.triggerHullHit("ship1", 0.9f);
    sys.update(0.1f); // shake_decay=3.0 → 0.9 - 3.0*0.1 = 0.6
    assertTrue(approxEqual(sys.getScreenShake("ship1"), 0.6f), "Screen shake decayed");
}

static void testDamageFeedbackHudIntensityCap() {
    std::cout << "\n=== DamageFeedbackHud: IntensityCap ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.triggerShieldHit("ship1", 1.5f);
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 1.0f), "Intensity capped at 1.0");
}

static void testDamageFeedbackHudMultipleHits() {
    std::cout << "\n=== DamageFeedbackHud: MultipleHits ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.triggerShieldHit("ship1", 0.5f);
    sys.triggerShieldHit("ship1", 0.8f);
    // Takes the max of existing and new
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 0.8f), "Max of hits used");
    assertTrue(sys.getTotalShieldHits("ship1") == 2, "2 shield hits total");
}

static void testDamageFeedbackHudVisibility() {
    std::cout << "\n=== DamageFeedbackHud: Visibility ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setVisible("ship1", false), "Set invisible succeeds");
    auto* entity = world.getEntity("ship1");
    auto* fb = entity->getComponent<components::DamageFeedbackHud>();
    assertTrue(!fb->visible, "Feedback is invisible");
}

static void testDamageFeedbackHudAllLayers() {
    std::cout << "\n=== DamageFeedbackHud: AllLayers ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.triggerShieldHit("ship1", 0.5f);
    sys.triggerArmorHit("ship1", 0.6f);
    sys.triggerHullHit("ship1", 0.7f);
    assertTrue(approxEqual(sys.getShieldIntensity("ship1"), 0.5f), "Shield layer active");
    assertTrue(approxEqual(sys.getArmorIntensity("ship1"), 0.6f), "Armor layer active");
    assertTrue(approxEqual(sys.getHullIntensity("ship1"), 0.7f), "Hull layer active");
    assertTrue(sys.getTotalShieldHits("ship1") == 1, "1 shield hit");
    assertTrue(sys.getTotalArmorHits("ship1") == 1, "1 armor hit");
    assertTrue(sys.getTotalHullHits("ship1") == 1, "1 hull hit");
}

static void testDamageFeedbackHudMissing() {
    std::cout << "\n=== DamageFeedbackHud: Missing ===" << std::endl;
    ecs::World world;
    systems::DamageFeedbackHudSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.triggerShieldHit("nonexistent", 0.5f), "Shield hit fails on missing");
    assertTrue(!sys.triggerArmorHit("nonexistent", 0.5f), "Armor hit fails on missing");
    assertTrue(!sys.triggerHullHit("nonexistent", 0.5f), "Hull hit fails on missing");
    assertTrue(approxEqual(sys.getShieldIntensity("nonexistent"), 0.0f), "0 on missing");
    assertTrue(approxEqual(sys.getArmorIntensity("nonexistent"), 0.0f), "0 on missing");
    assertTrue(approxEqual(sys.getHullIntensity("nonexistent"), 0.0f), "0 on missing");
    assertTrue(approxEqual(sys.getScreenShake("nonexistent"), 0.0f), "0 on missing");
    assertTrue(sys.getTotalShieldHits("nonexistent") == 0, "0 hits on missing");
    assertTrue(sys.getTotalArmorHits("nonexistent") == 0, "0 hits on missing");
    assertTrue(sys.getTotalHullHits("nonexistent") == 0, "0 hits on missing");
    assertTrue(!sys.setVisible("nonexistent", true), "Visible fails on missing");
}

void run_damage_feedback_hud_system_tests() {
    testDamageFeedbackHudCreate();
    testDamageFeedbackHudShieldHit();
    testDamageFeedbackHudArmorHit();
    testDamageFeedbackHudHullHit();
    testDamageFeedbackHudDecay();
    testDamageFeedbackHudScreenShakeDecay();
    testDamageFeedbackHudIntensityCap();
    testDamageFeedbackHudMultipleHits();
    testDamageFeedbackHudVisibility();
    testDamageFeedbackHudAllLayers();
    testDamageFeedbackHudMissing();
}
