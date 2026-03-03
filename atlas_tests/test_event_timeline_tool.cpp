/**
 * Tests for EventTimelineTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - AddEvent posts command, records DeltaEdit
 *   - RemoveEvent posts command, records DeltaEdit
 *   - SetEventProperty posts command, records DeltaEdit
 *   - Undo reverses add, remove, and property changes
 *   - Multiple events in sequence
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/event_timeline_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// EventTimelineTool tests
// ══════════════════════════════════════════════════════════════════

void test_event_timeline_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EventTimelineTool tool(bus, store);
    assert(std::string(tool.Name()) == "Event Timeline");
}

void test_event_timeline_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EventTimelineTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_event_timeline_add_event() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.Activate();
    tool.AddEvent(1, "spawn", 0.0f);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::AddObject);
    assert(edit.entityID == 1);
    assert(edit.objectType == "timeline_event:spawn");
    assert(edit.position[0] == 0.0f);
}

void test_event_timeline_add_event_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.AddEvent(1, "animation", 1.5f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].type == DeltaEditType::RemoveObject);
    assert(store.Edits()[1].entityID == 1);
}

void test_event_timeline_remove_event() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.RemoveEvent(1, "physics", 2.0f);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::RemoveObject);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "event_type");
    assert(edit.propertyValue == "physics");
}

void test_event_timeline_remove_event_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.RemoveEvent(1, "trigger", 3.0f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].type == DeltaEditType::AddObject);
    assert(store.Edits()[1].objectType == "timeline_event:trigger");
    assert(store.Edits()[1].position[0] == 3.0f);
}

void test_event_timeline_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.SetEventProperty(1, "duration", "1.0", "2.5");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "duration");
    assert(edit.propertyValue == "2.5");
}

void test_event_timeline_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.SetEventProperty(1, "duration", "1.0", "2.5");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "2.5");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "1.0");
}

void test_event_timeline_multiple_events() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EventTimelineTool tool(bus, store);

    tool.AddEvent(1, "spawn", 0.0f);
    tool.AddEvent(1, "animation", 1.0f);
    tool.AddEvent(1, "physics", 2.0f);
    tool.AddEvent(1, "trigger", 3.0f);
    assert(bus.PendingCount() == 4);

    bus.ProcessCommands();
    assert(store.Count() == 4);
    assert(store.Edits()[0].objectType == "timeline_event:spawn");
    assert(store.Edits()[1].objectType == "timeline_event:animation");
    assert(store.Edits()[2].objectType == "timeline_event:physics");
    assert(store.Edits()[3].objectType == "timeline_event:trigger");
}
