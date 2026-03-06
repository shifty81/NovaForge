// Tests for: Event Notification Feed System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/event_notification_feed_system.h"

using namespace atlas;

// ==================== Event Notification Feed System Tests ====================

static void testNotifFeedCreate() {
    std::cout << "\n=== NotifFeed: Create ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getNotificationCount("p1") == 0, "No notifications");
    assertTrue(sys.getUnreadCount("p1") == 0, "No unread");
    assertTrue(sys.getTotalPushed("p1") == 0, "0 pushed");
    assertTrue(sys.getTotalExpired("p1") == 0, "0 expired");
    assertTrue(sys.getLatestMessage("p1").empty(), "No latest");
}

static void testNotifFeedPushAndQuery() {
    std::cout << "\n=== NotifFeed: PushAndQuery ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.pushNotification("p1", "e1", "Combat", "Shield hit!", 3, 10.0f), "Push e1");
    assertTrue(sys.pushNotification("p1", "e2", "Trade", "Ore sold for 5000 ISK", 2, 15.0f), "Push e2");
    assertTrue(sys.getNotificationCount("p1") == 2, "2 notifications");
    assertTrue(sys.hasNotification("p1", "e1"), "Has e1");
    assertTrue(sys.hasNotification("p1", "e2"), "Has e2");
    assertTrue(!sys.hasNotification("p1", "e3"), "No e3");
    assertTrue(!sys.pushNotification("p1", "e1", "Combat", "Dup", 3, 10.0f), "Dup rejected");
    assertTrue(sys.getTotalPushed("p1") == 2, "2 total pushed");
}

static void testNotifFeedMax() {
    std::cout << "\n=== NotifFeed: Max ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::EventNotificationFeed>();
    state->max_notifications = 2;
    sys.pushNotification("p1", "e1", "Combat", "Hit 1", 1, 10.0f);
    sys.pushNotification("p1", "e2", "Combat", "Hit 2", 2, 10.0f);
    // Push higher priority evicts lowest
    assertTrue(sys.pushNotification("p1", "e3", "Combat", "Hit 3", 3, 10.0f), "Higher priority evicts");
    assertTrue(sys.getNotificationCount("p1") == 2, "Still 2");
    assertTrue(!sys.hasNotification("p1", "e1"), "e1 evicted (lowest prio)");
    assertTrue(sys.hasNotification("p1", "e2"), "e2 remains");
    assertTrue(sys.hasNotification("p1", "e3"), "e3 added");
    // Can't push same or lower priority
    assertTrue(!sys.pushNotification("p1", "e4", "Combat", "Hit 4", 1, 10.0f), "Low priority rejected at cap");
}

static void testNotifFeedReadAndDismiss() {
    std::cout << "\n=== NotifFeed: ReadAndDismiss ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.pushNotification("p1", "e1", "Combat", "Shield hit!", 3, 10.0f);
    sys.pushNotification("p1", "e2", "Trade", "Ore sold", 2, 15.0f);
    assertTrue(sys.getUnreadCount("p1") == 2, "2 unread");
    assertTrue(sys.markRead("p1", "e1"), "Mark e1 read");
    assertTrue(sys.getUnreadCount("p1") == 1, "1 unread");
    assertTrue(!sys.markRead("p1", "e1"), "Double read fails");
    assertTrue(sys.dismissNotification("p1", "e2"), "Dismiss e2");
    assertTrue(sys.getNotificationCount("p1") == 1, "1 left");
    assertTrue(!sys.dismissNotification("p1", "e2"), "Double dismiss fails");
    assertTrue(!sys.markRead("p1", "nonexistent"), "Read nonexistent fails");
}

static void testNotifFeedCategories() {
    std::cout << "\n=== NotifFeed: Categories ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.pushNotification("p1", "e1", "Combat", "Hit!", 3, 10.0f);
    sys.pushNotification("p1", "e2", "Combat", "Miss!", 2, 10.0f);
    sys.pushNotification("p1", "e3", "Trade", "Sold ore", 1, 10.0f);
    assertTrue(sys.getCountByCategory("p1", "Combat") == 2, "2 Combat");
    assertTrue(sys.getCountByCategory("p1", "Trade") == 1, "1 Trade");
    assertTrue(sys.getCountByCategory("p1", "Mining") == 0, "0 Mining");
    assertTrue(sys.getLatestMessageInCategory("p1", "Combat") == "Miss!", "Latest Combat");
    assertTrue(sys.getLatestMessageInCategory("p1", "Trade") == "Sold ore", "Latest Trade");
    assertTrue(sys.getLatestMessageInCategory("p1", "Mining").empty(), "No Mining msg");
}

