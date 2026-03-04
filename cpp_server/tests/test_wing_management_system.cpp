// Tests for: WingManagementSystem Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/wing_management_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== WingManagementSystem Tests ====================

static void testWingManagementCreateWing() {
    std::cout << "\n=== Wing Management: Create Wing ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    assertTrue(sys.createWing("fleet1", "wing_alpha", "Combat"), "Create combat wing succeeds");
    assertTrue(sys.getWingCount("fleet1") == 1, "Fleet has 1 wing");
    assertTrue(sys.getWingRole("fleet1", "wing_alpha") == "Combat", "Wing role is Combat");
}

static void testWingManagementDuplicateWing() {
    std::cout << "\n=== Wing Management: Duplicate Wing ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    assertTrue(!sys.createWing("fleet1", "wing_alpha", "Mining"), "Duplicate wing creation fails");
    assertTrue(sys.getWingCount("fleet1") == 1, "Still 1 wing");
}

static void testWingManagementDissolveWing() {
    std::cout << "\n=== Wing Management: Dissolve Wing ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    assertTrue(sys.dissolveWing("fleet1", "wing_alpha"), "Dissolve wing succeeds");
    assertTrue(sys.getWingCount("fleet1") == 0, "Fleet has 0 wings");
}

static void testWingManagementAssignMember() {
    std::cout << "\n=== Wing Management: Assign Member ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Mining");
    assertTrue(sys.assignToWing("fleet1", "wing_alpha", "pilot1"), "Assign pilot1 succeeds");
    assertTrue(sys.assignToWing("fleet1", "wing_alpha", "pilot2"), "Assign pilot2 succeeds");

    auto members = sys.getWingMembers("fleet1", "wing_alpha");
    assertTrue(static_cast<int>(members.size()) == 2, "Wing has 2 members");
}

static void testWingManagementMaxMembers() {
    std::cout << "\n=== Wing Management: Max Members ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    for (int i = 0; i < 5; i++) {
        sys.assignToWing("fleet1", "wing_alpha", "pilot" + std::to_string(i));
    }
    assertTrue(!sys.assignToWing("fleet1", "wing_alpha", "pilot5"), "6th member rejected");
    assertTrue(static_cast<int>(sys.getWingMembers("fleet1", "wing_alpha").size()) == 5, "Wing capped at 5");
}

static void testWingManagementRemoveMember() {
    std::cout << "\n=== Wing Management: Remove Member ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    sys.assignToWing("fleet1", "wing_alpha", "pilot1");
    sys.assignToWing("fleet1", "wing_alpha", "pilot2");
    assertTrue(sys.removeFromWing("fleet1", "wing_alpha", "pilot1"), "Remove pilot1 succeeds");
    assertTrue(static_cast<int>(sys.getWingMembers("fleet1", "wing_alpha").size()) == 1, "Wing has 1 member");
}

static void testWingManagementSetCommander() {
    std::cout << "\n=== Wing Management: Set Commander ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Logistics");
    sys.assignToWing("fleet1", "wing_alpha", "pilot1");
    assertTrue(sys.setWingCommander("fleet1", "wing_alpha", "pilot1"), "Set commander succeeds");
    assertTrue(sys.getWingCommander("fleet1", "wing_alpha") == "pilot1", "Commander is pilot1");
}

static void testWingManagementMorale() {
    std::cout << "\n=== Wing Management: Wing Morale ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    auto* p1 = world.createEntity("pilot1");
    auto* m1 = addComp<components::FleetMorale>(p1);
    m1->morale_score = 80.0f;

    auto* p2 = world.createEntity("pilot2");
    auto* m2 = addComp<components::FleetMorale>(p2);
    m2->morale_score = 40.0f;

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    sys.assignToWing("fleet1", "wing_alpha", "pilot1");
    sys.assignToWing("fleet1", "wing_alpha", "pilot2");
    sys.update(1.0f);

    // morale_score is -100..+100, mapped to 0..100: (score+100)/2
    // pilot1: (80+100)/2=90, pilot2: (40+100)/2=70, avg=80
    float morale = sys.getWingMorale("fleet1", "wing_alpha");
    assertTrue(approxEqual(morale, 80.0f), "Wing morale is average of mapped members (80)");
}

static void testWingManagementInvalidFleet() {
    std::cout << "\n=== Wing Management: Invalid Fleet ===" << std::endl;
    ecs::World world;
    systems::WingManagementSystem sys(&world);
    assertTrue(!sys.createWing("nonexistent", "w1", "Combat"), "Create wing on missing fleet fails");
    assertTrue(sys.getWingCount("nonexistent") == 0, "Missing fleet has 0 wings");
    assertTrue(sys.getWingMorale("nonexistent", "w1") == 50.0f, "Default morale for missing");
}

static void testWingManagementInvalidRole() {
    std::cout << "\n=== Wing Management: Invalid Role ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    assertTrue(!sys.createWing("fleet1", "w1", "InvalidRole"), "Invalid role rejected");
    assertTrue(sys.getWingCount("fleet1") == 0, "No wings created");
}


void run_wing_management_system_tests() {
    testWingManagementCreateWing();
    testWingManagementDuplicateWing();
    testWingManagementDissolveWing();
    testWingManagementAssignMember();
    testWingManagementMaxMembers();
    testWingManagementRemoveMember();
    testWingManagementSetCommander();
    testWingManagementMorale();
    testWingManagementInvalidFleet();
    testWingManagementInvalidRole();
}
