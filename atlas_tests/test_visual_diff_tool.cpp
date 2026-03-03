/**
 * Tests for VisualDiffTool:
 *   - Empty store produces empty diff
 *   - AddObject counted correctly
 *   - RemoveObject counted correctly
 *   - MoveObject counted correctly
 *   - SetProperty counted correctly
 *   - Mixed edits counted correctly
 *   - TotalChanges aggregation
 *   - AffectedEntities list
 *   - EntriesForEntity filtering
 *   - EntriesByType filtering
 *   - Description strings non-empty
 */

#include <cassert>
#include <string>
#include <algorithm>
#include "../cpp_client/include/editor/visual_diff_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// VisualDiffTool tests
// ══════════════════════════════════════════════════════════════════

void test_vdiff_empty_store() {
    DeltaEditStore store;
    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.entries.empty());
    assert(summary.TotalChanges() == 0);
    assert(summary.addedCount == 0);
    assert(summary.removedCount == 0);
    assert(summary.movedCount == 0);
    assert(summary.propertyCount == 0);
}

void test_vdiff_add_object() {
    DeltaEditStore store(42);
    DeltaEdit edit{};
    edit.type       = DeltaEditType::AddObject;
    edit.entityID   = 1;
    edit.objectType = "station";
    edit.position[0] = 10.0f;
    store.Record(edit);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.addedCount == 1);
    assert(summary.TotalChanges() == 1);
    assert(summary.entries.size() == 1);
    assert(summary.entries[0].type == DeltaEditType::AddObject);
    assert(summary.entries[0].entityID == 1);
}

void test_vdiff_remove_object() {
    DeltaEditStore store(42);
    DeltaEdit edit{};
    edit.type     = DeltaEditType::RemoveObject;
    edit.entityID = 5;
    store.Record(edit);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.removedCount == 1);
    assert(summary.TotalChanges() == 1);
}

void test_vdiff_move_object() {
    DeltaEditStore store(42);
    DeltaEdit edit{};
    edit.type        = DeltaEditType::MoveObject;
    edit.entityID    = 3;
    edit.position[0] = 1.0f;
    edit.position[1] = 2.0f;
    edit.position[2] = 3.0f;
    store.Record(edit);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.movedCount == 1);
    assert(summary.TotalChanges() == 1);
}

void test_vdiff_set_property() {
    DeltaEditStore store(42);
    DeltaEdit edit{};
    edit.type          = DeltaEditType::SetProperty;
    edit.entityID      = 7;
    edit.propertyName  = "color";
    edit.propertyValue = "red";
    store.Record(edit);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.propertyCount == 1);
    assert(summary.TotalChanges() == 1);
}

void test_vdiff_mixed_edits() {
    DeltaEditStore store(42);

    DeltaEdit add{};
    add.type       = DeltaEditType::AddObject;
    add.entityID   = 1;
    add.objectType = "ship";
    store.Record(add);

    DeltaEdit rem{};
    rem.type     = DeltaEditType::RemoveObject;
    rem.entityID = 2;
    store.Record(rem);

    DeltaEdit mov{};
    mov.type        = DeltaEditType::MoveObject;
    mov.entityID    = 3;
    mov.position[0] = 5.0f;
    store.Record(mov);

    DeltaEdit prop{};
    prop.type          = DeltaEditType::SetProperty;
    prop.entityID      = 4;
    prop.propertyName  = "hp";
    prop.propertyValue = "100";
    store.Record(prop);

    DeltaEdit prop2{};
    prop2.type          = DeltaEditType::SetProperty;
    prop2.entityID      = 5;
    prop2.propertyName  = "shield";
    prop2.propertyValue = "50";
    store.Record(prop2);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.addedCount == 1);
    assert(summary.removedCount == 1);
    assert(summary.movedCount == 1);
    assert(summary.propertyCount == 2);
    assert(summary.TotalChanges() == 5);
    assert(summary.entries.size() == 5);
}

