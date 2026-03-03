/**
 * Tests for NPCSpawnerTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SpawnNPC posts command, creates entity and records DeltaEdit
 *   - DespawnNPC posts command, destroys entity and records DeltaEdit
 *   - SetNPCProperty posts command, records DeltaEdit
 *   - Undo reverses spawn and property changes
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/npc_spawner_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// NPCSpawnerTool tests
// ══════════════════════════════════════════════════════════════════

void test_npc_spawner_name() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    NPCSpawnerTool tool(world, bus, store);
    assert(std::string(tool.Name()) == "NPC Spawner");
}

void test_npc_spawner_activate_deactivate() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    NPCSpawnerTool tool(world, bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_npc_spawner_spawn() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    NPCSpawnerTool tool(world, bus, store);

    tool.Activate();
    tool.SpawnNPC("pirate_captain", "Keldari", 10.0f, 20.0f, 30.0f);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::AddObject);
    assert(edit.objectType == "npc:pirate_captain");
    assert(edit.position[0] == 10.0f);
    assert(edit.position[1] == 20.0f);
    assert(edit.position[2] == 30.0f);
    assert(edit.propertyName == "faction");
    assert(edit.propertyValue == "Keldari");
}

void test_npc_spawner_spawn_undo() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    NPCSpawnerTool tool(world, bus, store);

    tool.SpawnNPC("trader", "Solari", 1.0f, 2.0f, 3.0f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    // The entity created should be alive
    EntityID eid = store.Edits()[0].entityID;
    assert(world.IsAlive(eid));

    // Undo should destroy the entity
    bus.Undo();
    assert(!world.IsAlive(eid));
}

void test_npc_spawner_despawn() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    NPCSpawnerTool tool(world, bus, store);

    // Create an entity first
    EntityID eid = world.CreateEntity();
    assert(world.IsAlive(eid));

    tool.DespawnNPC(eid);
    bus.ProcessCommands();

    assert(!world.IsAlive(eid));
    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::RemoveObject);
    assert(store.Edits()[0].entityID == eid);
}

void test_npc_spawner_despawn_dead_entity() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    NPCSpawnerTool tool(world, bus, store);

    // Despawn a non-existent entity — no edit should be recorded
    tool.DespawnNPC(9999);
    bus.ProcessCommands();
    assert(store.Count() == 0);
}

void test_npc_spawner_set_property() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    NPCSpawnerTool tool(world, bus, store);

    tool.SetNPCProperty(1, "aggression", "0.5", "0.9");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "aggression");
    assert(edit.propertyValue == "0.9");
}

void test_npc_spawner_set_property_undo() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    NPCSpawnerTool tool(world, bus, store);

    tool.SetNPCProperty(1, "aggression", "0.5", "0.9");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "0.9");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "0.5");
}

void test_npc_spawner_multiple_spawns() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    NPCSpawnerTool tool(world, bus, store);

    tool.SpawnNPC("miner", "Veyren", 0.0f, 0.0f, 0.0f);
    tool.SpawnNPC("security", "Aurelian", 10.0f, 0.0f, 0.0f);
    tool.SpawnNPC("hauler", "Solari", 20.0f, 0.0f, 0.0f);
    assert(bus.PendingCount() == 3);

    bus.ProcessCommands();
    assert(store.Count() == 3);
    assert(store.Edits()[0].objectType == "npc:miner");
    assert(store.Edits()[1].objectType == "npc:security");
    assert(store.Edits()[2].objectType == "npc:hauler");
}
