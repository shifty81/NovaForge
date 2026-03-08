// Tests for: SessionPersistenceSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/session_persistence_system.h"

using namespace atlas;

// ==================== SessionPersistenceSystem Tests ====================

static void testSessionDefaultState() {
    std::cout << "\n=== SessionPersistence: DefaultState ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);

    assertTrue(state->status == components::SessionPersistenceState::SaveStatus::Clean,
               "Default status is Clean");
    assertTrue(state->total_saves == 0, "Zero saves initially");
    assertTrue(state->total_loads == 0, "Zero loads initially");
    assertTrue(approxEqual(state->auto_save_interval, 300.0f), "Default auto-save interval 300s");
    assertTrue(state->current_system == "sol", "Default system is sol");
    assertTrue(state->docked_station.empty(), "Not docked by default");
    assertTrue(sys.getTotalSaves("player1") == 0, "getTotalSaves returns 0");
    assertTrue(sys.getTotalLoads("player1") == 0, "getTotalLoads returns 0");
}

static void testSessionMarkDirty() {
    std::cout << "\n=== SessionPersistence: MarkDirty ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::SessionPersistenceState>(e);

    assertTrue(!sys.isDirty("player1"), "Not dirty initially");
    assertTrue(sys.markDirty("player1"), "markDirty succeeds");
    assertTrue(sys.isDirty("player1"), "Now dirty");

    // Missing entity
    assertTrue(!sys.markDirty("missing"), "markDirty on missing fails");
}

static void testSessionTriggerSave() {
    std::cout << "\n=== SessionPersistence: TriggerSave ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);
    state->time_since_last_save = 100.0f;

    assertTrue(sys.triggerSave("player1"), "triggerSave succeeds");
    assertTrue(sys.getTotalSaves("player1") == 1, "1 save after trigger");
    assertTrue(approxEqual(sys.getTimeSinceLastSave("player1"), 0.0f), "Time reset after save");
    assertTrue(!sys.isDirty("player1"), "Clean after save");

    // Multiple saves
    sys.triggerSave("player1");
    assertTrue(sys.getTotalSaves("player1") == 2, "2 saves after second trigger");

    // Missing entity
    assertTrue(!sys.triggerSave("missing"), "triggerSave on missing fails");
}

static void testSessionTriggerLoad() {
    std::cout << "\n=== SessionPersistence: TriggerLoad ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);
    state->time_since_last_save = 50.0f;

    assertTrue(sys.triggerLoad("player1"), "triggerLoad succeeds");
    assertTrue(sys.getTotalLoads("player1") == 1, "1 load after trigger");
    assertTrue(approxEqual(sys.getTimeSinceLastSave("player1"), 0.0f), "Time reset after load");

    // Missing entity
    assertTrue(!sys.triggerLoad("missing"), "triggerLoad on missing fails");
}

static void testSessionAutoSave() {
    std::cout << "\n=== SessionPersistence: AutoSave ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);
    state->auto_save_interval = 10.0f;

    // Mark dirty first
    sys.markDirty("player1");

    // Not yet at interval
    sys.update(5.0f);
    assertTrue(sys.getTotalSaves("player1") == 0, "No save before interval");
    assertTrue(sys.isDirty("player1"), "Still dirty before interval");

    // Pass interval
    sys.update(6.0f);
    assertTrue(sys.getTotalSaves("player1") == 1, "Auto-save triggered at interval");
    assertTrue(!sys.isDirty("player1"), "Clean after auto-save");
}

static void testSessionAutoSaveCleanSkips() {
    std::cout << "\n=== SessionPersistence: AutoSaveCleanSkips ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);
    state->auto_save_interval = 5.0f;

    // Don't mark dirty — auto-save should not trigger
    sys.update(10.0f);
    assertTrue(sys.getTotalSaves("player1") == 0, "No auto-save when clean");
}

static void testSessionPositionSnapshot() {
    std::cout << "\n=== SessionPersistence: PositionSnapshot ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);

    state->position_x = 100.0f;
    state->position_y = -50.0f;
    state->position_z = 200.0f;
    state->credits = 50000.0;
    state->cargo_units = 42;
    state->current_system = "alpha_centauri";
    state->docked_station = "station_alpha";

    assertTrue(approxEqual(state->position_x, 100.0f), "Position X stored");
    assertTrue(approxEqual(state->position_y, -50.0f), "Position Y stored");
    assertTrue(approxEqual(state->position_z, 200.0f), "Position Z stored");
    assertTrue(state->credits == 50000.0, "Credits stored");
    assertTrue(state->cargo_units == 42, "Cargo stored");
    assertTrue(sys.getCurrentSystem("player1") == "alpha_centauri", "System stored");
    assertTrue(state->docked_station == "station_alpha", "Docked station stored");
}

static void testSessionShipState() {
    std::cout << "\n=== SessionPersistence: ShipState ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);

    assertTrue(state->ship_hull_hp == 100, "Default hull 100");
    assertTrue(state->ship_armor_hp == 100, "Default armor 100");
    assertTrue(state->ship_shield_hp == 100, "Default shield 100");

    state->ship_hull_hp = 50;
    state->ship_armor_hp = 30;
    state->ship_shield_hp = 0;

    assertTrue(state->ship_hull_hp == 50, "Hull updated");
    assertTrue(state->ship_armor_hp == 30, "Armor updated");
    assertTrue(state->ship_shield_hp == 0, "Shield zeroed");
}

static void testSessionMissingEntity() {
    std::cout << "\n=== SessionPersistence: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);

    assertTrue(!sys.markDirty("missing"), "markDirty on missing");
    assertTrue(!sys.triggerSave("missing"), "triggerSave on missing");
    assertTrue(!sys.triggerLoad("missing"), "triggerLoad on missing");
    assertTrue(!sys.isDirty("missing"), "isDirty on missing");
    assertTrue(sys.getTotalSaves("missing") == 0, "getTotalSaves on missing");
    assertTrue(sys.getTotalLoads("missing") == 0, "getTotalLoads on missing");
    assertTrue(approxEqual(sys.getTimeSinceLastSave("missing"), 0.0f), "getTimeSinceLastSave on missing");
    assertTrue(sys.getCurrentSystem("missing").empty(), "getCurrentSystem on missing");
}

static void testSessionSaveFailureTracking() {
    std::cout << "\n=== SessionPersistence: SaveFailureTracking ===" << std::endl;
    ecs::World world;
    systems::SessionPersistenceSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::SessionPersistenceState>(e);

    assertTrue(state->save_failures == 0, "Zero failures initially");
    state->save_failures = 3;
    assertTrue(state->save_failures == 3, "Failures tracked");
}

void run_session_persistence_system_tests() {
    testSessionDefaultState();
    testSessionMarkDirty();
    testSessionTriggerSave();
    testSessionTriggerLoad();
    testSessionAutoSave();
    testSessionAutoSaveCleanSkips();
    testSessionPositionSnapshot();
    testSessionShipState();
    testSessionMissingEntity();
    testSessionSaveFailureTracking();
}
