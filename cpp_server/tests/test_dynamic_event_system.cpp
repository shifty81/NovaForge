// Tests for: DynamicEventSystem Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/dynamic_event_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== DynamicEventSystem Tests ====================

static void testDynamicEventCreate() {
    std::cout << "\n=== DynamicEvent: Create ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    assertTrue(sys.createEventManager("world1"), "Create event manager succeeds");
    assertTrue(sys.getActiveEventCount("world1") == 0, "Initial active events is 0");
    assertTrue(sys.getTotalCompleted("world1") == 0, "Initial total completed is 0");
}

static void testDynamicEventSchedule() {
    std::cout << "\n=== DynamicEvent: Schedule ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    assertTrue(sys.scheduleEvent("world1", "ev1", "PirateInvasion", 100.0f, 0.8f), "Schedule succeeds");
    assertTrue(sys.getEventState("world1", "ev1") == "Pending", "New event is Pending");
    assertTrue(sys.getEventType("world1", "ev1") == "PirateInvasion", "Event type is PirateInvasion");
    assertTrue(approxEqual(sys.getIntensity("world1", "ev1"), 0.8f), "Intensity is 0.8");
}

static void testDynamicEventStart() {
    std::cout << "\n=== DynamicEvent: Start ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "ResourceSurge", 60.0f, 0.5f);
    assertTrue(sys.startEvent("world1", "ev1"), "Manual start succeeds");
    assertTrue(sys.getEventState("world1", "ev1") == "Active", "Event is Active after start");
    assertTrue(sys.getActiveEventCount("world1") == 1, "Active event count is 1");
}

static void testDynamicEventJoinLeave() {
    std::cout << "\n=== DynamicEvent: JoinLeave ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "AnomalyStorm", 120.0f, 0.6f);
    sys.startEvent("world1", "ev1");
    assertTrue(sys.joinEvent("world1", "ev1", "player1"), "Join event succeeds");
    assertTrue(sys.getParticipantCount("world1", "ev1") == 1, "1 participant");
    assertTrue(sys.joinEvent("world1", "ev1", "player2"), "Second join succeeds");
    assertTrue(sys.getParticipantCount("world1", "ev1") == 2, "2 participants");
    assertTrue(!sys.joinEvent("world1", "ev1", "player1"), "Duplicate join rejected");
    assertTrue(sys.leaveEvent("world1", "ev1", "player1"), "Leave succeeds");
    assertTrue(sys.getParticipantCount("world1", "ev1") == 1, "1 participant after leave");
}

static void testDynamicEventLifecycle() {
    std::cout << "\n=== DynamicEvent: Lifecycle ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "TradeBonus", 100.0f, 0.5f);
    sys.startEvent("world1", "ev1");
    sys.update(79.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Active", "Still Active at 79%");
    sys.update(1.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Concluding", "Concluding at 80%");
    sys.update(20.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Completed", "Completed at 100%");
    assertTrue(sys.getTotalCompleted("world1") == 1, "Total completed is 1");
}

static void testDynamicEventRewardPool() {
    std::cout << "\n=== DynamicEvent: RewardPool ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "SecurityAlert", 100.0f, 1.0f);
    sys.startEvent("world1", "ev1");
    assertTrue(approxEqual(sys.getRewardPool("world1", "ev1"), 0.0f), "Initial reward pool is 0");
    sys.update(10.0f);
    assertTrue(sys.getRewardPool("world1", "ev1") > 0.0f, "Reward pool accumulates");
    float pool = sys.getRewardPool("world1", "ev1");
    assertTrue(approxEqual(pool, 1000.0f), "10s at intensity 1.0 yields 1000 reward");
}

static void testDynamicEventAutoStart() {
    std::cout << "\n=== DynamicEvent: AutoStart ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "FactionConflict", 50.0f, 0.7f);
    assertTrue(sys.getEventState("world1", "ev1") == "Pending", "Starts as Pending");
    sys.update(4.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Pending", "Still Pending at 4s");
    sys.update(2.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Active", "Auto-starts after delay");
}

static void testDynamicEventCancel() {
    std::cout << "\n=== DynamicEvent: Cancel ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "PirateInvasion", 100.0f, 0.5f);
    sys.startEvent("world1", "ev1");
    assertTrue(sys.cancelEvent("world1", "ev1"), "Cancel active event succeeds");
    assertTrue(sys.getEventState("world1", "ev1") == "Completed", "Cancelled event is Completed");
    assertTrue(!sys.cancelEvent("world1", "ev1"), "Cannot cancel already Completed event");
}

static void testDynamicEventMaxConcurrent() {
    std::cout << "\n=== DynamicEvent: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    assertTrue(sys.scheduleEvent("world1", "e1", "PirateInvasion", 100.0f, 0.5f), "Schedule 1 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e2", "ResourceSurge", 100.0f, 0.5f), "Schedule 2 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e3", "AnomalyStorm", 100.0f, 0.5f), "Schedule 3 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e4", "TradeBonus", 100.0f, 0.5f), "Schedule 4 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e5", "SecurityAlert", 100.0f, 0.5f), "Schedule 5 succeeds");
    assertTrue(!sys.scheduleEvent("world1", "e6", "FactionConflict", 100.0f, 0.5f), "Schedule 6 rejected (max 5)");
}

static void testDynamicEventMissing() {
    std::cout << "\n=== DynamicEvent: Missing ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    assertTrue(!sys.createEventManager("nonexistent"), "Create fails on missing entity");
    assertTrue(!sys.scheduleEvent("nonexistent", "x", "PirateInvasion", 100.0f, 0.5f), "Schedule fails on missing");
    assertTrue(!sys.startEvent("nonexistent", "x"), "Start fails on missing");
    assertTrue(!sys.joinEvent("nonexistent", "x", "p1"), "Join fails on missing");
    assertTrue(!sys.leaveEvent("nonexistent", "x", "p1"), "Leave fails on missing");
    assertTrue(sys.getEventState("nonexistent", "x") == "", "Empty state on missing");
    assertTrue(sys.getActiveEventCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getElapsedTime("nonexistent", "x"), 0.0f), "0 elapsed on missing");
    assertTrue(!sys.cancelEvent("nonexistent", "x"), "Cancel fails on missing");
}


void run_dynamic_event_system_tests() {
    testDynamicEventCreate();
    testDynamicEventSchedule();
    testDynamicEventStart();
    testDynamicEventJoinLeave();
    testDynamicEventLifecycle();
    testDynamicEventRewardPool();
    testDynamicEventAutoStart();
    testDynamicEventCancel();
    testDynamicEventMaxConcurrent();
    testDynamicEventMissing();
}
