// Tests for: FleetMoraleSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_morale_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== FleetMoraleSystem Tests ====================

static void testFleetMoraleRecordWin() {
    std::cout << "\n=== Fleet Morale Record Win ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordWin("cap1");
    assertTrue(sys.getMoraleScore("cap1") > 0.0f, "Morale score positive after win");
    assertTrue(sys.getMoraleState("cap1") == "Steady", "Morale state is Steady after one win");
}

static void testFleetMoraleRecordLoss() {
    std::cout << "\n=== Fleet Morale Record Loss ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordLoss("cap1");
    assertTrue(sys.getMoraleScore("cap1") < 0.0f, "Morale score negative after loss");
}

static void testFleetMoraleMultipleEvents() {
    std::cout << "\n=== Fleet Morale Multiple Events ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    for (int i = 0; i < 10; i++) {
        sys.recordWin("cap1");
    }
    // 10 wins * 1.0 = 10, but let's accumulate: each recordWin increments wins
    // After 10 wins: score = 10 * 1.0 = 10 ... need >= 50
    // Actually wins accumulate: after 10 calls, wins=10, score=10. Need 50 wins for 50.
    for (int i = 0; i < 40; i++) {
        sys.recordWin("cap1");
    }
    assertTrue(sys.getMoraleScore("cap1") >= 50.0f, "Morale >= 50 after 50 wins");
    assertTrue(sys.getMoraleState("cap1") == "Inspired", "Morale state Inspired at high morale");
}

static void testFleetMoraleLossStreak() {
    std::cout << "\n=== Fleet Morale Loss Streak ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    for (int i = 0; i < 5; i++) {
        sys.recordLoss("cap1");
    }
    sys.recordShipLost("cap1");
    sys.recordShipLost("cap1");
    // score = 0*1 - 5*1.5 - 2*2.0 + 0 = -11.5 => Doubtful
    std::string state = sys.getMoraleState("cap1");
    assertTrue(state == "Doubtful" || state == "Disengaged",
               "Morale state Doubtful or Disengaged after losses");
}

static void testFleetMoraleSavedByPlayer() {
    std::cout << "\n=== Fleet Morale Saved By Player ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordSavedByPlayer("cap1");
    assertTrue(sys.getMoraleScore("cap1") > 0.0f, "Saved by player increases morale");
}

static void testFleetMoraleMissionTogether() {
    std::cout << "\n=== Fleet Morale Mission Together ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    sys.recordMissionTogether("cap1");
    auto* morale = entity->getComponent<components::FleetMorale>();
    assertTrue(morale != nullptr, "FleetMorale component created");
    assertTrue(morale->missions_together == 1, "Missions together counter incremented");
}


void run_fleet_morale_system_tests() {
    testFleetMoraleRecordWin();
    testFleetMoraleRecordLoss();
    testFleetMoraleMultipleEvents();
    testFleetMoraleLossStreak();
    testFleetMoraleSavedByPlayer();
    testFleetMoraleMissionTogether();
}