void test_vdiff_total_changes() {
    DeltaEditStore store(42);
    DeltaEdit e1{}; e1.type = DeltaEditType::AddObject; e1.entityID = 1;
    DeltaEdit e2{}; e2.type = DeltaEditType::MoveObject; e2.entityID = 2;
    DeltaEdit e3{}; e3.type = DeltaEditType::MoveObject; e3.entityID = 3;
    store.Record(e1);
    store.Record(e2);
    store.Record(e3);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    assert(summary.TotalChanges() == 3);
}

void test_vdiff_affected_entities() {
    DeltaEditStore store(42);
    DeltaEdit e1{}; e1.type = DeltaEditType::AddObject; e1.entityID = 5;
    DeltaEdit e2{}; e2.type = DeltaEditType::MoveObject; e2.entityID = 3;
    DeltaEdit e3{}; e3.type = DeltaEditType::SetProperty; e3.entityID = 5;
    e3.propertyName = "x"; e3.propertyValue = "1";
    store.Record(e1);
    store.Record(e2);
    store.Record(e3);

    VisualDiffTool diff;
    auto affected = diff.AffectedEntities(store);
    assert(affected.size() == 2);
    // Should be sorted
    assert(affected[0] == 3);
    assert(affected[1] == 5);
}

void test_vdiff_entries_for_entity() {
    DeltaEditStore store(42);
    DeltaEdit e1{}; e1.type = DeltaEditType::AddObject; e1.entityID = 1;
    DeltaEdit e2{}; e2.type = DeltaEditType::MoveObject; e2.entityID = 1;
    DeltaEdit e3{}; e3.type = DeltaEditType::MoveObject; e3.entityID = 2;
    store.Record(e1);
    store.Record(e2);
    store.Record(e3);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    auto entity1 = diff.EntriesForEntity(summary, 1);
    assert(entity1.size() == 2);
    auto entity2 = diff.EntriesForEntity(summary, 2);
    assert(entity2.size() == 1);
    auto entity99 = diff.EntriesForEntity(summary, 99);
    assert(entity99.empty());
}

void test_vdiff_entries_by_type() {
    DeltaEditStore store(42);
    DeltaEdit e1{}; e1.type = DeltaEditType::AddObject; e1.entityID = 1;
    DeltaEdit e2{}; e2.type = DeltaEditType::MoveObject; e2.entityID = 2;
    DeltaEdit e3{}; e3.type = DeltaEditType::MoveObject; e3.entityID = 3;
    DeltaEdit e4{}; e4.type = DeltaEditType::SetProperty; e4.entityID = 4;
    e4.propertyName = "x"; e4.propertyValue = "1";
    store.Record(e1);
    store.Record(e2);
    store.Record(e3);
    store.Record(e4);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    auto moves = diff.EntriesByType(summary, DeltaEditType::MoveObject);
    assert(moves.size() == 2);
    auto adds = diff.EntriesByType(summary, DeltaEditType::AddObject);
    assert(adds.size() == 1);
    auto removes = diff.EntriesByType(summary, DeltaEditType::RemoveObject);
    assert(removes.empty());
}

void test_vdiff_descriptions_nonempty() {
    DeltaEditStore store(42);
    DeltaEdit e1{}; e1.type = DeltaEditType::AddObject; e1.entityID = 1;
    e1.objectType = "ship";
    DeltaEdit e2{}; e2.type = DeltaEditType::RemoveObject; e2.entityID = 2;
    DeltaEdit e3{}; e3.type = DeltaEditType::MoveObject; e3.entityID = 3;
    DeltaEdit e4{}; e4.type = DeltaEditType::SetProperty; e4.entityID = 4;
    e4.propertyName = "x"; e4.propertyValue = "1";
    store.Record(e1);
    store.Record(e2);
    store.Record(e3);
    store.Record(e4);

    VisualDiffTool diff;
    DiffSummary summary = diff.ComputeDiff(store);
    for (const auto& entry : summary.entries) {
        assert(!entry.description.empty());
    }
}
