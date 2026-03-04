// Tests for: Fleet Formation System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/movement_system.h"
#include "systems/fleet_formation_system.h"

using namespace atlas;

// ==================== Fleet Formation System Tests ====================

static void testFleetFormationSetFormation() {
    std::cout << "\n=== Fleet Formation: Set Formation ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("leader");
    world.createEntity("wing1");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("leader", FT::Arrow, 0);
    sys.setFormation("wing1", FT::Arrow, 1);

    assertTrue(sys.getFormation("leader") == FT::Arrow, "Leader has Arrow formation");
    assertTrue(sys.getFormation("wing1") == FT::Arrow, "Wing1 has Arrow formation");
}

static void testFleetFormationLeaderAtOrigin() {
    std::cout << "\n=== Fleet Formation: Leader At Origin ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("leader");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("leader", FT::Arrow, 0);
    sys.computeOffsets();

    float ox = 0, oy = 0, oz = 0;
    assertTrue(sys.getOffset("leader", ox, oy, oz), "Leader has offset");
    assertTrue(approxEqual(ox, 0.0f) && approxEqual(oy, 0.0f) && approxEqual(oz, 0.0f),
               "Leader offset is (0,0,0)");
}

static void testFleetFormationArrowOffsets() {
    std::cout << "\n=== Fleet Formation: Arrow Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("s0");
    world.createEntity("s1");
    world.createEntity("s2");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("s0", FT::Arrow, 0);
    sys.setFormation("s1", FT::Arrow, 1);
    sys.setFormation("s2", FT::Arrow, 2);
    sys.computeOffsets();

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("s1", ox1, oy1, oz1);
    assertTrue(ox1 < 0.0f, "Slot 1 is to the left");
    assertTrue(oz1 < 0.0f, "Slot 1 is behind");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("s2", ox2, oy2, oz2);
    assertTrue(ox2 > 0.0f, "Slot 2 is to the right");
    assertTrue(oz2 < 0.0f, "Slot 2 is behind");
}

static void testFleetFormationLineOffsets() {
    std::cout << "\n=== Fleet Formation: Line Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("s0");
    world.createEntity("s1");
    world.createEntity("s2");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("s0", FT::Line, 0);
    sys.setFormation("s1", FT::Line, 1);
    sys.setFormation("s2", FT::Line, 2);
    sys.computeOffsets();

    float ox0 = 0, oy0 = 0, oz0 = 0;
    sys.getOffset("s0", ox0, oy0, oz0);
    assertTrue(approxEqual(ox0, 0.0f) && approxEqual(oz0, 0.0f), "Line slot 0 at origin");

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("s1", ox1, oy1, oz1);
    assertTrue(approxEqual(ox1, 0.0f), "Line slot 1 aligned in X");
    assertTrue(oz1 < 0.0f, "Line slot 1 behind leader");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("s2", ox2, oy2, oz2);
    assertTrue(oz2 < oz1, "Line slot 2 further behind than slot 1");
}

static void testFleetFormationDiamondOffsets() {
    std::cout << "\n=== Fleet Formation: Diamond Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    for (int i = 0; i < 4; ++i)
        world.createEntity("d" + std::to_string(i));

    using FT = components::FleetFormation::FormationType;
    for (int i = 0; i < 4; ++i)
        sys.setFormation("d" + std::to_string(i), FT::Diamond, i);
    sys.computeOffsets();

    float ox0 = 0, oy0 = 0, oz0 = 0;
    sys.getOffset("d0", ox0, oy0, oz0);
    assertTrue(approxEqual(ox0, 0.0f) && approxEqual(oz0, 0.0f), "Diamond slot 0 at origin");

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("d1", ox1, oy1, oz1);
    assertTrue(ox1 < 0.0f, "Diamond slot 1 to the left");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("d2", ox2, oy2, oz2);
    assertTrue(ox2 > 0.0f, "Diamond slot 2 to the right");

    float ox3 = 0, oy3 = 0, oz3 = 0;
    sys.getOffset("d3", ox3, oy3, oz3);
    assertTrue(approxEqual(ox3, 0.0f), "Diamond slot 3 centered in X");
    assertTrue(oz3 < oz1, "Diamond slot 3 behind slots 1 & 2");
}

static void testFleetFormationNoneType() {
    std::cout << "\n=== Fleet Formation: None Type ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("e1");

    assertTrue(sys.getFormation("e1") == components::FleetFormation::FormationType::None,
               "Entity without formation returns None");

    float ox = 0, oy = 0, oz = 0;
    assertTrue(!sys.getOffset("e1", ox, oy, oz), "No offset for entity without component");
}


void run_fleet_formation_system_tests() {
    testFleetFormationSetFormation();
    testFleetFormationLeaderAtOrigin();
    testFleetFormationArrowOffsets();
    testFleetFormationLineOffsets();
    testFleetFormationDiamondOffsets();
    testFleetFormationNoneType();
}
