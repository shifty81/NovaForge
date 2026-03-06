// Tests for: NPC Schedule System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/npc_schedule_system.h"

using namespace atlas;

// ==================== NPC Schedule System Tests ====================

static void testNPCScheduleCreate() {
    std::cout << "\n=== NPCSchedule: Create ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    assertTrue(sys.initialize("npc1"), "Init succeeds");
    assertTrue(sys.getEntryCount("npc1") == 0, "No entries initially");
    assertTrue(sys.getCurrentActivity("npc1") == 0, "Idle initially");
    assertTrue(approxEqual(sys.getCurrentHour("npc1"), 0.0f), "Hour is 0");
    assertTrue(sys.getTransitions("npc1") == 0, "No transitions");
    assertTrue(sys.getDaysCompleted("npc1") == 0, "No days completed");
    assertTrue(approxEqual(sys.getAdherenceScore("npc1"), 1.0f), "Full adherence");
}

static void testNPCScheduleAddEntry() {
    std::cout << "\n=== NPCSchedule: AddEntry ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    // Activity::Mining = 1, start 6:00, end 12:00, location "Belt-Alpha", priority 3
    assertTrue(sys.addEntry("npc1", 1, 6.0f, 12.0f, "Belt-Alpha", 3), "Add mining entry");
    // Activity::Resting = 5, start 22:00, end 6:00, location "Station-1", priority 2
    assertTrue(sys.addEntry("npc1", 5, 22.0f, 6.0f, "Station-1", 2), "Add resting entry");
    assertTrue(sys.getEntryCount("npc1") == 2, "2 entries added");
}

static void testNPCScheduleRemoveEntry() {
    std::cout << "\n=== NPCSchedule: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    sys.addEntry("npc1", 1, 6.0f, 12.0f, "Belt-Alpha", 3);
    sys.addEntry("npc1", 3, 12.0f, 18.0f, "Market-Hub", 2);
    assertTrue(sys.removeEntry("npc1", 0), "Remove first entry");
    assertTrue(sys.getEntryCount("npc1") == 1, "1 entry remains");
    assertTrue(!sys.removeEntry("npc1", 5), "Out of bounds remove fails");
}

static void testNPCScheduleTimeAdvance() {
    std::cout << "\n=== NPCSchedule: TimeAdvance ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    // Set a short day length for testing: 240 seconds = 4 minutes
    auto* entity = world.getEntity("npc1");
    auto* sched = entity->getComponent<components::NPCSchedule>();
    sched->day_length = 240.0f;

    // Mining from 6:00 to 12:00
    sys.addEntry("npc1", 1, 6.0f, 12.0f, "Belt-Alpha", 3);

    // Advance to hour 6 (60 seconds into 240-second day)
    sys.update(60.0f);
    float hour = sys.getCurrentHour("npc1");
    assertTrue(hour >= 5.9f && hour <= 6.1f, "Hour is ~6");
    assertTrue(sys.getCurrentActivity("npc1") == 1, "Mining at hour 6");
    assertTrue(sys.getTransitions("npc1") == 1, "1 transition (Idle->Mining)");
}

static void testNPCScheduleDayCompletion() {
    std::cout << "\n=== NPCSchedule: DayCompletion ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* sched = entity->getComponent<components::NPCSchedule>();
    sched->day_length = 100.0f;  // 100-second day

    sys.update(100.0f);
    assertTrue(sys.getDaysCompleted("npc1") == 1, "1 day completed");
    sys.update(100.0f);
    assertTrue(sys.getDaysCompleted("npc1") == 2, "2 days completed");
}

static void testNPCSchedulePriorityResolution() {
    std::cout << "\n=== NPCSchedule: PriorityResolution ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* sched = entity->getComponent<components::NPCSchedule>();
    sched->day_length = 240.0f;

    // Overlapping entries: Mining (prio 2) and Patrolling (prio 4) both at 6-12
    sys.addEntry("npc1", 1, 6.0f, 12.0f, "Belt-Alpha", 2);
    sys.addEntry("npc1", 4, 6.0f, 12.0f, "Sector-7", 4);

    // Advance to hour 8
    sys.update(80.0f);  // 80/240 * 24 = 8
    assertTrue(sys.getCurrentActivity("npc1") == 4, "Higher priority patrol wins");
    assertTrue(sys.getCurrentLocation("npc1") == "Sector-7", "Location from higher priority");
}

