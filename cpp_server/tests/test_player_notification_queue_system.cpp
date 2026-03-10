// Tests for: PlayerNotificationQueueSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/player_notification_queue_system.h"

using namespace atlas;

using PNQ  = components::PlayerNotificationQueue;
using Type = PNQ::NotificationType;

// ==================== PlayerNotificationQueueSystem Tests ====================

static void testPNQInit() {
    std::cout << "\n=== PNQ: Init ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(sys.getTotalCount("p1")   == 0, "Zero notifications initially");
    assertTrue(sys.getUnreadCount("p1")  == 0, "Zero unread initially");
    assertTrue(sys.getTotalPushed("p1")  == 0, "Zero pushed initially");
    assertTrue(sys.getTotalExpired("p1") == 0, "Zero expired initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testPNQPush() {
    std::cout << "\n=== PNQ: Push ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.push("p1", "n1", Type::GameEvent, "Test event"), "Push succeeds");
    assertTrue(sys.getTotalCount("p1")  == 1, "Count is 1 after push");
    assertTrue(sys.getUnreadCount("p1") == 1, "Unread is 1 after push");
    assertTrue(sys.getTotalPushed("p1") == 1, "Total pushed is 1");

    assertTrue(!sys.push("p1", "",   Type::GameEvent, "Empty ID rejected"),
               "Empty notif ID rejected");
    assertTrue(!sys.push("p1", "n2", Type::GameEvent, ""),
               "Empty message rejected");
    assertTrue(!sys.push("p1", "n3", Type::GameEvent, "Bad lifetime", -1.0f),
               "Non-positive lifetime rejected");
    assertTrue(sys.getTotalCount("p1") == 1, "Count unchanged after rejections");
}

static void testPNQMarkRead() {
    std::cout << "\n=== PNQ: MarkRead ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.push("p1", "n1", Type::MissionUpdate, "Mission done");
    sys.push("p1", "n2", Type::CombatEvent,   "Ship destroyed");
    assertTrue(sys.getUnreadCount("p1") == 2, "2 unread after 2 pushes");

    assertTrue(sys.markRead("p1", "n1"),     "MarkRead succeeds");
    assertTrue(sys.getUnreadCount("p1") == 1, "1 unread after marking one read");
    assertTrue(!sys.markRead("p1", "nx"),    "MarkRead fails for unknown ID");
    assertTrue(sys.getUnreadCount("p1") == 1, "Count unchanged after failed markRead");
}

static void testPNQMarkAllRead() {
    std::cout << "\n=== PNQ: MarkAllRead ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.push("p1", "n1", Type::TradeEvent,   "Trade complete");
    sys.push("p1", "n2", Type::SystemAlert,  "Jump gate online");
    sys.push("p1", "n3", Type::MissionUpdate,"New mission");
    assertTrue(sys.getUnreadCount("p1") == 3, "3 unread initially");

    assertTrue(sys.markAllRead("p1"), "MarkAllRead succeeds");
    assertTrue(sys.getUnreadCount("p1") == 0, "0 unread after markAllRead");
    assertTrue(sys.getTotalCount("p1")  == 3, "Notifications not deleted by markAllRead");
}

static void testPNQClear() {
    std::cout << "\n=== PNQ: Clear ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.push("p1", "n1", Type::GameEvent, "Event 1");
    sys.push("p1", "n2", Type::GameEvent, "Event 2");
    assertTrue(sys.getTotalCount("p1") == 2, "2 notifications before clear");

    assertTrue(sys.clearNotifications("p1"), "Clear succeeds");
    assertTrue(sys.getTotalCount("p1")   == 0, "0 notifications after clear");
    assertTrue(sys.getUnreadCount("p1")  == 0, "0 unread after clear");
    assertTrue(sys.getTotalPushed("p1")  == 2, "TotalPushed counter preserved");
}

static void testPNQMaxCapacity() {
    std::cout << "\n=== PNQ: MaxCapacity ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    // Fill to default max (50)
    for (int i = 0; i < 50; ++i) {
        sys.push("p1", "n" + std::to_string(i), Type::GameEvent,
                 "Event " + std::to_string(i));
    }
    assertTrue(sys.getTotalCount("p1") == 50, "Queue at max capacity");

    // Push one more — oldest should be evicted
    sys.push("p1", "n_new", Type::SystemAlert, "New event");
    assertTrue(sys.getTotalCount("p1") == 50, "Count stays at max after eviction");
    assertTrue(sys.getTotalPushed("p1") == 51, "TotalPushed still increments");
}

static void testPNQExpiry() {
    std::cout << "\n=== PNQ: Expiry ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.push("p1", "short", Type::GameEvent, "Short-lived", 10.0f);
    sys.push("p1", "long",  Type::GameEvent, "Long-lived",  100.0f);
    assertTrue(sys.getTotalCount("p1") == 2, "2 notifications before expiry tick");

    sys.update(15.0f); // short one expires
    assertTrue(sys.getTotalCount("p1")   == 1, "1 notification after short expires");
    assertTrue(sys.getTotalExpired("p1") == 1, "1 expired counted");

    sys.update(100.0f); // long one expires
    assertTrue(sys.getTotalCount("p1")   == 0, "0 notifications after long expires");
    assertTrue(sys.getTotalExpired("p1") == 2, "2 expired total");
}

static void testPNQGetMostRecentUnread() {
    std::cout << "\n=== PNQ: GetMostRecentUnread ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    // Push with different timestamps via update ticks
    sys.push("p1", "n1", Type::GameEvent, "First");
    sys.update(1.0f);
    sys.push("p1", "n2", Type::CombatEvent, "Second");

    auto latest = sys.getMostRecentUnread("p1");
    assertTrue(latest.id == "n2", "Most recent unread is the second notification");

    sys.markRead("p1", "n2");
    auto after = sys.getMostRecentUnread("p1");
    assertTrue(after.id == "n1", "After marking n2 read, n1 is most recent unread");

    sys.markAllRead("p1");
    auto none = sys.getMostRecentUnread("p1");
    assertTrue(none.id.empty(), "Empty id returned when all read");
}

static void testPNQCountByType() {
    std::cout << "\n=== PNQ: CountByType ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.push("p1", "n1", Type::MissionUpdate, "M1");
    sys.push("p1", "n2", Type::MissionUpdate, "M2");
    sys.push("p1", "n3", Type::CombatEvent,   "C1");
    sys.push("p1", "n4", Type::SystemAlert,   "S1");

    assertTrue(sys.getCountByType("p1", Type::MissionUpdate) == 2, "2 mission updates");
    assertTrue(sys.getCountByType("p1", Type::CombatEvent)   == 1, "1 combat event");
    assertTrue(sys.getCountByType("p1", Type::SystemAlert)   == 1, "1 system alert");
    assertTrue(sys.getCountByType("p1", Type::TradeEvent)    == 0, "0 trade events");
}

static void testPNQMissing() {
    std::cout << "\n=== PNQ: Missing ===" << std::endl;
    ecs::World world;
    systems::PlayerNotificationQueueSystem sys(&world);

    assertTrue(!sys.push("nx", "n1", Type::GameEvent, "msg"), "Push fails on missing");
    assertTrue(!sys.markRead("nx", "n1"),    "MarkRead fails on missing");
    assertTrue(!sys.markAllRead("nx"),       "MarkAllRead fails on missing");
    assertTrue(!sys.clearNotifications("nx"), "Clear fails on missing");
    assertTrue(sys.getTotalCount("nx")   == 0, "0 count on missing");
    assertTrue(sys.getUnreadCount("nx")  == 0, "0 unread on missing");
    assertTrue(sys.getTotalPushed("nx")  == 0, "0 pushed on missing");
    assertTrue(sys.getTotalExpired("nx") == 0, "0 expired on missing");
    assertTrue(sys.getMostRecentUnread("nx").id.empty(), "Empty notification on missing");
    assertTrue(sys.getCountByType("nx", Type::GameEvent) == 0, "0 by type on missing");
}

void run_player_notification_queue_system_tests() {
    testPNQInit();
    testPNQPush();
    testPNQMarkRead();
    testPNQMarkAllRead();
    testPNQClear();
    testPNQMaxCapacity();
    testPNQExpiry();
    testPNQGetMostRecentUnread();
    testPNQCountByType();
    testPNQMissing();
}
