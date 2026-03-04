// Tests for: Fleet History System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/movement_system.h"
#include "systems/fleet_history_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Fleet History System Tests ====================

static void testFleetHistoryEmpty() {
    std::cout << "\n=== Fleet History Empty ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::FleetHistory>(e);

    systems::FleetHistorySystem sys(&world);
    assertTrue(sys.getEventCount("fleet1") == 0, "No events initially");
}

static void testFleetHistoryRecordEvent() {
    std::cout << "\n=== Fleet History Record Event ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet2");
    addComp<components::FleetHistory>(e);

    systems::FleetHistorySystem sys(&world);
    sys.recordEvent("fleet2", "mission_complete", "Cleared pirates", 100.0f, "mission_1");
    assertTrue(sys.getEventCount("fleet2") == 1, "One event recorded");
    auto history = sys.getHistory("fleet2");
    assertTrue(history[0].event_type == "mission_complete", "Correct event type");
}

static void testFleetHistoryMaxEvents() {
    std::cout << "\n=== Fleet History Max Events ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet3");
    auto* hist = addComp<components::FleetHistory>(e);
    hist->max_events = 5;

    systems::FleetHistorySystem sys(&world);
    for (int i = 0; i < 10; i++) {
        sys.recordEvent("fleet3", "event", "desc " + std::to_string(i), static_cast<float>(i));
    }
    assertTrue(sys.getEventCount("fleet3") == 5, "Events trimmed to max");
}

static void testFleetHistoryByType() {
    std::cout << "\n=== Fleet History By Type ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet4");
    addComp<components::FleetHistory>(e);

    systems::FleetHistorySystem sys(&world);
    sys.recordEvent("fleet4", "mission_complete", "Mission 1", 1.0f);
    sys.recordEvent("fleet4", "ship_lost", "Lost a frigate", 2.0f);
    sys.recordEvent("fleet4", "mission_complete", "Mission 2", 3.0f);

    auto missions = sys.getEventsByType("fleet4", "mission_complete");
    assertTrue(static_cast<int>(missions.size()) == 2, "Two mission_complete events");
    auto losses = sys.getEventsByType("fleet4", "ship_lost");
    assertTrue(static_cast<int>(losses.size()) == 1, "One ship_lost event");
}


void run_fleet_history_system_tests() {
    testFleetHistoryEmpty();
    testFleetHistoryRecordEvent();
    testFleetHistoryMaxEvents();
    testFleetHistoryByType();
}
