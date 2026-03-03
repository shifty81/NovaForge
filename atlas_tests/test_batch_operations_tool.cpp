/**
 * Tests for BatchOperationsTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetPropertyOnMany posts command, records DeltaEdits for all targets
 *   - TransformMany posts command, records DeltaEdits for all targets
 *   - Undo reverses batch property changes and transforms
 *   - Multiple batch operations in sequence
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/batch_operations_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// BatchOperationsTool tests
// ══════════════════════════════════════════════════════════════════

void test_batch_ops_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    BatchOperationsTool tool(bus, store);
    assert(std::string(tool.Name()) == "Batch Operations");
}

void test_batch_ops_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    BatchOperationsTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_batch_ops_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    BatchOperationsTool tool(bus, store);

    tool.Activate();
    std::vector<std::pair<uint32_t, std::string>> targets = {
        {1, "red"}, {2, "blue"}, {3, "green"}
    };
    tool.SetPropertyOnMany(targets, "color", "white");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    // Three entities updated
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[0].propertyName == "color");
    assert(store.Edits()[0].propertyValue == "white");

    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[1].propertyValue == "white");

    assert(store.Edits()[2].entityID == 3);
    assert(store.Edits()[2].propertyValue == "white");
}

void test_batch_ops_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    BatchOperationsTool tool(bus, store);

    std::vector<std::pair<uint32_t, std::string>> targets = {
        {1, "100"}, {2, "200"}
    };
    tool.SetPropertyOnMany(targets, "health", "999");
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].entityID == 1);
    assert(store.Edits()[2].propertyValue == "100");
    assert(store.Edits()[3].entityID == 2);
    assert(store.Edits()[3].propertyValue == "200");
}

void test_batch_ops_transform() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    BatchOperationsTool tool(bus, store);

    std::vector<BatchTransformCommand::EntityTransform> transforms = {
        {1, 0.0f, 0.0f, 0.0f, 10.0f, 20.0f, 30.0f},
        {2, 5.0f, 5.0f, 5.0f, 15.0f, 25.0f, 35.0f}
    };
    tool.TransformMany(transforms);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 2);

    assert(store.Edits()[0].type == DeltaEditType::MoveObject);
    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[0].position[0] == 10.0f);
    assert(store.Edits()[0].position[1] == 20.0f);
    assert(store.Edits()[0].position[2] == 30.0f);

    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[1].position[0] == 15.0f);
}

void test_batch_ops_transform_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    BatchOperationsTool tool(bus, store);

    std::vector<BatchTransformCommand::EntityTransform> transforms = {
        {1, 0.0f, 0.0f, 0.0f, 10.0f, 20.0f, 30.0f},
        {2, 5.0f, 5.0f, 5.0f, 15.0f, 25.0f, 35.0f}
    };
    tool.TransformMany(transforms);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    // Undo should restore original positions
    assert(store.Edits()[2].type == DeltaEditType::MoveObject);
    assert(store.Edits()[2].entityID == 1);
    assert(store.Edits()[2].position[0] == 0.0f);
    assert(store.Edits()[2].position[1] == 0.0f);
    assert(store.Edits()[2].position[2] == 0.0f);

    assert(store.Edits()[3].entityID == 2);
    assert(store.Edits()[3].position[0] == 5.0f);
    assert(store.Edits()[3].position[1] == 5.0f);
    assert(store.Edits()[3].position[2] == 5.0f);
}

void test_batch_ops_multiple_operations() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    BatchOperationsTool tool(bus, store);

    std::vector<std::pair<uint32_t, std::string>> targets1 = {
        {1, "old1"}, {2, "old2"}
    };
    std::vector<std::pair<uint32_t, std::string>> targets2 = {
        {3, "old3"}, {4, "old4"}
    };
    tool.SetPropertyOnMany(targets1, "faction", "Solari");
    tool.SetPropertyOnMany(targets2, "faction", "Veyren");
    assert(bus.PendingCount() == 2);

    bus.ProcessCommands();
    assert(store.Count() == 4);
    assert(store.Edits()[0].propertyValue == "Solari");
    assert(store.Edits()[1].propertyValue == "Solari");
    assert(store.Edits()[2].propertyValue == "Veyren");
    assert(store.Edits()[3].propertyValue == "Veyren");
}
