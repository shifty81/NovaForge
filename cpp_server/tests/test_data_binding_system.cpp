// Tests for: DataBindingSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/data_binding_system.h"

using namespace atlas;

// ==================== DataBindingSystem Tests ====================

static void testDataBindInitialize() {
    std::cout << "\n=== DataBinding: Initialize ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");

    assertTrue(sys.initializeBindings("ui1", "player1"), "Initialize bindings");
    assertTrue(sys.getBindingCount("ui1") == 0, "No bindings initially");
    assertTrue(sys.getObserverCount("ui1") == 0, "No observers initially");
}

static void testDataBindDuplicateInitRejected() {
    std::cout << "\n=== DataBinding: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");

    assertTrue(sys.initializeBindings("ui1", "player1"), "First init ok");
    assertTrue(!sys.initializeBindings("ui1", "player2"), "Duplicate init rejected");
}

static void testDataBindAddBinding() {
    std::cout << "\n=== DataBinding: AddBinding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    assertTrue(sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent"), "Add binding");
    assertTrue(sys.getBindingCount("ui1") == 1, "Binding count is 1");
}

static void testDataBindAddMultipleBindings() {
    std::cout << "\n=== DataBinding: AddMultipleBindings ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    assertTrue(sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent"), "Add binding 1");
    assertTrue(sys.addBinding("ui1", "b2", "ship.shield", "shield_bar", "percent"), "Add binding 2");
    assertTrue(sys.addBinding("ui1", "b3", "ship.speed", "speed_label", "format_speed"), "Add binding 3");
    assertTrue(sys.getBindingCount("ui1") == 3, "Binding count is 3");
}

static void testDataBindDuplicateBindingRejected() {
    std::cout << "\n=== DataBinding: DuplicateBindingRejected ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    assertTrue(sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent"), "Add binding");
    assertTrue(!sys.addBinding("ui1", "b1", "ship.armor", "armor_bar", "percent"), "Duplicate rejected");
    assertTrue(sys.getBindingCount("ui1") == 1, "Still 1 binding");
}

static void testDataBindRemoveBinding() {
    std::cout << "\n=== DataBinding: RemoveBinding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent");
    sys.addBinding("ui1", "b2", "ship.shield", "shield_bar", "percent");

    assertTrue(sys.removeBinding("ui1", "b1"), "Remove binding b1");
    assertTrue(sys.getBindingCount("ui1") == 1, "1 binding remains");
    assertTrue(!sys.removeBinding("ui1", "b1"), "Remove again fails");
}

static void testDataBindUpdateBinding() {
    std::cout << "\n=== DataBinding: UpdateBinding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");
    sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent");

    assertTrue(sys.updateBinding("ui1", "b1", "85"), "Update binding value");
    assertTrue(sys.getDirtyCount("ui1") == 1, "Binding is dirty after update");
}

static void testDataBindUpdateSameValueNotDirty() {
    std::cout << "\n=== DataBinding: UpdateSameValueNotDirty ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    auto* e = world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");
    sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent");

    sys.updateBinding("ui1", "b1", "85");
    // Clear dirty by accessing component directly
    auto* db = e->getComponent<components::DataBinding>();
    db->bindings[0].dirty = false;

    // Update with same value — should not re-dirty
    sys.updateBinding("ui1", "b1", "85");
    assertTrue(db->bindings[0].dirty == false, "Same value does not re-dirty");
}

static void testDataBindAddObserver() {
    std::cout << "\n=== DataBinding: AddObserver ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    assertTrue(sys.addObserver("ui1", "obs1", "ship.*", "on_ship_change"), "Add observer");
    assertTrue(sys.getObserverCount("ui1") == 1, "Observer count is 1");
}

static void testDataBindDuplicateObserverRejected() {
    std::cout << "\n=== DataBinding: DuplicateObserverRejected ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    assertTrue(sys.addObserver("ui1", "obs1", "ship.*", "on_ship_change"), "Add observer");
    assertTrue(!sys.addObserver("ui1", "obs1", "cargo.*", "on_cargo_change"), "Duplicate rejected");
    assertTrue(sys.getObserverCount("ui1") == 1, "Still 1 observer");
}

static void testDataBindRemoveObserver() {
    std::cout << "\n=== DataBinding: RemoveObserver ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    sys.addObserver("ui1", "obs1", "ship.*", "on_ship_change");
    sys.addObserver("ui1", "obs2", "cargo.*", "on_cargo_change");

    assertTrue(sys.removeObserver("ui1", "obs1"), "Remove observer");
    assertTrue(sys.getObserverCount("ui1") == 1, "1 observer remains");
    assertTrue(!sys.removeObserver("ui1", "obs1"), "Remove again fails");
}

