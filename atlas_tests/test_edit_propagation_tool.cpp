/**
 * Tests for EditPropagationTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - PropagateProperty posts command, records DeltaEdits for all targets
 *   - PropagateProperty undo restores old values
 *   - PropagatePosition posts command, records MoveObject edits
 *   - PropagatePosition undo restores old positions
 *   - Multiple propagation operations in sequence
 *   - Single-entity propagation (edge case)
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/edit_propagation_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// EditPropagationTool tests
// ══════════════════════════════════════════════════════════════════

void test_edit_prop_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EditPropagationTool tool(bus, store);
    assert(std::string(tool.Name()) == "Edit Propagation");
}

void test_edit_prop_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EditPropagationTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_edit_prop_propagate_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EditPropagationTool tool(bus, store);

    tool.Activate();
    std::vector<std::pair<uint32_t, std::string>> targets = {
        {1, "red"}, {2, "blue"}, {3, "green"}
    };
    tool.PropagateProperty(targets, "color", "white");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[0].propertyName == "color");
    assert(store.Edits()[0].propertyValue == "white");

    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[1].propertyValue == "white");

    assert(store.Edits()[2].entityID == 3);
    assert(store.Edits()[2].propertyValue == "white");
}

void test_edit_prop_propagate_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EditPropagationTool tool(bus, store);

    std::vector<std::pair<uint32_t, std::string>> targets = {
        {1, "100"}, {2, "200"}
    };
    tool.PropagateProperty(targets, "shield", "999");
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].entityID == 1);
    assert(store.Edits()[2].propertyValue == "100");
    assert(store.Edits()[3].entityID == 2);
    assert(store.Edits()[3].propertyValue == "200");
}

void test_edit_prop_propagate_position() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EditPropagationTool tool(bus, store);

    std::vector<PropagatePositionCommand::EntityPosition> targets = {
        {1, 0.0f, 0.0f, 0.0f},
        {2, 5.0f, 5.0f, 5.0f}
    };
    tool.PropagatePosition(targets, 10.0f, 20.0f, 30.0f);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 2);

    assert(store.Edits()[0].type == DeltaEditType::MoveObject);
    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[0].position[0] == 10.0f);
    assert(store.Edits()[0].position[1] == 20.0f);
    assert(store.Edits()[0].position[2] == 30.0f);

    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[1].position[0] == 10.0f);
}

void test_edit_prop_propagate_position_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EditPropagationTool tool(bus, store);

    std::vector<PropagatePositionCommand::EntityPosition> targets = {
        {1, 0.0f, 0.0f, 0.0f},
        {2, 5.0f, 5.0f, 5.0f}
    };
    tool.PropagatePosition(targets, 10.0f, 20.0f, 30.0f);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].entityID == 1);
    assert(store.Edits()[2].position[0] == 0.0f);
    assert(store.Edits()[2].position[1] == 0.0f);
    assert(store.Edits()[2].position[2] == 0.0f);

    assert(store.Edits()[3].entityID == 2);
    assert(store.Edits()[3].position[0] == 5.0f);
    assert(store.Edits()[3].position[1] == 5.0f);
    assert(store.Edits()[3].position[2] == 5.0f);
}

void test_edit_prop_multiple_operations() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EditPropagationTool tool(bus, store);

    std::vector<std::pair<uint32_t, std::string>> targets1 = {
        {1, "old1"}, {2, "old2"}
    };
    std::vector<std::pair<uint32_t, std::string>> targets2 = {
        {3, "old3"}, {4, "old4"}
    };
    tool.PropagateProperty(targets1, "faction", "Solari");
    tool.PropagateProperty(targets2, "faction", "Veyren");
    assert(bus.PendingCount() == 2);

    bus.ProcessCommands();
    assert(store.Count() == 4);
    assert(store.Edits()[0].propertyValue == "Solari");
    assert(store.Edits()[1].propertyValue == "Solari");
    assert(store.Edits()[2].propertyValue == "Veyren");
    assert(store.Edits()[3].propertyValue == "Veyren");
}

void test_edit_prop_single_entity() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EditPropagationTool tool(bus, store);

    std::vector<std::pair<uint32_t, std::string>> targets = {
        {1, "old"}
    };
    tool.PropagateProperty(targets, "material", "gold");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "gold");
}
