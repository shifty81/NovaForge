// Tests for: NotificationSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/notification_system.h"

using namespace atlas;

// ==================== NotificationSystem Tests ====================

static void testNotificationInit() {
    std::cout << "\n=== Notification: Init ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getNotificationCount("e1") == 0, "Zero notifications initially");
    assertTrue(sys.getUnreadCount("e1") == 0, "Zero unread initially");
    assertTrue(sys.getTotalNotificationsSent("e1") == 0, "Zero total sent");
    assertTrue(sys.getTotalExpired("e1") == 0, "Zero expired");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testNotificationSend() {
    std::cout << "\n=== Notification: Send ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    assertTrue(sys.sendNotification("e1", "n1", "You gained XP!", NT::Achievement, 60.0f),
               "Send notification");
    assertTrue(sys.getNotificationCount("e1") == 1, "1 notification");
    assertTrue(sys.hasNotification("e1", "n1"), "Has n1");
    assertTrue(sys.getNotificationMessage("e1", "n1") == "You gained XP!", "Message matches");
    assertTrue(sys.getUnreadCount("e1") == 1, "1 unread");
    assertTrue(sys.getTotalNotificationsSent("e1") == 1, "1 total sent");
    assertTrue(!sys.isRead("e1", "n1"), "n1 not read initially");

    assertTrue(sys.sendNotification("e1", "n2", "Ship lost!", NT::Combat, 120.0f),
               "Send second notification");
    assertTrue(sys.getNotificationCount("e1") == 2, "2 notifications");
    assertTrue(sys.getUnreadCount("e1") == 2, "2 unread");
}

static void testNotificationSendValidation() {
    std::cout << "\n=== Notification: SendValidation ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    assertTrue(!sys.sendNotification("e1", "", "msg", NT::Info, 60), "Empty id rejected");
    assertTrue(!sys.sendNotification("e1", "n1", "", NT::Info, 60), "Empty message rejected");
    assertTrue(!sys.sendNotification("e1", "n1", "msg", NT::Info, 0), "Zero lifetime rejected");
    assertTrue(!sys.sendNotification("e1", "n1", "msg", NT::Info, -10), "Negative lifetime rejected");
    assertTrue(sys.sendNotification("e1", "n1", "Good msg", NT::Info, 60), "Valid send");
    assertTrue(!sys.sendNotification("e1", "n1", "Dup", NT::Info, 60), "Duplicate id rejected");
    assertTrue(sys.getNotificationCount("e1") == 1, "Still 1 notification");
    assertTrue(!sys.sendNotification("missing", "n2", "msg", NT::Info, 60), "Missing entity rejected");
}

static void testNotificationCapPurge() {
    std::cout << "\n=== Notification: CapPurge ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxNotifications("e1", 3);

    using NT = components::NotificationState::NotifType;
    assertTrue(sys.sendNotification("e1", "n1", "msg1", NT::Info, 60), "Send 1");
    assertTrue(sys.sendNotification("e1", "n2", "msg2", NT::Info, 60), "Send 2");
    assertTrue(sys.sendNotification("e1", "n3", "msg3", NT::Info, 60), "Send 3 at cap");
    // n4 purges n1 (oldest)
    assertTrue(sys.sendNotification("e1", "n4", "msg4", NT::Info, 60), "Send 4, oldest purged");
    assertTrue(sys.getNotificationCount("e1") == 3, "Still 3 notifications");
    assertTrue(!sys.hasNotification("e1", "n1"), "n1 purged");
    assertTrue(sys.hasNotification("e1", "n4"), "n4 present");
    assertTrue(sys.getTotalNotificationsSent("e1") == 4, "4 total sent");
}

static void testNotificationMarkRead() {
    std::cout << "\n=== Notification: MarkRead ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    sys.sendNotification("e1", "n1", "msg1", NT::Info, 60);
    sys.sendNotification("e1", "n2", "msg2", NT::Warning, 60);

    assertTrue(sys.getUnreadCount("e1") == 2, "2 unread");
    assertTrue(sys.markRead("e1", "n1"), "Mark n1 read");
    assertTrue(sys.isRead("e1", "n1"), "n1 is read");
    assertTrue(sys.getUnreadCount("e1") == 1, "1 unread");
    assertTrue(!sys.markRead("e1", "n99"), "Mark missing fails");
    assertTrue(!sys.markRead("missing", "n1"), "Mark missing entity fails");
}

static void testNotificationMarkAllRead() {
    std::cout << "\n=== Notification: MarkAllRead ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    sys.sendNotification("e1", "n1", "msg1", NT::Info, 60);
    sys.sendNotification("e1", "n2", "msg2", NT::Warning, 60);
    sys.sendNotification("e1", "n3", "msg3", NT::Error, 60);

    assertTrue(sys.getUnreadCount("e1") == 3, "3 unread");
    assertTrue(sys.markAllRead("e1"), "MarkAllRead succeeds");
    assertTrue(sys.getUnreadCount("e1") == 0, "0 unread after markAll");
    assertTrue(sys.isRead("e1", "n1"), "n1 is read");
    assertTrue(sys.isRead("e1", "n3"), "n3 is read");
}

