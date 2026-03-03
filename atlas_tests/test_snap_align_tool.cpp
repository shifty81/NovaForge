/**
 * Tests for SnapAlignTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - Grid snap posts command, records DeltaEdit with correct position
 *   - Grid snap undo restores original position
 *   - Surface snap posts command with correct Y
 *   - Surface snap undo restores original Y
 *   - Align entities on axis
 *   - Align entities undo
 *   - Grid size configuration
 */

#include <cassert>
#include <cmath>
#include <string>
#include "../cpp_client/include/editor/snap_align_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// SnapAlignTool tests
// ══════════════════════════════════════════════════════════════════

void test_snap_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SnapAlignTool tool(bus, store);
    assert(std::string(tool.Name()) == "Snap & Align");
}

void test_snap_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SnapAlignTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_snap_tool_grid_size() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SnapAlignTool tool(bus, store, 2.0f);

    assert(tool.GridSize() == 2.0f);
    tool.SetGridSize(0.5f);
    assert(tool.GridSize() == 0.5f);
    tool.SetGridSize(-1.0f); // ignored
    assert(tool.GridSize() == 0.5f);
}

void test_snap_tool_grid_snap() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store, 1.0f);

    tool.Activate();
    tool.SnapToGrid(1, 1.3f, 2.7f, -0.4f);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::MoveObject);
    assert(edit.entityID == 1);
    assert(edit.position[0] == 1.0f);
    assert(edit.position[1] == 3.0f);
    assert(edit.position[2] == 0.0f);
}

void test_snap_tool_grid_snap_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store, 1.0f);

    tool.SnapToGrid(1, 1.3f, 2.7f, -0.4f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    const auto& edit = store.Edits()[1];
    assert(edit.type == DeltaEditType::MoveObject);
    assert(edit.position[0] == 1.3f);
    assert(edit.position[1] == 2.7f);
    assert(edit.position[2] == -0.4f);
}

void test_snap_tool_surface_snap() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store);

    tool.SnapToSurface(1, 5.0f, 10.0f, 3.0f, 0.0f);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::MoveObject);
    assert(edit.entityID == 1);
    assert(edit.position[0] == 5.0f);
    assert(edit.position[1] == 0.0f);
    assert(edit.position[2] == 3.0f);
}

void test_snap_tool_surface_snap_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store);

    tool.SnapToSurface(1, 5.0f, 10.0f, 3.0f, 0.0f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    const auto& edit = store.Edits()[1];
    assert(edit.position[0] == 5.0f);
    assert(edit.position[1] == 10.0f);
    assert(edit.position[2] == 3.0f);
}

void test_snap_tool_align_entities() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store);

    std::vector<AlignEntitiesCommand::EntityPos> entities = {
        {1, 1.0f, 5.0f, 0.0f},
        {2, 3.0f, 8.0f, 0.0f},
        {3, 7.0f, 2.0f, 0.0f},
    };

    tool.AlignOnAxis(entities, 1, 4.0f); // align Y to 4.0
    bus.ProcessCommands();
    assert(store.Count() == 3);

    for (size_t i = 0; i < 3; ++i) {
        assert(store.Edits()[i].type == DeltaEditType::MoveObject);
        assert(store.Edits()[i].position[1] == 4.0f);
    }
    // X unchanged
    assert(store.Edits()[0].position[0] == 1.0f);
    assert(store.Edits()[1].position[0] == 3.0f);
    assert(store.Edits()[2].position[0] == 7.0f);
}

void test_snap_tool_align_entities_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store);

    std::vector<AlignEntitiesCommand::EntityPos> entities = {
        {1, 1.0f, 5.0f, 0.0f},
        {2, 3.0f, 8.0f, 0.0f},
    };

    tool.AlignOnAxis(entities, 1, 4.0f);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    // Undone edits restore original positions
    assert(store.Edits()[2].position[1] == 5.0f);
    assert(store.Edits()[3].position[1] == 8.0f);
}

void test_snap_tool_multiple_operations() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SnapAlignTool tool(bus, store, 2.0f);

    tool.SnapToGrid(1, 1.1f, 2.9f, 3.5f);
    tool.SnapToSurface(2, 5.0f, 10.0f, 3.0f, 0.0f);
    assert(bus.PendingCount() == 2);

    bus.ProcessCommands();
    assert(store.Count() == 2);
    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
}
