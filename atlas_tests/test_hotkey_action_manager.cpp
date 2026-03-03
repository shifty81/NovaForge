/**
 * Tests for HotkeyActionManager:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - Bind posts command, records DeltaEdit with hotkey: prefix
 *   - Bind undo restores old binding
 *   - Unbind posts command, records DeltaEdit with empty value
 *   - Unbind undo restores previous binding
 *   - Local binding map tracks keys and actions
 *   - AllBindings returns all current bindings
 *   - ClearBindings removes all local bindings
 *   - Multiple bindings on different keys
 *   - Rebinding an existing key
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/hotkey_action_manager.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// HotkeyActionManager tests
// ══════════════════════════════════════════════════════════════════

void test_hotkey_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    HotkeyActionManager tool(bus, store);
    assert(std::string(tool.Name()) == "Hotkey Action Manager");
}

void test_hotkey_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    HotkeyActionManager tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_hotkey_bind() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Activate();
    tool.Bind(1, "Ctrl+Z", "undo");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "hotkey:Ctrl+Z");
    assert(edit.propertyValue == "undo");
}

void test_hotkey_bind_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "G", "grab");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "grab");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "hotkey:G");
    assert(store.Edits()[1].propertyValue == "");  // was unbound before
}

void test_hotkey_unbind() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "R", "rotate");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    tool.Unbind(1, "R");
    bus.ProcessCommands();
    assert(store.Count() == 2);

    const auto& edit = store.Edits()[1];
    assert(edit.propertyName == "hotkey:R");
    assert(edit.propertyValue == "");  // empty = unbound
}

void test_hotkey_unbind_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "S", "scale");
    bus.ProcessCommands();

    tool.Unbind(1, "S");
    bus.ProcessCommands();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "");

    bus.Undo();
    assert(store.Count() == 3);
    assert(store.Edits()[2].propertyName == "hotkey:S");
    assert(store.Edits()[2].propertyValue == "scale");
}

void test_hotkey_local_map() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    HotkeyActionManager tool(bus, store);

    assert(tool.BindingCount() == 0);
    assert(tool.ActionForKey("G") == "");

    tool.Bind(1, "G", "grab");
    assert(tool.BindingCount() == 1);
    assert(tool.ActionForKey("G") == "grab");

    tool.Bind(1, "R", "rotate");
    assert(tool.BindingCount() == 2);
    assert(tool.ActionForKey("R") == "rotate");

    tool.Unbind(1, "G");
    assert(tool.BindingCount() == 1);
    assert(tool.ActionForKey("G") == "");
}

void test_hotkey_all_bindings() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "G", "grab");
    tool.Bind(1, "R", "rotate");
    tool.Bind(1, "S", "scale");

    auto bindings = tool.AllBindings();
    assert(bindings.size() == 3);

    // Check that all expected bindings are present (order may vary)
    bool foundG = false, foundR = false, foundS = false;
    for (const auto& b : bindings) {
        if (b.key == "G" && b.action == "grab") foundG = true;
        if (b.key == "R" && b.action == "rotate") foundR = true;
        if (b.key == "S" && b.action == "scale") foundS = true;
    }
    assert(foundG && foundR && foundS);
}

void test_hotkey_clear_bindings() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "G", "grab");
    tool.Bind(1, "R", "rotate");
    assert(tool.BindingCount() == 2);

    tool.ClearBindings();
    assert(tool.BindingCount() == 0);
    assert(tool.ActionForKey("G") == "");
}

void test_hotkey_rebind() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "G", "grab");
    bus.ProcessCommands();

    tool.Bind(1, "G", "translate");
    bus.ProcessCommands();
    assert(store.Count() == 2);

    assert(store.Edits()[1].propertyName == "hotkey:G");
    assert(store.Edits()[1].propertyValue == "translate");
    assert(tool.ActionForKey("G") == "translate");
}

void test_hotkey_rebind_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "G", "grab");
    bus.ProcessCommands();

    tool.Bind(1, "G", "translate");
    bus.ProcessCommands();

    bus.Undo();
    assert(store.Count() == 3);
    assert(store.Edits()[2].propertyName == "hotkey:G");
    assert(store.Edits()[2].propertyValue == "grab");
}

void test_hotkey_multiple_keys() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    HotkeyActionManager tool(bus, store);

    tool.Bind(1, "Ctrl+Z", "undo");
    tool.Bind(1, "Ctrl+Y", "redo");
    tool.Bind(1, "Shift+D", "duplicate");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "hotkey:Ctrl+Z");
    assert(store.Edits()[1].propertyName == "hotkey:Ctrl+Y");
    assert(store.Edits()[2].propertyName == "hotkey:Shift+D");
}