static void testNPCScheduleWrappingEntry() {
    std::cout << "\n=== NPCSchedule: WrappingEntry ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* sched = entity->getComponent<components::NPCSchedule>();
    sched->day_length = 240.0f;

    // Resting from 22:00 to 6:00 (wrapping)
    sys.addEntry("npc1", 5, 22.0f, 6.0f, "Station-1", 3);

    // Advance to hour 23 (230/240*24 = 23)
    sys.update(230.0f);
    assertTrue(sys.getCurrentActivity("npc1") == 5, "Resting at hour 23 (wrapping entry)");

    // Advance to hour 2 (wraps around) — need another day cycle portion
    // Reset: total will be 230 + 20 = 250 → wraps to 10s → hour = (10/240)*24 = 1.0
    sys.update(20.0f);
    assertTrue(sys.getCurrentActivity("npc1") == 5, "Still resting at hour 1 (wrapping entry)");
}

static void testNPCScheduleSetHour() {
    std::cout << "\n=== NPCSchedule: SetHour ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    assertTrue(sys.setCurrentHour("npc1", 15.0f), "Set hour succeeds");
    assertTrue(approxEqual(sys.getCurrentHour("npc1"), 15.0f), "Hour is 15");
    assertTrue(sys.setCurrentHour("npc1", 30.0f), "Wrapping hour succeeds");
    assertTrue(approxEqual(sys.getCurrentHour("npc1"), 6.0f), "Hour wraps to 6");
}

static void testNPCScheduleMaxEntries() {
    std::cout << "\n=== NPCSchedule: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* sched = entity->getComponent<components::NPCSchedule>();
    sched->max_entries = 2;

    sys.addEntry("npc1", 1, 0.0f, 8.0f, "A", 1);
    sys.addEntry("npc1", 2, 8.0f, 16.0f, "B", 1);
    assertTrue(!sys.addEntry("npc1", 3, 16.0f, 24.0f, "C", 1), "Max entries enforced");
}

static void testNPCScheduleInvalidActivity() {
    std::cout << "\n=== NPCSchedule: InvalidActivity ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    assertTrue(!sys.addEntry("npc1", -1, 0.0f, 8.0f, "A", 1), "Negative activity rejected");
    assertTrue(!sys.addEntry("npc1", 7, 0.0f, 8.0f, "A", 1), "Out-of-range activity rejected");
    assertTrue(!sys.addEntry("npc1", 1, 0.0f, 8.0f, "A", 0), "Priority 0 rejected");
    assertTrue(!sys.addEntry("npc1", 1, 0.0f, 8.0f, "A", 6), "Priority 6 rejected");
}

static void testNPCScheduleMissing() {
    std::cout << "\n=== NPCSchedule: Missing ===" << std::endl;
    ecs::World world;
    systems::NPCScheduleSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addEntry("nonexistent", 1, 0.0f, 8.0f, "A", 1), "Add fails on missing");
    assertTrue(!sys.removeEntry("nonexistent", 0), "Remove fails on missing");
    assertTrue(!sys.clearSchedule("nonexistent"), "Clear fails on missing");
    assertTrue(!sys.setCurrentHour("nonexistent", 5.0f), "SetHour fails on missing");
    assertTrue(!sys.setDayLength("nonexistent", 100.0f), "SetDayLength fails on missing");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(sys.getCurrentActivity("nonexistent") == 0, "Idle on missing");
    assertTrue(approxEqual(sys.getCurrentHour("nonexistent"), 0.0f), "0 hour on missing");
    assertTrue(sys.getTransitions("nonexistent") == 0, "0 transitions on missing");
    assertTrue(sys.getDaysCompleted("nonexistent") == 0, "0 days on missing");
    assertTrue(approxEqual(sys.getAdherenceScore("nonexistent"), 0.0f), "0 adherence on missing");
    assertTrue(sys.getCurrentLocation("nonexistent") == "", "Empty location on missing");
}


void run_npc_schedule_system_tests() {
    testNPCScheduleCreate();
    testNPCScheduleAddEntry();
    testNPCScheduleRemoveEntry();
    testNPCScheduleTimeAdvance();
    testNPCScheduleDayCompletion();
    testNPCSchedulePriorityResolution();
    testNPCScheduleWrappingEntry();
    testNPCScheduleSetHour();
    testNPCScheduleMaxEntries();
    testNPCScheduleInvalidActivity();
    testNPCScheduleMissing();
}
