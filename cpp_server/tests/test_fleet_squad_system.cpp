// Tests for: FleetSquadSystem Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_squad_system.h"

using namespace atlas;

// ==================== FleetSquadSystem Tests ====================

static void testFleetSquadCreate() {
    std::cout << "\n=== Fleet Squad: Create ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    assertTrue(sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault), "Squad created");
    assertTrue(sys.getMemberCount("squad1") == 1, "Leader is first member");
    assertTrue(sys.getLeaderId("squad1") == "leader1", "Leader ID correct");
    assertTrue(sys.getRole("squad1") == "assault", "Role is assault");
    assertTrue(!sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault), "Duplicate create rejected");
}

static void testFleetSquadDissolve() {
    std::cout << "\n=== Fleet Squad: Dissolve ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.dissolveSquad("squad1"), "Squad dissolved");
    assertTrue(sys.getMemberCount("squad1") == 0, "No members after dissolve");
    assertTrue(!sys.dissolveSquad("squad1"), "Dissolve on missing fails");
}

static void testFleetSquadAddMember() {
    std::cout << "\n=== Fleet Squad: Add Member ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.addMember("squad1", "member2"), "Member added");
    assertTrue(sys.getMemberCount("squad1") == 2, "2 members now");
    assertTrue(!sys.addMember("squad1", "member2"), "Duplicate member rejected");
}

static void testFleetSquadMaxMembers() {
    std::cout << "\n=== Fleet Squad: Max Members ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "m2");
    sys.addMember("squad1", "m3");
    sys.addMember("squad1", "m4");
    sys.addMember("squad1", "m5");
    assertTrue(sys.getMemberCount("squad1") == 5, "Squad is full");
    assertTrue(!sys.addMember("squad1", "m6"), "Cannot exceed max members");
}

static void testFleetSquadRemoveMember() {
    std::cout << "\n=== Fleet Squad: Remove Member ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "member2");
    assertTrue(sys.removeMember("squad1", "member2"), "Member removed");
    assertTrue(sys.getMemberCount("squad1") == 1, "1 member remaining");
    assertTrue(!sys.removeMember("squad1", "member2"), "Remove missing member fails");
}

static void testFleetSquadRemoveLeader() {
    std::cout << "\n=== Fleet Squad: Remove Leader ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "member2");
    sys.addMember("squad1", "member3");
    assertTrue(sys.removeMember("squad1", "leader1"), "Leader removed");
    assertTrue(sys.getLeaderId("squad1") == "member2", "First remaining member promoted to leader");
    assertTrue(sys.getMemberCount("squad1") == 2, "2 members remaining");
}

static void testFleetSquadFormation() {
    std::cout << "\n=== Fleet Squad: Formation ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.getFormation("squad1") == "line", "Default formation is line");
    sys.setFormation("squad1", components::FleetSquad::SquadFormation::Wedge);
    assertTrue(sys.getFormation("squad1") == "wedge", "Formation changed to wedge");
    sys.setFormation("squad1", components::FleetSquad::SquadFormation::Circle);
    assertTrue(sys.getFormation("squad1") == "circle", "Formation changed to circle");
}

static void testFleetSquadRole() {
    std::cout << "\n=== Fleet Squad: Role ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.getRole("squad1") == "assault", "Default role is assault");
    sys.setRole("squad1", components::FleetSquad::SquadRole::Defense);
    assertTrue(sys.getRole("squad1") == "defense", "Role changed to defense");
    sys.setRole("squad1", components::FleetSquad::SquadRole::Scout);
    assertTrue(sys.getRole("squad1") == "scout", "Role changed to scout");
}

static void testFleetSquadEffectiveness() {
    std::cout << "\n=== Fleet Squad: Effectiveness ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "m2");
    sys.addMember("squad1", "m3");
    sys.update(0.0f);
    // 3 members: 1.0 + 0.05 * (3-1) = 1.10
    assertTrue(approxEqual(sys.getEffectiveness("squad1"), 1.10f), "Effectiveness with 3 members");
    assertTrue(approxEqual(sys.getCohesion("squad1"), 1.0f), "Cohesion is 1.0 for active squad");
    assertTrue(sys.isSquadActive("squad1"), "Squad is active");
}

static void testFleetSquadMissing() {
    std::cout << "\n=== Fleet Squad: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetSquadSystem sys(&world);
    assertTrue(sys.getMemberCount("nonexistent") == 0, "Default member count for missing");
    assertTrue(sys.getLeaderId("nonexistent") == "", "Default leader for missing");
    assertTrue(sys.getRole("nonexistent") == "", "Default role for missing");
    assertTrue(sys.getFormation("nonexistent") == "", "Default formation for missing");
    assertTrue(approxEqual(sys.getCohesion("nonexistent"), 0.0f), "Default cohesion for missing");
    assertTrue(approxEqual(sys.getEffectiveness("nonexistent"), 0.0f), "Default effectiveness for missing");
    assertTrue(!sys.isSquadActive("nonexistent"), "Default inactive for missing");
}


void run_fleet_squad_system_tests() {
    testFleetSquadCreate();
    testFleetSquadDissolve();
    testFleetSquadAddMember();
    testFleetSquadMaxMembers();
    testFleetSquadRemoveMember();
    testFleetSquadRemoveLeader();
    testFleetSquadFormation();
    testFleetSquadRole();
    testFleetSquadEffectiveness();
    testFleetSquadMissing();
}
