/**
 * Tests for MapEditorTool:
 *   - ITool interface compliance (activate, deactivate, name)
 *   - AddObject, MoveObject, RemoveObject via command bus
 *   - DeltaEditStore integration
 */

#include <cassert>
#include <string>
#include <memory>
#include "../cpp_client/include/editor/map_editor_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// MapEditorTool tests
// ══════════════════════════════════════════════════════════════════

void test_map_tool_name() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);
    assert(std::string(tool.Name()) == "Map Editor");
}

void test_map_tool_activate_deactivate() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);
    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_map_tool_add_object() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(100);
    MapEditorTool tool(world, bus, store);

    tool.Activate();
    tool.AddObject("planet", 10.0f, 20.0f, 30.0f);
    assert(bus.PendingCount() == 1);
    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);

    // Entity was created in the world
    assert(world.EntityCount() == 1);

    // DeltaEdit was recorded
    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::AddObject);
    assert(store.Edits()[0].objectType == "planet");
}

void test_map_tool_move_object() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);

    EntityID eid = world.CreateEntity();
    tool.MoveObject(eid, 0, 0, 0, 50, 60, 70);
    bus.ProcessCommands();

    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::MoveObject);
    assert(store.Edits()[0].entityID == eid);
    float dx = store.Edits()[0].position[0] - 50.0f;
    assert(dx > -0.01f && dx < 0.01f);
}

void test_map_tool_remove_object() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);

    EntityID eid = world.CreateEntity();
    assert(world.EntityCount() == 1);
    tool.RemoveObject(eid);
    bus.ProcessCommands();

    assert(world.EntityCount() == 0);
    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::RemoveObject);
}

void test_map_tool_add_undo() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);

    tool.AddObject("station", 1, 2, 3);
    bus.ProcessCommands();
    assert(world.EntityCount() == 1);

    bus.Undo();
    assert(world.EntityCount() == 0);
}

void test_map_tool_remove_noop_dead_entity() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);

    // Removing a non-existent entity should be a no-op
    tool.RemoveObject(999);
    bus.ProcessCommands();
    assert(store.Count() == 0); // no edit recorded for dead entity
}

void test_map_tool_multiple_adds() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    MapEditorTool tool(world, bus, store);

    tool.AddObject("planet", 0, 0, 0);
    tool.AddObject("station", 10, 0, 0);
    tool.AddObject("asteroid", 20, 0, 0);
    bus.ProcessCommands();

    assert(world.EntityCount() == 3);
    assert(store.Count() == 3);
}
