// Tests for: NPCBehaviorSchedulerSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/npc_behavior_scheduler_system.h"

using namespace atlas;
using Behavior = components::NPCBehaviorSchedulerState::Behavior;

// ==================== NPCBehaviorSchedulerSystem Tests ====================

static void testNPCSchedulerCreate() {
    std::cout << "\n=== NPCBehaviorScheduler: Create ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    assertTrue(sys.initialize("npc1", "miner_01", "caldari"), "Init NPC succeeds");
    assertTrue(sys.getFaction("npc1") == "caldari", "Faction caldari");
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Idle), "Idle initially");
    assertTrue(sys.getScheduleEntryCount("npc1") == 0, "Zero schedule entries");
    assertTrue(sys.getTotalTransitions("npc1") == 0, "Zero transitions");
    assertTrue(sys.getTotalCombatEntries("npc1") == 0, "Zero combat entries");
    assertTrue(sys.getTotalTradeTrips("npc1") == 0, "Zero trade trips");
}

static void testNPCSchedulerInvalidInit() {
    std::cout << "\n=== NPCBehaviorScheduler: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    assertTrue(!sys.initialize("missing", "n", "f"), "Missing entity fails");
    world.createEntity("npc1");
    assertTrue(!sys.initialize("npc1", "", "caldari"), "Empty npc_id fails");
    assertTrue(!sys.initialize("npc1", "miner_01", ""), "Empty faction fails");
}

static void testNPCSchedulerAddEntry() {
    std::cout << "\n=== NPCBehaviorScheduler: AddEntry ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");

    assertTrue(sys.addScheduleEntry("npc1", "morning_patrol", static_cast<int>(Behavior::Patrol), 6.0f, 4.0f),
               "Add morning patrol");
    assertTrue(sys.addScheduleEntry("npc1", "midday_trade", static_cast<int>(Behavior::Trade), 10.0f, 3.0f),
               "Add midday trade");
    assertTrue(sys.addScheduleEntry("npc1", "afternoon_mine", static_cast<int>(Behavior::Mine), 13.0f, 5.0f),
               "Add afternoon mine");
    assertTrue(sys.getScheduleEntryCount("npc1") == 3, "3 entries");

    // Duplicate label rejected
    assertTrue(!sys.addScheduleEntry("npc1", "morning_patrol", static_cast<int>(Behavior::Idle), 0.0f, 1.0f),
               "Duplicate label rejected");

    // Invalid inputs
    assertTrue(!sys.addScheduleEntry("npc1", "", static_cast<int>(Behavior::Idle), 0.0f, 1.0f),
               "Empty label rejected");
    assertTrue(!sys.addScheduleEntry("npc1", "x", -1, 0.0f, 1.0f), "Invalid behavior rejected");
    assertTrue(!sys.addScheduleEntry("npc1", "x", 99, 0.0f, 1.0f), "Out-of-range behavior rejected");
    assertTrue(!sys.addScheduleEntry("npc1", "x", 0, -1.0f, 1.0f), "Negative hour rejected");
    assertTrue(!sys.addScheduleEntry("npc1", "x", 0, 24.0f, 1.0f), "Hour >= 24 rejected");
    assertTrue(!sys.addScheduleEntry("npc1", "x", 0, 0.0f, 0.0f), "Zero duration rejected");
    assertTrue(!sys.addScheduleEntry("nonexistent", "x", 0, 0.0f, 1.0f), "Missing entity rejected");
}

static void testNPCSchedulerRemoveEntry() {
    std::cout << "\n=== NPCBehaviorScheduler: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");
    sys.addScheduleEntry("npc1", "patrol", static_cast<int>(Behavior::Patrol), 6.0f, 4.0f);
    sys.addScheduleEntry("npc1", "trade", static_cast<int>(Behavior::Trade), 10.0f, 3.0f);

    assertTrue(sys.removeScheduleEntry("npc1", "patrol"), "Remove patrol succeeds");
    assertTrue(sys.getScheduleEntryCount("npc1") == 1, "1 entry remaining");
    assertTrue(!sys.removeScheduleEntry("npc1", "patrol"), "Double remove fails");
    assertTrue(!sys.removeScheduleEntry("npc1", "nonexistent"), "Remove nonexistent fails");
}

static void testNPCSchedulerClearSchedule() {
    std::cout << "\n=== NPCBehaviorScheduler: ClearSchedule ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");
    sys.addScheduleEntry("npc1", "patrol", static_cast<int>(Behavior::Patrol), 6.0f, 4.0f);
    sys.addScheduleEntry("npc1", "trade", static_cast<int>(Behavior::Trade), 10.0f, 3.0f);

    assertTrue(sys.clearSchedule("npc1"), "Clear succeeds");
    assertTrue(sys.getScheduleEntryCount("npc1") == 0, "Zero entries after clear");
}

