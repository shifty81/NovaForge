/**
 * Tests for FunctionAssignmentTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - AssignFunction posts command, records DeltaEdit with fn: prefix
 *   - AssignFunction undo restores previous config
 *   - RemoveFunction posts command, records empty value
 *   - RemoveFunction undo restores previous config
 *   - Multiple function assignments on same entity
 *   - Multiple entities with different functions
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/function_assignment_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// FunctionAssignmentTool tests
// ══════════════════════════════════════════════════════════════════

void test_fn_assign_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    FunctionAssignmentTool tool(bus, store);
    assert(std::string(tool.Name()) == "Function Assignment");
}

void test_fn_assign_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    FunctionAssignmentTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_fn_assign_basic() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    FunctionAssignmentTool tool(bus, store);

    tool.Activate();
    tool.AssignFunction(1, "door_open", "speed=2.0");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "fn:door_open");
    assert(edit.propertyValue == "speed=2.0");
}

void test_fn_assign_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    FunctionAssignmentTool tool(bus, store);

    tool.AssignFunction(1, "turret_fire", "rate=5", "rate=3");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "rate=5");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "fn:turret_fire");
    assert(store.Edits()[1].propertyValue == "rate=3");
}

void test_fn_remove_basic() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    FunctionAssignmentTool tool(bus, store);

    tool.RemoveFunction(1, "lights_flicker", "interval=0.5");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "fn:lights_flicker");
    assert(edit.propertyValue == "");
}

void test_fn_remove_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    FunctionAssignmentTool tool(bus, store);

    tool.RemoveFunction(1, "ai_spawn", "count=3");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "fn:ai_spawn");
    assert(store.Edits()[1].propertyValue == "count=3");
}

void test_fn_assign_multiple_on_entity() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    FunctionAssignmentTool tool(bus, store);

    tool.AssignFunction(1, "door_open", "speed=2.0");
    tool.AssignFunction(1, "lights_flicker", "interval=0.5");
    bus.ProcessCommands();
    assert(store.Count() == 2);

    assert(store.Edits()[0].propertyName == "fn:door_open");
    assert(store.Edits()[1].propertyName == "fn:lights_flicker");
}

void test_fn_assign_multiple_entities() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    FunctionAssignmentTool tool(bus, store);

    tool.AssignFunction(1, "door_open", "speed=1.0");
    tool.AssignFunction(2, "turret_fire", "rate=10");
    tool.AssignFunction(3, "ai_spawn", "count=5");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[2].entityID == 3);
}
