// Tests for: Phase 9: Friendship/Grudge Formation Effects Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_formation_system.h"

using namespace atlas;

// ==================== Phase 9: Friendship/Grudge Formation Effects Tests ====================

static void testFormationFriendCloser() {
    std::cout << "\n=== Formation Friend Closer ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Arrow, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Arrow, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", 60.0f);  // Friend (>50)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Friend spacing: 0.7 × 500 = 350m
    assertTrue(approxEqual(std::fabs(ox), 350.0f, 1.0f), "Friend flies closer (x offset ~350)");
    assertTrue(approxEqual(std::fabs(oz), 350.0f, 1.0f), "Friend flies closer (z offset ~350)");
}

static void testFormationGrudgeWider() {
    std::cout << "\n=== Formation Grudge Wider ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Arrow, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Arrow, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", -60.0f);  // Grudge (<-50)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Grudge spacing: 1.5 × 500 = 750m
    assertTrue(approxEqual(std::fabs(ox), 750.0f, 1.0f), "Grudge flies wider (x offset ~750)");
    assertTrue(approxEqual(std::fabs(oz), 750.0f, 1.0f), "Grudge flies wider (z offset ~750)");
}

static void testFormationNeutralDefault() {
    std::cout << "\n=== Formation Neutral Default ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Arrow, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Arrow, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", 5.0f);  // Neutral

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Neutral spacing: 1.0 × 500 = 500m
    assertTrue(approxEqual(std::fabs(ox), 500.0f, 1.0f), "Neutral uses default spacing (x)");
    assertTrue(approxEqual(std::fabs(oz), 500.0f, 1.0f), "Neutral uses default spacing (z)");
}

static void testFormationAllySpacing() {
    std::cout << "\n=== Formation Ally Spacing ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Line, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Line, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", 30.0f);  // Ally (>20)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Ally spacing: 0.85 × 500 = 425m
    assertTrue(approxEqual(std::fabs(oz), 425.0f, 1.0f), "Ally spacing ~425m in line formation");
}

static void testFormationRivalSpacing() {
    std::cout << "\n=== Formation Rival Spacing ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Line, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Line, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", -30.0f);  // Rival (<-20)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Rival spacing: 1.25 × 500 = 625m
    assertTrue(approxEqual(std::fabs(oz), 625.0f, 1.0f), "Rival spacing ~625m in line formation");
}


void run_captain_relationship_tests() {
    testFormationFriendCloser();
    testFormationGrudgeWider();
    testFormationNeutralDefault();
    testFormationAllySpacing();
    testFormationRivalSpacing();
}
