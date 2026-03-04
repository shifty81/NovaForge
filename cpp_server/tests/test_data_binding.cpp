// Tests for: DataBinding Tests
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/data_binding_system.h"

using namespace atlas;

// ==================== DataBinding Tests ====================

static void testDataBindingInit() {
    std::cout << "\n=== DataBinding: Init ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    assertTrue(sys.initializeBindings("db_1", "player_1"), "Bindings initialized");
    assertTrue(sys.getBindingCount("db_1") == 0, "No bindings initially");
    assertTrue(sys.getObserverCount("db_1") == 0, "No observers initially");
    assertTrue(!sys.initializeBindings("db_1", "player_1"), "Duplicate init fails");
}

static void testDataBindingAdd() {
    std::cout << "\n=== DataBinding: Add Binding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    assertTrue(sys.addBinding("db_1", "b1", "ship.shield", "shield_bar", "percent"), "Binding added");
    assertTrue(sys.getBindingCount("db_1") == 1, "Binding count is 1");
    assertTrue(!sys.addBinding("db_1", "b1", "ship.shield", "shield_bar", "percent"), "Duplicate fails");
}

static void testDataBindingRemove() {
    std::cout << "\n=== DataBinding: Remove Binding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addBinding("db_1", "b1", "ship.shield", "shield_bar", "percent");
    assertTrue(sys.removeBinding("db_1", "b1"), "Binding removed");
    assertTrue(sys.getBindingCount("db_1") == 0, "Binding count is 0");
    assertTrue(!sys.removeBinding("db_1", "b1"), "Double remove fails");
}

static void testDataBindingUpdate() {
    std::cout << "\n=== DataBinding: Update ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addBinding("db_1", "b1", "ship.hull", "hull_bar", "percent");
    assertTrue(sys.updateBinding("db_1", "b1", "85"), "Binding updated");
    assertTrue(sys.getDirtyCount("db_1") == 1, "1 dirty binding");
}

static void testDataBindingObserver() {
    std::cout << "\n=== DataBinding: Observer ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    assertTrue(sys.addObserver("db_1", "obs_1", "ship.*", "cb_shield"), "Observer added");
    assertTrue(sys.getObserverCount("db_1") == 1, "Observer count is 1");
    assertTrue(!sys.addObserver("db_1", "obs_1", "ship.*", "cb_shield"), "Duplicate observer fails");
}

static void testDataBindingRemoveObserver() {
    std::cout << "\n=== DataBinding: Remove Observer ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addObserver("db_1", "obs_1", "ship.*", "cb_shield");
    assertTrue(sys.removeObserver("db_1", "obs_1"), "Observer removed");
    assertTrue(sys.getObserverCount("db_1") == 0, "Observer count is 0");
    assertTrue(!sys.removeObserver("db_1", "obs_1"), "Double remove fails");
}

static void testDataBindingNotify() {
    std::cout << "\n=== DataBinding: Notify ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addObserver("db_1", "obs_1", "ship.shield", "cb_1");
    assertTrue(sys.notifyObservers("db_1", "ship.shield"), "Notification queued");
    assertTrue(sys.processNotifications("db_1"), "Notifications processed");
}

static void testDataBindingDirty() {
    std::cout << "\n=== DataBinding: Dirty Tracking ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addBinding("db_1", "b1", "ship.cap", "cap_bar", "percent");
    assertTrue(sys.getDirtyCount("db_1") == 1, "New binding is dirty");
    assertTrue(sys.setDirty("db_1", "b1"), "Set dirty succeeds");
}

static void testDataBindingMaxBindings() {
    std::cout << "\n=== DataBinding: Max Bindings ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    for (int i = 0; i < 50; i++) {
        sys.addBinding("db_1", "b_" + std::to_string(i), "path", "widget", "func");
    }
    assertTrue(sys.getBindingCount("db_1") == 50, "50 bindings added");
    assertTrue(!sys.addBinding("db_1", "b_overflow", "path", "widget", "func"), "Cannot exceed max bindings");
}

static void testDataBindingMissing() {
    std::cout << "\n=== DataBinding: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    assertTrue(!sys.initializeBindings("nonexistent", "o"), "Init fails on missing");
    assertTrue(sys.getBindingCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getObserverCount("nonexistent") == 0, "Observer count 0 on missing");
    assertTrue(sys.getDirtyCount("nonexistent") == 0, "Dirty 0 on missing");
}


void run_data_binding_tests() {
    testDataBindingInit();
    testDataBindingAdd();
    testDataBindingRemove();
    testDataBindingUpdate();
    testDataBindingObserver();
    testDataBindingRemoveObserver();
    testDataBindingNotify();
    testDataBindingDirty();
    testDataBindingMaxBindings();
    testDataBindingMissing();
}
