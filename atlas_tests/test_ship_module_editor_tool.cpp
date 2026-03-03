/**
 * Tests for ShipModuleEditorTool:
 *   - ITool interface compliance (activate, deactivate, name)
 *   - AttachModule, DetachModule, SetModuleProperty via command bus
 *   - DeltaEditStore integration
 */

#include <cassert>
#include <string>
#include <memory>
#include "../cpp_client/include/editor/ship_module_editor_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// ShipModuleEditorTool tests
// ══════════════════════════════════════════════════════════════════

void test_ship_tool_name() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);
    assert(std::string(tool.Name()) == "Ship Module Editor");
}

void test_ship_tool_activate_deactivate() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);
    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_ship_tool_attach_module() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);

    EntityID shipID = world.CreateEntity();
    tool.AttachModule(shipID, "shield_booster", "mid_slot_1");
    bus.ProcessCommands();

    // Module entity created (ship + module = 2)
    assert(world.EntityCount() == 2);
    // DeltaEdit recorded
    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::AddObject);
    assert(store.Edits()[0].objectType == "shield_booster");
    assert(store.Edits()[0].propertyName == "slot");
    assert(store.Edits()[0].propertyValue == "mid_slot_1");
}

void test_ship_tool_detach_module() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);

    EntityID shipID = world.CreateEntity();
    EntityID modID  = world.CreateEntity();
    assert(world.EntityCount() == 2);

    tool.DetachModule(modID);
    bus.ProcessCommands();

    assert(world.EntityCount() == 1); // only ship remains
    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::RemoveObject);
}

void test_ship_tool_set_module_property() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);

    EntityID modID = world.CreateEntity();
    tool.SetModuleProperty(modID, "damage", "100", "150");
    bus.ProcessCommands();

    assert(store.Count() == 1);
    assert(store.Edits()[0].type == DeltaEditType::SetProperty);
    assert(store.Edits()[0].propertyName == "damage");
    assert(store.Edits()[0].propertyValue == "150");
}

void test_ship_tool_attach_undo() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);

    EntityID shipID = world.CreateEntity();
    tool.AttachModule(shipID, "railgun", "high_slot_1");
    bus.ProcessCommands();
    assert(world.EntityCount() == 2);

    bus.Undo();
    assert(world.EntityCount() == 1); // module removed by undo
}

void test_ship_tool_property_undo() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);

    EntityID modID = world.CreateEntity();
    tool.SetModuleProperty(modID, "range", "50", "100");
    bus.ProcessCommands();

    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "100");

    bus.Undo();
    // Undo records a reverse edit
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "50");
}

void test_ship_tool_multiple_modules() {
    World world;
    UndoableCommandBus bus;
    DeltaEditStore store;
    ShipModuleEditorTool tool(world, bus, store);

    EntityID shipID = world.CreateEntity();
    tool.AttachModule(shipID, "laser", "high_1");
    tool.AttachModule(shipID, "armor_plate", "low_1");
    tool.AttachModule(shipID, "afterburner", "mid_1");
    bus.ProcessCommands();

    assert(world.EntityCount() == 4); // ship + 3 modules
    assert(store.Count() == 3);
}
