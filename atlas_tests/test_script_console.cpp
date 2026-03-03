/**
 * Tests for ScriptConsole:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - Log/Warn/Error add entries with correct level
 *   - EntryCount tracks entries
 *   - Clear removes all entries
 *   - Filter by level
 *   - Ring buffer evicts oldest entries when capacity exceeded
 *   - ExecuteSetProperty posts undoable command with console: prefix
 *   - ExecuteSetProperty undo restores old value
 *   - Multiple commands accumulate in store
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/script_console.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// ScriptConsole tests
// ══════════════════════════════════════════════════════════════════

void test_sconsole_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);
    assert(std::string(console.Name()) == "Script Console");
}

void test_sconsole_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);

    assert(!console.IsActive());
    console.Activate();
    assert(console.IsActive());
    console.Deactivate();
    assert(!console.IsActive());
}

void test_sconsole_log() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);

    console.Log("Hello world");
    assert(console.EntryCount() == 1);
    assert(console.Entries()[0].level == ConsoleEntry::Level::Info);
    assert(console.Entries()[0].message == "Hello world");
}

void test_sconsole_warn() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);

    console.Warn("Low memory");
    assert(console.EntryCount() == 1);
    assert(console.Entries()[0].level == ConsoleEntry::Level::Warning);
    assert(console.Entries()[0].message == "Low memory");
}

void test_sconsole_error() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);

    console.Error("Crash detected");
    assert(console.EntryCount() == 1);
    assert(console.Entries()[0].level == ConsoleEntry::Level::Error);
    assert(console.Entries()[0].message == "Crash detected");
}

void test_sconsole_clear() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);

    console.Log("msg1");
    console.Warn("msg2");
    console.Error("msg3");
    assert(console.EntryCount() == 3);

    console.Clear();
    assert(console.EntryCount() == 0);
}

void test_sconsole_filter() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store);

    console.Log("info1");
    console.Warn("warn1");
    console.Error("error1");
    console.Log("info2");
    console.Warn("warn2");

    auto infos = console.Filter(ConsoleEntry::Level::Info);
    assert(infos.size() == 2);
    assert(infos[0]->message == "info1");
    assert(infos[1]->message == "info2");

    auto warns = console.Filter(ConsoleEntry::Level::Warning);
    assert(warns.size() == 2);

    auto errors = console.Filter(ConsoleEntry::Level::Error);
    assert(errors.size() == 1);
    assert(errors[0]->message == "error1");
}

void test_sconsole_ring_buffer() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ScriptConsole console(bus, store, 3);  // max 3 entries

    console.Log("msg1");
    console.Log("msg2");
    console.Log("msg3");
    assert(console.EntryCount() == 3);

    console.Log("msg4");  // should evict msg1
    assert(console.EntryCount() == 3);
    assert(console.Entries()[0].message == "msg2");
    assert(console.Entries()[1].message == "msg3");
    assert(console.Entries()[2].message == "msg4");
}

void test_sconsole_execute_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ScriptConsole console(bus, store);

    console.ExecuteSetProperty(1, "debug_flag", "false", "true");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "console:debug_flag");
    assert(edit.propertyValue == "true");
}

void test_sconsole_execute_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ScriptConsole console(bus, store);

    console.ExecuteSetProperty(1, "speed", "1.0", "2.0");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "2.0");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "console:speed");
    assert(store.Edits()[1].propertyValue == "1.0");
}

void test_sconsole_multiple_commands() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ScriptConsole console(bus, store);

    console.ExecuteSetProperty(1, "flag_a", "off", "on");
    console.ExecuteSetProperty(2, "flag_b", "0", "1");
    console.ExecuteSetProperty(3, "mode", "normal", "debug");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[2].entityID == 3);
}
