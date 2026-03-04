// Tests for: Mission Generator System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/mission_generator_system.h"
#include "systems/mission_template_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mission Generator System Tests ====================

static void testMissionGeneratorGeneratesMissions() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    // Create a system entity with DifficultyZone
    auto* sys_entity = world.createEntity("test_system");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->security_status = 0.5f;

    int count = genSys.generateMissionsForSystem("test_system", 42);
    assertTrue(count > 0, "Generator produces missions");
}

static void testMissionGeneratorAvailableMissions() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    auto* sys_entity = world.createEntity("sys1");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->security_status = 0.5f;

    genSys.generateMissionsForSystem("sys1", 99);
    auto available = genSys.getAvailableMissions("sys1");
    assertTrue(!available.empty(), "Available missions list is not empty");
}

static void testMissionGeneratorOfferToPlayer() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    auto* sys_entity = world.createEntity("sys1");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->security_status = 0.5f;

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    genSys.generateMissionsForSystem("sys1", 42);
    bool offered = genSys.offerMissionToPlayer("player1", "sys1", 0);
    assertTrue(offered, "Mission offered successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(!tracker->active_missions.empty(), "Player has active mission after offer");
}

static void testMissionGeneratorInvalidIndex() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    bool offered = genSys.offerMissionToPlayer("player1", "nonexistent", 0);
    assertTrue(!offered, "Invalid system returns false");
}


void run_mission_generator_system_tests() {
    testMissionGeneratorGeneratesMissions();
    testMissionGeneratorAvailableMissions();
    testMissionGeneratorOfferToPlayer();
    testMissionGeneratorInvalidIndex();
}
