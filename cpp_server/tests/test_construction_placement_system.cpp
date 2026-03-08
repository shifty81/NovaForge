// Tests for: ConstructionPlacementSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/construction_placement_system.h"

using namespace atlas;
using BC = components::ConstructionPlacementState::BuildContext;

// ==================== ConstructionPlacementSystem Tests ====================

static void testConstructDefaultState() {
    std::cout << "\n=== ConstructionPlacement: DefaultState ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("builder1");
    auto* state = addComp<components::ConstructionPlacementState>(e);

    assertTrue(state->context == BC::ShipInterior, "Default context ShipInterior");
    assertTrue(approxEqual(state->grid_size, 1.0f), "Default grid 1.0");
    assertTrue(state->snap_to_grid, "Default snap true");
    assertTrue(state->max_sockets == 20, "Default max sockets 20");
    assertTrue(state->occupied_sockets == 0, "Default occupied 0");
    assertTrue(state->total_placements == 0, "Default placements 0");
    assertTrue(state->total_removals == 0, "Default removals 0");
    assertTrue(approxEqual(state->placement_x, 0.0f), "Default x 0");
    assertTrue(approxEqual(state->placement_y, 0.0f), "Default y 0");
    assertTrue(approxEqual(state->placement_z, 0.0f), "Default z 0");
    assertTrue(approxEqual(state->placement_rotation, 0.0f), "Default rotation 0");
    assertTrue(state->selected_module_id.empty(), "Default no module");
    assertTrue(!state->placement_valid, "Default not valid");
    assertTrue(state->active, "Default active");
}

static void testConstructPlaceModuleSnap() {
    std::cout << "\n=== ConstructionPlacement: PlaceModuleSnap ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("b1");
    auto* state = addComp<components::ConstructionPlacementState>(e);

    assertTrue(sys.placeModule("b1", "reactor_mk1", 1.3f, 2.7f, 0.1f), "Place OK");
    assertTrue(approxEqual(state->placement_x, 1.0f), "X snapped to 1.0");
    assertTrue(approxEqual(state->placement_y, 3.0f), "Y snapped to 3.0");
    assertTrue(approxEqual(state->placement_z, 0.0f), "Z snapped to 0.0");
    assertTrue(state->occupied_sockets == 1, "1 occupied");
    assertTrue(state->total_placements == 1, "1 placement");
    assertTrue(state->placement_valid, "Placement valid");
    assertTrue(state->selected_module_id == "reactor_mk1", "Module stored");
}

static void testConstructCannotPlaceWhenFull() {
    std::cout << "\n=== ConstructionPlacement: CannotPlaceWhenFull ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("b1");
    auto* state = addComp<components::ConstructionPlacementState>(e);
    state->max_sockets = 2;

    assertTrue(sys.placeModule("b1", "m1", 1.0f, 0.0f, 0.0f), "Place 1 OK");
    assertTrue(sys.placeModule("b1", "m2", 2.0f, 0.0f, 0.0f), "Place 2 OK");
    assertTrue(!sys.placeModule("b1", "m3", 3.0f, 0.0f, 0.0f), "Place 3 rejected");
    assertTrue(state->occupied_sockets == 2, "Still 2 occupied");
    assertTrue(sys.getAvailableSockets("b1") == 0, "0 available");
}

static void testConstructRemoveModule() {
    std::cout << "\n=== ConstructionPlacement: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("b1");
    auto* state = addComp<components::ConstructionPlacementState>(e);

    sys.placeModule("b1", "m1", 1.0f, 0.0f, 0.0f);
    sys.placeModule("b1", "m2", 2.0f, 0.0f, 0.0f);
    assertTrue(state->occupied_sockets == 2, "2 occupied");

    assertTrue(sys.removeModule("b1", 0), "Remove OK");
    assertTrue(state->occupied_sockets == 1, "1 occupied after remove");
    assertTrue(state->total_removals == 1, "1 removal");

    assertTrue(!sys.removeModule("b1", 5), "Remove invalid index");
    assertTrue(!sys.removeModule("b1", -1), "Remove negative index");
}

static void testConstructGridAligned() {
    std::cout << "\n=== ConstructionPlacement: GridAligned ===" << std::endl;

    assertTrue(systems::ConstructionPlacementSystem::isGridAligned(1.0f, 2.0f, 3.0f, 1.0f),
               "1,2,3 aligned to grid 1.0");
    assertTrue(systems::ConstructionPlacementSystem::isGridAligned(0.5f, 1.0f, 1.5f, 0.5f),
               "0.5,1.0,1.5 aligned to grid 0.5");
    assertTrue(!systems::ConstructionPlacementSystem::isGridAligned(0.3f, 0.0f, 0.0f, 1.0f),
               "0.3 not aligned to grid 1.0");
    assertTrue(!systems::ConstructionPlacementSystem::isGridAligned(0.0f, 0.7f, 0.0f, 1.0f),
               "0.7 not aligned to grid 1.0");
    assertTrue(systems::ConstructionPlacementSystem::isGridAligned(0.0f, 0.0f, 0.0f, 1.0f),
               "Origin aligned");
}