static void testDataBindNotifyObservers() {
    std::cout << "\n=== DataBinding: NotifyObservers ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    auto* e = world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");
    sys.addObserver("ui1", "obs1", "ship.*", "on_ship_change");

    assertTrue(sys.notifyObservers("ui1", "ship.*"), "Notify observers");
    assertTrue(sys.processNotifications("ui1"), "Process notifications");

    auto* db = e->getComponent<components::DataBinding>();
    assertTrue(db->total_notifications == 1, "1 notification processed");
}

static void testDataBindSetDirty() {
    std::cout << "\n=== DataBinding: SetDirty ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");
    sys.addBinding("ui1", "b1", "ship.health", "health_bar", "percent");

    // New bindings start dirty
    assertTrue(sys.getDirtyCount("ui1") == 1, "New binding starts dirty");
    assertTrue(sys.setDirty("ui1", "b1"), "Set dirty explicitly");
    assertTrue(!sys.setDirty("ui1", "nonexistent"), "Set dirty on nonexistent fails");
}

static void testDataBindMaxBindingsEnforced() {
    std::cout << "\n=== DataBinding: MaxBindingsEnforced ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    auto* e = world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    auto* db = e->getComponent<components::DataBinding>();
    db->max_bindings = 2;

    assertTrue(sys.addBinding("ui1", "b1", "a", "w1", "f1"), "Add binding 1");
    assertTrue(sys.addBinding("ui1", "b2", "b", "w2", "f2"), "Add binding 2");
    assertTrue(!sys.addBinding("ui1", "b3", "c", "w3", "f3"), "3rd binding rejected");
    assertTrue(sys.getBindingCount("ui1") == 2, "Still 2 bindings");
}

static void testDataBindProcessEmptyNotifications() {
    std::cout << "\n=== DataBinding: ProcessEmptyNotifications ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");

    assertTrue(!sys.processNotifications("ui1"), "Process empty notifications returns false");
}

static void testDataBindMissingEntity() {
    std::cout << "\n=== DataBinding: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);

    assertTrue(!sys.initializeBindings("ghost", "player1"), "Init fails for missing entity");
    assertTrue(!sys.addBinding("ghost", "b1", "a", "w1", "f1"), "Add binding fails for missing");
    assertTrue(!sys.removeBinding("ghost", "b1"), "Remove binding fails for missing");
    assertTrue(!sys.updateBinding("ghost", "b1", "val"), "Update fails for missing");
    assertTrue(!sys.addObserver("ghost", "o1", "p", "c"), "Add observer fails for missing");
    assertTrue(!sys.removeObserver("ghost", "o1"), "Remove observer fails for missing");
    assertTrue(!sys.notifyObservers("ghost", "p"), "Notify fails for missing");
    assertTrue(!sys.setDirty("ghost", "b1"), "Set dirty fails for missing");
    assertTrue(sys.getDirtyCount("ghost") == 0, "Dirty count 0 for missing");
    assertTrue(sys.getBindingCount("ghost") == 0, "Binding count 0 for missing");
    assertTrue(sys.getObserverCount("ghost") == 0, "Observer count 0 for missing");
    assertTrue(!sys.processNotifications("ghost"), "Process fails for missing");
}

static void testDataBindUpdateComponentProcessesPending() {
    std::cout << "\n=== DataBinding: UpdateComponentProcessesPending ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    auto* e = world.createEntity("ui1");
    sys.initializeBindings("ui1", "player1");
    sys.addObserver("ui1", "obs1", "ship.*", "callback1");

    // Queue a notification
    sys.notifyObservers("ui1", "ship.*");

    // updateComponent should process pending notifications
    sys.update(0.016f);

    auto* db = e->getComponent<components::DataBinding>();
    assertTrue(db->total_notifications == 1, "Update processed pending notification");
    assertTrue(db->pending_notifications.empty(), "Pending cleared after update");
}

void run_data_binding_system_tests() {
    testDataBindInitialize();
    testDataBindDuplicateInitRejected();
    testDataBindAddBinding();
    testDataBindAddMultipleBindings();
    testDataBindDuplicateBindingRejected();
    testDataBindRemoveBinding();
    testDataBindUpdateBinding();
    testDataBindUpdateSameValueNotDirty();
    testDataBindAddObserver();
    testDataBindDuplicateObserverRejected();
    testDataBindRemoveObserver();
    testDataBindNotifyObservers();
    testDataBindSetDirty();
    testDataBindMaxBindingsEnforced();
    testDataBindProcessEmptyNotifications();
    testDataBindMissingEntity();
    testDataBindUpdateComponentProcessesPending();
}
