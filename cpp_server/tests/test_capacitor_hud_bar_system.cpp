// Tests for: Capacitor HUD Bar System
#include "test_log.h"
#include "components/ui_components.h"
#include "systems/capacitor_hud_bar_system.h"

using namespace atlas;

// ==================== Capacitor HUD Bar System Tests ====================

static void testCapacitorHudBarCreate() {
    std::cout << "\n=== CapacitorHudBar: Create ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getPercent("ship1"), 100.0f), "Starts at 100%");
    assertTrue(sys.getColorState("ship1") == 0, "Starts green");
    assertTrue(!sys.isWarningActive("ship1"), "No warning at 100%");
}

static void testCapacitorHudBarSetCapacitor() {
    std::cout << "\n=== CapacitorHudBar: SetCapacitor ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setCapacitor("ship1", 75.0f, 100.0f), "Set cap succeeds");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getPercent("ship1"), 75.0f), "75% after set");
    assertTrue(sys.getColorState("ship1") == 0, "Still green at 75%");
}

static void testCapacitorHudBarColorYellow() {
    std::cout << "\n=== CapacitorHudBar: ColorYellow ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setCapacitor("ship1", 40.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getColorState("ship1") == 1, "Yellow at 40%");
    assertTrue(!sys.isWarningActive("ship1"), "No warning at 40%");
}

static void testCapacitorHudBarColorRed() {
    std::cout << "\n=== CapacitorHudBar: ColorRed ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setCapacitor("ship1", 20.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getColorState("ship1") == 2, "Red at 20%");
    assertTrue(sys.isWarningActive("ship1"), "Warning active at 20%");
}

static void testCapacitorHudBarEmpty() {
    std::cout << "\n=== CapacitorHudBar: Empty ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setCapacitor("ship1", 0.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getPercent("ship1"), 0.0f), "0% when empty");
    assertTrue(sys.getColorState("ship1") == 2, "Red when empty");
    assertTrue(sys.isWarningActive("ship1"), "Warning when empty");
}

static void testCapacitorHudBarDrainRate() {
    std::cout << "\n=== CapacitorHudBar: DrainRate ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setDrainRate("ship1", 15.5f), "Set drain rate succeeds");
    assertTrue(approxEqual(sys.getDrainRate("ship1"), 15.5f), "Drain rate is 15.5");
}

static void testCapacitorHudBarClamp() {
    std::cout << "\n=== CapacitorHudBar: Clamp ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setCapacitor("ship1", 200.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getPercent("ship1"), 100.0f), "Clamped to max");
}

static void testCapacitorHudBarVisibility() {
    std::cout << "\n=== CapacitorHudBar: Visibility ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setVisible("ship1", false), "Set invisible succeeds");
    auto* entity = world.getEntity("ship1");
    auto* bar = entity->getComponent<components::CapacitorHudBar>();
    assertTrue(!bar->visible, "Bar is invisible");
}

static void testCapacitorHudBarMissing() {
    std::cout << "\n=== CapacitorHudBar: Missing ===" << std::endl;
    ecs::World world;
    systems::CapacitorHudBarSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.setCapacitor("nonexistent", 50.0f, 100.0f), "Set fails on missing");
    assertTrue(!sys.setDrainRate("nonexistent", 1.0f), "Drain rate fails on missing");
    assertTrue(approxEqual(sys.getPercent("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getColorState("nonexistent") == 0, "0 color on missing");
    assertTrue(!sys.isWarningActive("nonexistent"), "No warning on missing");
    assertTrue(approxEqual(sys.getDrainRate("nonexistent"), 0.0f), "0 drain on missing");
    assertTrue(!sys.setVisible("nonexistent", true), "Set visible fails on missing");
}

void run_capacitor_hud_bar_system_tests() {
    testCapacitorHudBarCreate();
    testCapacitorHudBarSetCapacitor();
    testCapacitorHudBarColorYellow();
    testCapacitorHudBarColorRed();
    testCapacitorHudBarEmpty();
    testCapacitorHudBarDrainRate();
    testCapacitorHudBarClamp();
    testCapacitorHudBarVisibility();
    testCapacitorHudBarMissing();
}
