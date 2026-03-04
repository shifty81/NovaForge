/**
 * Tests for PCGSnapshotManager:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - TakeSnapshot captures store state, increases SnapshotCount
 *   - Undo of TakeSnapshot removes the snapshot
 *   - RestoreSnapshot reverts the store to a previous state
 *   - Undo of RestoreSnapshot returns the store to pre-restore state
 *   - RemoveSnapshot removes existing snapshots, returns false for unknown
 *   - SnapshotNames returns correct list of all snapshot names
 *   - GetSnapshot returns correct data or nullptr for unknown
 */

#include <cassert>
#include <string>
#include <vector>
#include "../cpp_client/include/editor/pcg_snapshot_manager.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// PCGSnapshotManager tests
// ══════════════════════════════════════════════════════════════════

void test_pcg_snap_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    PCGSnapshotManager tool(bus, store);
    assert(std::string(tool.Name()) == "PCG Snapshot");
}

void test_pcg_snap_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    PCGSnapshotManager tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_pcg_snap_take() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    tool.TakeSnapshot("snap1");
    bus.ProcessCommands();

    assert(tool.SnapshotCount() == 1);
    assert(tool.HasSnapshot("snap1"));
}

void test_pcg_snap_take_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    tool.TakeSnapshot("snap1");
    bus.ProcessCommands();
    assert(tool.SnapshotCount() == 1);

    bus.Undo();
    assert(tool.SnapshotCount() == 0);
    assert(!tool.HasSnapshot("snap1"));
}

void test_pcg_snap_restore() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    // Take a snapshot of the empty store
    tool.TakeSnapshot("baseline");
    bus.ProcessCommands();
    assert(tool.SnapshotCount() == 1);

    // Add an edit to the store after the snapshot
    DeltaEdit edit{};
    edit.type = DeltaEditType::SetProperty;
    edit.entityID = 1;
    edit.propertyName = "gravity";
    edit.propertyValue = "9.81";
    store.Record(edit);
    assert(store.Count() == 1);

    // Restore the baseline snapshot — store should revert
    tool.RestoreSnapshot("baseline");
    bus.ProcessCommands();
    assert(store.Count() == 0);
}

void test_pcg_snap_restore_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    // Take a snapshot of the empty store
    tool.TakeSnapshot("baseline");
    bus.ProcessCommands();

    // Add an edit after the snapshot
    DeltaEdit edit{};
    edit.type = DeltaEditType::SetProperty;
    edit.entityID = 1;
    edit.propertyName = "gravity";
    edit.propertyValue = "9.81";
    store.Record(edit);
    size_t countBeforeRestore = store.Count();

    // Restore, then undo the restore
    tool.RestoreSnapshot("baseline");
    bus.ProcessCommands();
    assert(store.Count() == 0);

    bus.Undo();
    assert(store.Count() == countBeforeRestore);
    assert(store.Edits()[0].propertyValue == "9.81");
}

void test_pcg_snap_remove() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    tool.TakeSnapshot("temp");
    bus.ProcessCommands();
    assert(tool.HasSnapshot("temp"));

    bool removed = tool.RemoveSnapshot("temp");
    assert(removed);
    assert(!tool.HasSnapshot("temp"));
    assert(tool.SnapshotCount() == 0);
}

void test_pcg_snap_remove_nonexistent() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    bool removed = tool.RemoveSnapshot("does_not_exist");
    assert(!removed);
}

void test_pcg_snap_names_list() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    tool.TakeSnapshot("alpha");
    bus.ProcessCommands();
    tool.TakeSnapshot("beta");
    bus.ProcessCommands();
    tool.TakeSnapshot("gamma");
    bus.ProcessCommands();

    auto names = tool.SnapshotNames();
    assert(names.size() == 3);
    assert(names[0] == "alpha");
    assert(names[1] == "beta");
    assert(names[2] == "gamma");
}

void test_pcg_snap_get() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    PCGSnapshotManager tool(bus, store);

    tool.TakeSnapshot("snap1");
    bus.ProcessCommands();

    const PCGSnapshot* snap = tool.GetSnapshot("snap1");
    assert(snap != nullptr);
    assert(snap->name == "snap1");
    assert(!snap->json.empty());

    const PCGSnapshot* missing = tool.GetSnapshot("unknown");
    assert(missing == nullptr);
}
