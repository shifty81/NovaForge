/**
 * Tests for DeltaEditsMergeTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - DetectConflicts returns 0 conflicts for non-overlapping edits
 *   - DetectConflicts finds conflicts for same entityID+propertyName with different values
 *   - ConflictCount returns correct number
 *   - MergeNonConflicting skips conflicting edits, merges the rest
 *   - MergeAll includes all edits from theirs
 *   - Undo of MergeAll reverts store to pre-merge state
 *   - MergeResolved with acceptTheirs=true includes theirs value
 *   - MergeResolved with acceptTheirs=false skips theirs value
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/delta_edits_merge_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// DeltaEditsMergeTool tests
// ══════════════════════════════════════════════════════════════════

void test_merge_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    DeltaEditsMergeTool tool(bus, store);
    assert(std::string(tool.Name()) == "DeltaEdits Merge");
}

void test_merge_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    DeltaEditsMergeTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_merge_detect_no_conflicts() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    // Ours: entity 1, property "gravity"
    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    // Theirs: entity 2, property "wind"
    DeltaEditStore theirs(42);
    DeltaEdit editTheirs{};
    editTheirs.type = DeltaEditType::SetProperty;
    editTheirs.entityID = 2;
    editTheirs.propertyName = "wind";
    editTheirs.propertyValue = "5.0";
    theirs.Record(editTheirs);

    auto conflicts = tool.DetectConflicts(theirs);
    assert(conflicts.size() == 0);
}

void test_merge_detect_conflicts() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    // Ours: entity 1, property "gravity" = "9.81"
    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    // Theirs: entity 1, property "gravity" = "0.0" (conflict!)
    DeltaEditStore theirs(42);
    DeltaEdit editTheirs{};
    editTheirs.type = DeltaEditType::SetProperty;
    editTheirs.entityID = 1;
    editTheirs.propertyName = "gravity";
    editTheirs.propertyValue = "0.0";
    theirs.Record(editTheirs);

    auto conflicts = tool.DetectConflicts(theirs);
    assert(conflicts.size() == 1);
    assert(conflicts[0].entityID == 1);
    assert(conflicts[0].propertyName == "gravity");
    assert(conflicts[0].oursValue == "9.81");
    assert(conflicts[0].theirsValue == "0.0");
}

void test_merge_conflict_count() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    // Two conflicting properties
    DeltaEdit e1{};
    e1.type = DeltaEditType::SetProperty;
    e1.entityID = 1;
    e1.propertyName = "gravity";
    e1.propertyValue = "9.81";
    ours.Record(e1);

    DeltaEdit e2{};
    e2.type = DeltaEditType::SetProperty;
    e2.entityID = 1;
    e2.propertyName = "wind";
    e2.propertyValue = "1.0";
    ours.Record(e2);

    DeltaEditStore theirs(42);
    DeltaEdit t1{};
    t1.type = DeltaEditType::SetProperty;
    t1.entityID = 1;
    t1.propertyName = "gravity";
    t1.propertyValue = "0.0";
    theirs.Record(t1);

    DeltaEdit t2{};
    t2.type = DeltaEditType::SetProperty;
    t2.entityID = 1;
    t2.propertyName = "wind";
    t2.propertyValue = "5.0";
    theirs.Record(t2);

    assert(tool.ConflictCount(theirs) == 2);
}

void test_merge_non_conflicting() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    // Ours has "gravity"
    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    // Theirs has "gravity" (conflict) and "wind" (no conflict)
    DeltaEditStore theirs(42);
    DeltaEdit t1{};
    t1.type = DeltaEditType::SetProperty;
    t1.entityID = 1;
    t1.propertyName = "gravity";
    t1.propertyValue = "0.0";
    theirs.Record(t1);

    DeltaEdit t2{};
    t2.type = DeltaEditType::SetProperty;
    t2.entityID = 2;
    t2.propertyName = "wind";
    t2.propertyValue = "5.0";
    theirs.Record(t2);

    tool.MergeNonConflicting(theirs);
    bus.ProcessCommands();

    // Ours should now have the original edit + the non-conflicting "wind" edit
    assert(ours.Count() == 2);
    assert(ours.Edits()[1].propertyName == "wind");
    assert(ours.Edits()[1].propertyValue == "5.0");
}

void test_merge_all() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    // Ours has "gravity"
    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    // Theirs has "gravity" (conflict) and "wind"
    DeltaEditStore theirs(42);
    DeltaEdit t1{};
    t1.type = DeltaEditType::SetProperty;
    t1.entityID = 1;
    t1.propertyName = "gravity";
    t1.propertyValue = "0.0";
    theirs.Record(t1);

    DeltaEdit t2{};
    t2.type = DeltaEditType::SetProperty;
    t2.entityID = 2;
    t2.propertyName = "wind";
    t2.propertyValue = "5.0";
    theirs.Record(t2);

    tool.MergeAll(theirs);
    bus.ProcessCommands();

    // Ours original (1) + all theirs (2) = 3
    assert(ours.Count() == 3);
    assert(ours.Edits()[1].propertyName == "gravity");
    assert(ours.Edits()[1].propertyValue == "0.0");
    assert(ours.Edits()[2].propertyName == "wind");
    assert(ours.Edits()[2].propertyValue == "5.0");
}

void test_merge_all_undo() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    DeltaEditStore theirs(42);
    DeltaEdit t1{};
    t1.type = DeltaEditType::SetProperty;
    t1.entityID = 2;
    t1.propertyName = "wind";
    t1.propertyValue = "5.0";
    theirs.Record(t1);

    tool.MergeAll(theirs);
    bus.ProcessCommands();
    assert(ours.Count() == 2);

    bus.Undo();
    assert(ours.Count() == 1);
    assert(ours.Edits()[0].propertyName == "gravity");
    assert(ours.Edits()[0].propertyValue == "9.81");
}

void test_merge_resolved_accept_theirs() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    DeltaEditStore theirs(42);
    DeltaEdit t1{};
    t1.type = DeltaEditType::SetProperty;
    t1.entityID = 1;
    t1.propertyName = "gravity";
    t1.propertyValue = "0.0";
    theirs.Record(t1);

    // Resolve conflict: accept theirs
    auto conflicts = tool.DetectConflicts(theirs);
    assert(conflicts.size() == 1);
    conflicts[0].resolved = true;
    conflicts[0].acceptTheirs = true;

    tool.MergeResolved(conflicts, theirs);
    bus.ProcessCommands();

    // Ours original (1) + theirs gravity (1) = 2
    assert(ours.Count() == 2);
    assert(ours.Edits()[1].propertyName == "gravity");
    assert(ours.Edits()[1].propertyValue == "0.0");
}

void test_merge_resolved_keep_ours() {
    UndoableCommandBus bus;
    DeltaEditStore ours(42);
    DeltaEditsMergeTool tool(bus, ours);

    DeltaEdit editOurs{};
    editOurs.type = DeltaEditType::SetProperty;
    editOurs.entityID = 1;
    editOurs.propertyName = "gravity";
    editOurs.propertyValue = "9.81";
    ours.Record(editOurs);

    DeltaEditStore theirs(42);
    DeltaEdit t1{};
    t1.type = DeltaEditType::SetProperty;
    t1.entityID = 1;
    t1.propertyName = "gravity";
    t1.propertyValue = "0.0";
    theirs.Record(t1);

    // Resolve conflict: keep ours
    auto conflicts = tool.DetectConflicts(theirs);
    assert(conflicts.size() == 1);
    conflicts[0].resolved = true;
    conflicts[0].acceptTheirs = false;

    tool.MergeResolved(conflicts, theirs);
    bus.ProcessCommands();

    // Only ours original edit remains — theirs was skipped
    assert(ours.Count() == 1);
    assert(ours.Edits()[0].propertyValue == "9.81");
}
