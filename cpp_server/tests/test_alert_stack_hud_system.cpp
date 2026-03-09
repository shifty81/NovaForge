// Tests for: Alert Stack HUD System
#include "test_log.h"
#include "components/ui_components.h"
#include "systems/alert_stack_hud_system.h"

using namespace atlas;

// ==================== Alert Stack HUD System Tests ====================

static void testAlertStackHudCreate() {
    std::cout << "\n=== AlertStackHud: Create ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getAlertCount("ship1") == 0, "No alerts initially");
    assertTrue(sys.getTotalShown("ship1") == 0, "0 total shown");
    assertTrue(sys.getTotalExpired("ship1") == 0, "0 total expired");
    assertTrue(sys.getTotalDismissed("ship1") == 0, "0 total dismissed");
}

static void testAlertStackHudPush() {
    std::cout << "\n=== AlertStackHud: Push ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    int id1 = sys.pushAlert("ship1", 0, "Shield low", 5.0f);
    int id2 = sys.pushAlert("ship1", 1, "Cargo full", 3.0f);
    assertTrue(id1 > 0, "First alert id positive");
    assertTrue(id2 > id1, "Second alert id increments");
    assertTrue(sys.getAlertCount("ship1") == 2, "2 alerts pushed");
    assertTrue(sys.getTotalShown("ship1") == 2, "2 total shown");
}

static void testAlertStackHudExpire() {
    std::cout << "\n=== AlertStackHud: Expire ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.pushAlert("ship1", 0, "Shield low", 2.0f);
    assertTrue(sys.getAlertCount("ship1") == 1, "1 alert before expire");
    sys.update(2.1f);
    assertTrue(sys.getAlertCount("ship1") == 0, "0 alerts after expire");
    assertTrue(sys.getTotalExpired("ship1") == 1, "1 total expired");
}

static void testAlertStackHudPersistent() {
    std::cout << "\n=== AlertStackHud: Persistent ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.pushAlert("ship1", 2, "Under attack!", 5.0f, true);
    sys.update(10.0f);
    assertTrue(sys.getAlertCount("ship1") == 1, "Persistent alert survives expire");
    assertTrue(sys.getTotalExpired("ship1") == 0, "Persistent doesn't count as expired");
}

static void testAlertStackHudDismiss() {
    std::cout << "\n=== AlertStackHud: Dismiss ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    int id = sys.pushAlert("ship1", 1, "Warning", 5.0f, true);
    assertTrue(sys.dismissAlert("ship1", id), "Dismiss succeeds");
    assertTrue(sys.getTotalDismissed("ship1") == 1, "1 total dismissed");
    sys.update(0.1f); // clean up dismissed
    assertTrue(sys.getAlertCount("ship1") == 0, "0 after dismiss + tick");
    assertTrue(!sys.dismissAlert("ship1", id), "Double dismiss fails");
}

static void testAlertStackHudMaxAlerts() {
    std::cout << "\n=== AlertStackHud: MaxAlerts ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    auto* entity = world.getEntity("ship1");
    auto* stack = entity->getComponent<components::AlertStackHud>();
    stack->max_alerts = 3;

    sys.pushAlert("ship1", 0, "A", 5.0f);
    sys.pushAlert("ship1", 0, "B", 5.0f);
    sys.pushAlert("ship1", 0, "C", 5.0f);
    int overflow = sys.pushAlert("ship1", 0, "D", 5.0f);
    assertTrue(overflow == -1, "Overflow returns -1");
    assertTrue(sys.getAlertCount("ship1") == 3, "Still 3 alerts");
}

static void testAlertStackHudClearAll() {
    std::cout << "\n=== AlertStackHud: ClearAll ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.pushAlert("ship1", 0, "A", 5.0f);
    sys.pushAlert("ship1", 1, "B", 5.0f);
    assertTrue(sys.clearAll("ship1"), "Clear all succeeds");
    assertTrue(sys.getAlertCount("ship1") == 0, "0 alerts after clear");
}

static void testAlertStackHudVisibility() {
    std::cout << "\n=== AlertStackHud: Visibility ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setVisible("ship1", false), "Set invisible succeeds");
    auto* entity = world.getEntity("ship1");
    auto* stack = entity->getComponent<components::AlertStackHud>();
    assertTrue(!stack->visible, "Stack is invisible");
}

static void testAlertStackHudMixedExpire() {
    std::cout << "\n=== AlertStackHud: MixedExpire ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.pushAlert("ship1", 0, "Short", 1.0f);
    sys.pushAlert("ship1", 1, "Long", 10.0f);
    sys.pushAlert("ship1", 2, "Persist", 1.0f, true);
    sys.update(1.5f);
    assertTrue(sys.getAlertCount("ship1") == 2, "Short expired, long + persistent remain");
    assertTrue(sys.getTotalExpired("ship1") == 1, "1 expired");
}

static void testAlertStackHudMissing() {
    std::cout << "\n=== AlertStackHud: Missing ===" << std::endl;
    ecs::World world;
    systems::AlertStackHudSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(sys.pushAlert("nonexistent", 0, "Msg") == -1, "Push fails on missing");
    assertTrue(!sys.dismissAlert("nonexistent", 1), "Dismiss fails on missing");
    assertTrue(sys.getAlertCount("nonexistent") == 0, "0 alerts on missing");
    assertTrue(sys.getTotalShown("nonexistent") == 0, "0 shown on missing");
    assertTrue(sys.getTotalExpired("nonexistent") == 0, "0 expired on missing");
    assertTrue(sys.getTotalDismissed("nonexistent") == 0, "0 dismissed on missing");
    assertTrue(!sys.clearAll("nonexistent"), "Clear fails on missing");
    assertTrue(!sys.setVisible("nonexistent", true), "Visible fails on missing");
}

void run_alert_stack_hud_system_tests() {
    testAlertStackHudCreate();
    testAlertStackHudPush();
    testAlertStackHudExpire();
    testAlertStackHudPersistent();
    testAlertStackHudDismiss();
    testAlertStackHudMaxAlerts();
    testAlertStackHudClearAll();
    testAlertStackHudVisibility();
    testAlertStackHudMixedExpire();
    testAlertStackHudMissing();
}