static void testConstructSnapToGrid() {
    std::cout << "\n=== ConstructionPlacement: SnapToGrid ===" << std::endl;

    assertTrue(approxEqual(systems::ConstructionPlacementSystem::snapToGrid(1.3f, 1.0f), 1.0f),
               "1.3 snaps to 1.0");
    assertTrue(approxEqual(systems::ConstructionPlacementSystem::snapToGrid(1.7f, 1.0f), 2.0f),
               "1.7 snaps to 2.0");
    assertTrue(approxEqual(systems::ConstructionPlacementSystem::snapToGrid(0.25f, 0.5f), 0.5f),
               "0.25 snaps to 0.5 (grid 0.5)");
    assertTrue(approxEqual(systems::ConstructionPlacementSystem::snapToGrid(-0.3f, 1.0f), 0.0f),
               "-0.3 snaps to 0.0");
    assertTrue(approxEqual(systems::ConstructionPlacementSystem::snapToGrid(-1.7f, 1.0f), -2.0f),
               "-1.7 snaps to -2.0");
    assertTrue(approxEqual(systems::ConstructionPlacementSystem::snapToGrid(5.0f, 1.0f), 5.0f),
               "5.0 stays 5.0");
}

static void testConstructSetContext() {
    std::cout << "\n=== ConstructionPlacement: SetContext ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("b1");
    auto* state = addComp<components::ConstructionPlacementState>(e);

    assertTrue(sys.setContext("b1", BC::ShipExterior), "Set ShipExterior");
    assertTrue(state->context == BC::ShipExterior, "Context is ShipExterior");
    assertTrue(sys.setContext("b1", BC::StationModule), "Set StationModule");
    assertTrue(state->context == BC::StationModule, "Context is StationModule");
    assertTrue(sys.setContext("b1", BC::RoverBay), "Set RoverBay");
    assertTrue(state->context == BC::RoverBay, "Context is RoverBay");
    assertTrue(sys.setContext("b1", BC::RigLocker), "Set RigLocker");
    assertTrue(state->context == BC::RigLocker, "Context is RigLocker");
}

static void testConstructSelectModule() {
    std::cout << "\n=== ConstructionPlacement: SelectModule ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("b1");
    auto* state = addComp<components::ConstructionPlacementState>(e);

    assertTrue(sys.selectModule("b1", "turret_mk2"), "Select OK");
    assertTrue(state->selected_module_id == "turret_mk2", "Module selected");
}

static void testConstructValidatePlacement() {
    std::cout << "\n=== ConstructionPlacement: ValidatePlacement ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);
    auto* e = world.createEntity("b1");
    auto* state = addComp<components::ConstructionPlacementState>(e);

    assertTrue(sys.validatePlacement("b1", 1.0f, 2.0f, 3.0f), "Grid-aligned valid");
    assertTrue(!sys.validatePlacement("b1", 0.3f, 0.0f, 0.0f), "Non-aligned invalid");

    // Fill sockets
    state->occupied_sockets = state->max_sockets;
    assertTrue(!sys.validatePlacement("b1", 1.0f, 0.0f, 0.0f), "Full sockets invalid");
}

static void testConstructMissingEntity() {
    std::cout << "\n=== ConstructionPlacement: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::ConstructionPlacementSystem sys(&world);

    assertTrue(!sys.placeModule("x", "m1", 0.0f, 0.0f, 0.0f), "Place on missing");
    assertTrue(!sys.removeModule("x", 0), "Remove on missing");
    assertTrue(!sys.setContext("x", BC::ShipExterior), "SetContext on missing");
    assertTrue(!sys.selectModule("x", "m1"), "SelectModule on missing");
    assertTrue(!sys.validatePlacement("x", 0.0f, 0.0f, 0.0f), "Validate on missing");
    assertTrue(sys.getOccupiedSockets("x") == 0, "Occupied on missing");
    assertTrue(sys.getAvailableSockets("x") == 0, "Available on missing");
}

void run_construction_placement_system_tests() {
    testConstructDefaultState();
    testConstructPlaceModuleSnap();
    testConstructCannotPlaceWhenFull();
    testConstructRemoveModule();
    testConstructGridAligned();
    testConstructSnapToGrid();
    testConstructSetContext();
    testConstructSelectModule();
    testConstructValidatePlacement();
    testConstructMissingEntity();
}
