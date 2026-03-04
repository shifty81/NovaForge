// Tests for: Crew Activity System tests
#include "test_log.h"
#include "components/crew_components.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/crew_activity_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Crew Activity System tests ====================

static void testCrewActivityAssignRoom() {
    std::cout << "\n=== Crew Activity Assign Room ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    crew1->addComponent(std::make_unique<components::CrewActivity>());

    crewActSys.assignRoom("crew1", "engine_room");
    assertTrue(crewActSys.getAssignedRoom("crew1") == "engine_room", "Room assigned");
    assertTrue(crewActSys.getActivity("crew1") == "Idle", "Initially idle");
}

static void testCrewActivityDamageRepair() {
    std::cout << "\n=== Crew Activity Damage Repair ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    crew1->addComponent(std::make_unique<components::CrewActivity>());

    crewActSys.setShipDamaged("crew1", true);
    crewActSys.update(0.1f);
    assertTrue(crewActSys.getActivity("crew1") == "Repairing", "Crew repairs when ship damaged");
}

static void testCrewActivityHunger() {
    std::cout << "\n=== Crew Activity Hunger ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca = std::make_unique<components::CrewActivity>();
    ca->hunger = 0.79f;
    ca->current_activity = components::CrewActivity::Activity::Working;
    crew1->addComponent(std::move(ca));

    // hunger at 0.79, accumulation of 0.003 * delta pushes over 0.8
    crewActSys.update(10.0f);  // hunger += 0.003 * 10 = 0.03 → 0.82
    assertTrue(crewActSys.getActivity("crew1") == "Eating", "Crew eats when hungry");
}

static void testCrewActivityFatigue() {
    std::cout << "\n=== Crew Activity Fatigue ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca = std::make_unique<components::CrewActivity>();
    ca->fatigue = 0.79f;
    ca->current_activity = components::CrewActivity::Activity::Working;
    crew1->addComponent(std::move(ca));

    // fatigue at 0.79, accumulation of 0.002 * delta pushes over 0.8
    crewActSys.update(10.0f);  // fatigue += 0.002 * 10 = 0.02 → 0.81
    assertTrue(crewActSys.getActivity("crew1") == "Resting", "Crew rests when fatigued");
}

static void testCrewActivityGetCrewInActivity() {
    std::cout << "\n=== Crew Activity Get Crew In Activity ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca1 = std::make_unique<components::CrewActivity>();
    ca1->current_activity = components::CrewActivity::Activity::Working;
    crew1->addComponent(std::move(ca1));

    auto* crew2 = world.createEntity("crew2");
    auto ca2 = std::make_unique<components::CrewActivity>();
    ca2->current_activity = components::CrewActivity::Activity::Working;
    crew2->addComponent(std::move(ca2));

    auto* crew3 = world.createEntity("crew3");
    auto ca3 = std::make_unique<components::CrewActivity>();
    ca3->current_activity = components::CrewActivity::Activity::Resting;
    crew3->addComponent(std::move(ca3));

    auto working = crewActSys.getCrewInActivity(components::CrewActivity::Activity::Working);
    assertTrue(working.size() == 2, "Two crew working");
    auto resting = crewActSys.getCrewInActivity(components::CrewActivity::Activity::Resting);
    assertTrue(resting.size() == 1, "One crew resting");
}

static void testCrewActivityTransition() {
    std::cout << "\n=== Crew Activity Transition ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca = std::make_unique<components::CrewActivity>();
    ca->current_activity = components::CrewActivity::Activity::Idle;
    ca->assigned_room_id = "bridge";
    ca->activity_duration = 5.0f;
    crew1->addComponent(std::move(ca));

    // Idle → Walking after activity_duration
    crewActSys.update(6.0f);
    assertTrue(crewActSys.getActivity("crew1") == "Walking", "Crew walks to room after idle");

    // Walking → Manning after another activity_duration
    crewActSys.update(6.0f);
    assertTrue(crewActSys.getActivity("crew1") == "Manning", "Crew mans station after walking");
}


void run_crew_activity_system_tests() {
    testCrewActivityAssignRoom();
    testCrewActivityDamageRepair();
    testCrewActivityHunger();
    testCrewActivityFatigue();
    testCrewActivityGetCrewInActivity();
    testCrewActivityTransition();
}