static void testNotificationRemove() {
    std::cout << "\n=== Notification: Remove ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    sys.sendNotification("e1", "n1", "msg1", NT::Info, 60);
    sys.sendNotification("e1", "n2", "msg2", NT::Warning, 60);

    assertTrue(sys.removeNotification("e1", "n1"), "Remove n1");
    assertTrue(sys.getNotificationCount("e1") == 1, "1 left");
    assertTrue(!sys.hasNotification("e1", "n1"), "n1 gone");
    assertTrue(sys.hasNotification("e1", "n2"), "n2 present");
    assertTrue(!sys.removeNotification("e1", "n1"), "Remove already gone fails");
    assertTrue(!sys.removeNotification("e1", "unknown"), "Remove unknown fails");
}

static void testNotificationClearAll() {
    std::cout << "\n=== Notification: ClearAll ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    sys.sendNotification("e1", "n1", "msg1", NT::Info, 60);
    sys.sendNotification("e1", "n2", "msg2", NT::Warning, 60);

    assertTrue(sys.clearAll("e1"), "ClearAll succeeds");
    assertTrue(sys.getNotificationCount("e1") == 0, "0 after clear");
    assertTrue(!sys.hasNotification("e1", "n1"), "n1 gone");
    assertTrue(sys.getTotalNotificationsSent("e1") == 2, "Total sent preserved");
}

static void testNotificationCountByType() {
    std::cout << "\n=== Notification: CountByType ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    sys.sendNotification("e1", "n1", "msg1", NT::Info, 60);
    sys.sendNotification("e1", "n2", "msg2", NT::Info, 60);
    sys.sendNotification("e1", "n3", "msg3", NT::Combat, 60);
    sys.sendNotification("e1", "n4", "msg4", NT::Achievement, 60);
    sys.sendNotification("e1", "n5", "msg5", NT::Trade, 60);

    assertTrue(sys.getCountByType("e1", NT::Info) == 2, "2 info");
    assertTrue(sys.getCountByType("e1", NT::Combat) == 1, "1 combat");
    assertTrue(sys.getCountByType("e1", NT::Achievement) == 1, "1 achievement");
    assertTrue(sys.getCountByType("e1", NT::Trade) == 1, "1 trade");
    assertTrue(sys.getCountByType("e1", NT::Warning) == 0, "0 warning");
    assertTrue(sys.getCountByType("e1", NT::Error) == 0, "0 error");
}

static void testNotificationExpiry() {
    std::cout << "\n=== Notification: Expiry ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::NotificationState::NotifType;
    // Short lifetime notification
    sys.sendNotification("e1", "n1", "Short-lived", NT::Info, 2.0f);
    sys.sendNotification("e1", "n2", "Long-lived", NT::Info, 100.0f);
    assertTrue(sys.getNotificationCount("e1") == 2, "2 before expiry");

    // Tick 3 seconds — n1 should expire (age 3 > lifetime 2)
    sys.update(3.0f);
    assertTrue(sys.getNotificationCount("e1") == 1, "1 after expiry");
    assertTrue(!sys.hasNotification("e1", "n1"), "n1 expired");
    assertTrue(sys.hasNotification("e1", "n2"), "n2 still alive");
    assertTrue(sys.getTotalExpired("e1") == 1, "1 total expired");
}

static void testNotificationMissing() {
    std::cout << "\n=== Notification: Missing ===" << std::endl;
    ecs::World world;
    systems::NotificationSystem sys(&world);

    using NT = components::NotificationState::NotifType;
    assertTrue(!sys.sendNotification("none", "n1", "msg", NT::Info, 60),
               "Send fails on missing");
    assertTrue(!sys.markRead("none", "n1"), "MarkRead fails on missing");
    assertTrue(!sys.markAllRead("none"), "MarkAllRead fails on missing");
    assertTrue(!sys.removeNotification("none", "n1"), "Remove fails on missing");
    assertTrue(!sys.clearAll("none"), "ClearAll fails on missing");
    assertTrue(!sys.setMaxNotifications("none", 10), "SetMax fails on missing");
    assertTrue(sys.getNotificationCount("none") == 0, "0 count on missing");
    assertTrue(sys.getUnreadCount("none") == 0, "0 unread on missing");
    assertTrue(!sys.hasNotification("none", "n1"), "No notif on missing");
    assertTrue(!sys.isRead("none", "n1"), "Not read on missing");
    assertTrue(sys.getTotalNotificationsSent("none") == 0, "0 total on missing");
    assertTrue(sys.getTotalExpired("none") == 0, "0 expired on missing");
    assertTrue(sys.getCountByType("none", NT::Info) == 0, "0 type on missing");
    assertTrue(sys.getNotificationMessage("none", "n1") == "", "Empty msg on missing");
}

void run_notification_system_tests() {
    testNotificationInit();
    testNotificationSend();
    testNotificationSendValidation();
    testNotificationCapPurge();
    testNotificationMarkRead();
    testNotificationMarkAllRead();
    testNotificationRemove();
    testNotificationClearAll();
    testNotificationCountByType();
    testNotificationExpiry();
    testNotificationMissing();
}