static void testNotifFeedLatest() {
    std::cout << "\n=== NotifFeed: Latest ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.pushNotification("p1", "e1", "Combat", "First", 1, 10.0f);
    assertTrue(sys.getLatestMessage("p1") == "First", "Latest is First");
    sys.pushNotification("p1", "e2", "Trade", "Second", 1, 10.0f);
    assertTrue(sys.getLatestMessage("p1") == "Second", "Latest is Second");
}

static void testNotifFeedExpiry() {
    std::cout << "\n=== NotifFeed: Expiry ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.pushNotification("p1", "e1", "Combat", "Short-lived", 3, 5.0f);
    sys.pushNotification("p1", "e2", "Trade", "Long-lived", 2, 20.0f);
    assertTrue(sys.getNotificationCount("p1") == 2, "2 before expiry");
    sys.update(6.0f);
    assertTrue(sys.getNotificationCount("p1") == 1, "1 after expiry");
    assertTrue(!sys.hasNotification("p1", "e1"), "e1 expired");
    assertTrue(sys.hasNotification("p1", "e2"), "e2 still alive");
    assertTrue(sys.getTotalExpired("p1") == 1, "1 expired total");
}

static void testNotifFeedClearAll() {
    std::cout << "\n=== NotifFeed: ClearAll ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.pushNotification("p1", "e1", "Combat", "A", 1, 10.0f);
    sys.pushNotification("p1", "e2", "Trade", "B", 2, 10.0f);
    assertTrue(sys.clearAll("p1"), "Clear all");
    assertTrue(sys.getNotificationCount("p1") == 0, "0 after clear");
}

static void testNotifFeedClearCategory() {
    std::cout << "\n=== NotifFeed: ClearCategory ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.pushNotification("p1", "e1", "Combat", "A", 1, 10.0f);
    sys.pushNotification("p1", "e2", "Combat", "B", 2, 10.0f);
    sys.pushNotification("p1", "e3", "Trade", "C", 3, 10.0f);
    assertTrue(sys.clearCategory("p1", "Combat"), "Clear Combat");
    assertTrue(sys.getNotificationCount("p1") == 1, "1 left");
    assertTrue(sys.hasNotification("p1", "e3"), "Trade remains");
    assertTrue(!sys.clearCategory("p1", "Combat"), "No more Combat to clear");
}

static void testNotifFeedUpdate() {
    std::cout << "\n=== NotifFeed: Update ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::EventNotificationFeed>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testNotifFeedMissing() {
    std::cout << "\n=== NotifFeed: Missing ===" << std::endl;
    ecs::World world;
    systems::EventNotificationFeedSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.pushNotification("nonexistent", "e", "C", "m", 1, 1.0f), "push fails");
    assertTrue(sys.getNotificationCount("nonexistent") == 0, "0 count");
    assertTrue(!sys.hasNotification("nonexistent", "e"), "has false");
    assertTrue(!sys.markRead("nonexistent", "e"), "markRead fails");
    assertTrue(!sys.dismissNotification("nonexistent", "e"), "dismiss fails");
    assertTrue(sys.getUnreadCount("nonexistent") == 0, "0 unread");
    assertTrue(sys.getCountByCategory("nonexistent", "C") == 0, "0 by cat");
    assertTrue(sys.getLatestMessage("nonexistent").empty(), "No latest");
    assertTrue(sys.getLatestMessageInCategory("nonexistent", "C").empty(), "No cat latest");
    assertTrue(!sys.clearAll("nonexistent"), "clearAll fails");
    assertTrue(!sys.clearCategory("nonexistent", "C"), "clearCat fails");
    assertTrue(sys.getTotalPushed("nonexistent") == 0, "0 pushed");
    assertTrue(sys.getTotalExpired("nonexistent") == 0, "0 expired");
}

void run_event_notification_feed_system_tests() {
    testNotifFeedCreate();
    testNotifFeedPushAndQuery();
    testNotifFeedMax();
    testNotifFeedReadAndDismiss();
    testNotifFeedCategories();
    testNotifFeedLatest();
    testNotifFeedExpiry();
    testNotifFeedClearAll();
    testNotifFeedClearCategory();
    testNotifFeedUpdate();
    testNotifFeedMissing();
}