static void testNPCSchedulerBehaviorTransition() {
    std::cout << "\n=== NPCBehaviorScheduler: BehaviorTransition ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");
    sys.addScheduleEntry("npc1", "patrol", static_cast<int>(Behavior::Patrol), 6.0f, 4.0f);
    sys.addScheduleEntry("npc1", "trade", static_cast<int>(Behavior::Trade), 10.0f, 3.0f);

    // Set hour to 7 (within patrol window 6-10)
    sys.setGameHour("npc1", 7.0f);
    sys.update(0.1f);
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Patrol),
               "Patrol at hour 7");
    assertTrue(sys.getTotalTransitions("npc1") == 1, "1 transition");

    // Set hour to 11 (within trade window 10-13)
    sys.setGameHour("npc1", 11.0f);
    sys.update(0.1f);
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Trade),
               "Trade at hour 11");
    assertTrue(sys.getPreviousBehavior("npc1") == static_cast<int>(Behavior::Patrol),
               "Previous was patrol");
    assertTrue(sys.getTotalTransitions("npc1") == 2, "2 transitions");
    assertTrue(sys.getTotalTradeTrips("npc1") == 1, "1 trade trip");
}

static void testNPCSchedulerThreatOverride() {
    std::cout << "\n=== NPCBehaviorScheduler: ThreatOverride ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");
    sys.addScheduleEntry("npc1", "patrol", static_cast<int>(Behavior::Patrol), 0.0f, 24.0f);

    sys.setGameHour("npc1", 12.0f);
    sys.update(0.1f);
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Patrol),
               "Patrol normally");

    // High threat → combat
    sys.setThreatLevel("npc1", 0.6f);
    sys.update(0.1f);
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Combat),
               "Combat when threat high");
    assertTrue(sys.getTotalCombatEntries("npc1") == 1, "1 combat entry");

    // Very high threat → flee
    sys.setThreatLevel("npc1", 0.9f);
    sys.update(0.1f);
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Flee),
               "Flee when threat very high");
}

static void testNPCSchedulerSetThreatThreshold() {
    std::cout << "\n=== NPCBehaviorScheduler: SetThreatThreshold ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");

    assertTrue(sys.setThreatThreshold("npc1", 0.7f), "Set threshold to 0.7");
    assertTrue(!sys.setThreatThreshold("npc1", -0.1f), "Negative threshold rejected");
    assertTrue(!sys.setThreatThreshold("npc1", 1.1f), "Threshold > 1 rejected");
    assertTrue(!sys.setThreatThreshold("nonexistent", 0.5f), "Missing entity rejected");
}

static void testNPCSchedulerForceBehavior() {
    std::cout << "\n=== NPCBehaviorScheduler: ForceBehavior ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");

    assertTrue(sys.forceBehavior("npc1", static_cast<int>(Behavior::Dock)),
               "Force dock succeeds");
    assertTrue(sys.getCurrentBehavior("npc1") == static_cast<int>(Behavior::Dock),
               "Behavior is dock");
    assertTrue(sys.getPreviousBehavior("npc1") == static_cast<int>(Behavior::Idle),
               "Previous was idle");
    assertTrue(sys.getTotalTransitions("npc1") == 1, "1 transition");

    assertTrue(!sys.forceBehavior("npc1", -1), "Invalid behavior rejected");
    assertTrue(!sys.forceBehavior("npc1", 99), "Out-of-range rejected");
    assertTrue(!sys.forceBehavior("nonexistent", 0), "Missing entity rejected");
}

static void testNPCSchedulerGameHour() {
    std::cout << "\n=== NPCBehaviorScheduler: GameHour ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");

    assertTrue(sys.setGameHour("npc1", 12.0f), "Set hour to 12");
    assertTrue(approxEqual(sys.getGameHour("npc1"), 12.0f), "Hour is 12");

    assertTrue(!sys.setGameHour("npc1", -1.0f), "Negative hour rejected");
    assertTrue(!sys.setGameHour("npc1", 24.0f), "Hour >= 24 rejected");
    assertTrue(!sys.setGameHour("nonexistent", 12.0f), "Missing entity rejected");
}

static void testNPCSchedulerUpdate() {
    std::cout << "\n=== NPCBehaviorScheduler: Update ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1", "miner_01", "caldari");
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testNPCSchedulerMissing() {
    std::cout << "\n=== NPCBehaviorScheduler: Missing ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorSchedulerSystem sys(&world);
    assertTrue(sys.getCurrentBehavior("x") == 0, "Default behavior on missing");
    assertTrue(sys.getPreviousBehavior("x") == 0, "Default prev on missing");
    assertTrue(sys.getScheduleEntryCount("x") == 0, "Default entries on missing");
    assertTrue(approxEqual(sys.getGameHour("x"), 0.0f), "Default hour on missing");
    assertTrue(approxEqual(sys.getThreatLevel("x"), 0.0f), "Default threat on missing");
    assertTrue(sys.getTotalTransitions("x") == 0, "Default transitions on missing");
    assertTrue(sys.getTotalCombatEntries("x") == 0, "Default combat on missing");
    assertTrue(sys.getTotalTradeTrips("x") == 0, "Default trade on missing");
    assertTrue(sys.getFaction("x").empty(), "Default faction on missing");
}

void run_npc_behavior_scheduler_system_tests() {
    testNPCSchedulerCreate();
    testNPCSchedulerInvalidInit();
    testNPCSchedulerAddEntry();
    testNPCSchedulerRemoveEntry();
    testNPCSchedulerClearSchedule();
    testNPCSchedulerBehaviorTransition();
    testNPCSchedulerThreatOverride();
    testNPCSchedulerSetThreatThreshold();
    testNPCSchedulerForceBehavior();
    testNPCSchedulerGameHour();
    testNPCSchedulerUpdate();
    testNPCSchedulerMissing();
}
