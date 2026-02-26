/**
 * Tests for KeybindManager — editor keyboard shortcut system.
 */

#include <cassert>
#include <string>
#include "../editor/ui/KeybindManager.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_kb_defaults_installed() {
    KeybindManager mgr;
    assert(mgr.BindingCount() > 0);
    // Should have the Undo binding
    const Keybind* undo = mgr.FindBinding("Undo");
    assert(undo != nullptr);
    assert(undo->key == 'Z');
    assert(undo->mods == KeyMod::Ctrl);
}

void test_kb_clear() {
    KeybindManager mgr;
    mgr.Clear();
    assert(mgr.BindingCount() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Binding management
// ══════════════════════════════════════════════════════════════════

void test_kb_add_binding() {
    KeybindManager mgr;
    mgr.Clear();
    assert(mgr.AddBinding({"MyAction", "Test", 'A', KeyMod::None}));
    assert(mgr.BindingCount() == 1);
    const Keybind* kb = mgr.FindBinding("MyAction");
    assert(kb != nullptr);
    assert(kb->key == 'A');
    assert(kb->category == "Test");
}

void test_kb_add_duplicate_action_rejected() {
    KeybindManager mgr;
    mgr.Clear();
    assert(mgr.AddBinding({"Test", "Cat", 'A', KeyMod::None}));
    assert(!mgr.AddBinding({"Test", "Cat", 'B', KeyMod::None}));
    assert(mgr.BindingCount() == 1);
}

void test_kb_add_empty_action_rejected() {
    KeybindManager mgr;
    mgr.Clear();
    assert(!mgr.AddBinding({"", "Cat", 'A', KeyMod::None}));
}

void test_kb_remove_binding() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'A', KeyMod::None});
    assert(mgr.RemoveBinding("TestAction"));
    assert(mgr.BindingCount() == 0);
    assert(mgr.FindBinding("TestAction") == nullptr);
}

void test_kb_remove_nonexistent() {
    KeybindManager mgr;
    mgr.Clear();
    assert(!mgr.RemoveBinding("NonExistent"));
}

void test_kb_rebind() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'A', KeyMod::None});
    assert(mgr.Rebind("TestAction", 'B', KeyMod::Ctrl));
    const Keybind* kb = mgr.FindBinding("TestAction");
    assert(kb->key == 'B');
    assert(kb->mods == KeyMod::Ctrl);
}

void test_kb_rebind_nonexistent() {
    KeybindManager mgr;
    mgr.Clear();
    assert(!mgr.Rebind("NonExistent", 'A', KeyMod::None));
}

void test_kb_set_enabled() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'A', KeyMod::None});
    assert(mgr.SetEnabled("TestAction", false));
    assert(!mgr.FindBinding("TestAction")->enabled);
    assert(mgr.SetEnabled("TestAction", true));
    assert(mgr.FindBinding("TestAction")->enabled);
}

// ══════════════════════════════════════════════════════════════════
// Key lookup and categories
// ══════════════════════════════════════════════════════════════════

void test_kb_find_by_key() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"ActionA", "Cat", 'A', KeyMod::Ctrl});
    mgr.AddBinding({"ActionB", "Cat", 'B', KeyMod::Ctrl});
    auto results = mgr.FindByKey('A', KeyMod::Ctrl);
    assert(results.size() == 1);
    assert(results[0]->action == "ActionA");
}

void test_kb_bindings_in_category() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"A1", "CatA", 'A', KeyMod::None});
    mgr.AddBinding({"A2", "CatA", 'B', KeyMod::None});
    mgr.AddBinding({"B1", "CatB", 'C', KeyMod::None});
    auto catA = mgr.BindingsInCategory("CatA");
    assert(catA.size() == 2);
    auto catB = mgr.BindingsInCategory("CatB");
    assert(catB.size() == 1);
}

// ══════════════════════════════════════════════════════════════════
// Conflict detection
// ══════════════════════════════════════════════════════════════════

void test_kb_conflict_detection() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"ActionA", "Cat", 'A', KeyMod::Ctrl});
    assert(mgr.HasConflict('A', KeyMod::Ctrl));
    assert(!mgr.HasConflict('B', KeyMod::Ctrl));
}

void test_kb_conflict_with_exclude() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"ActionA", "Cat", 'A', KeyMod::Ctrl});
    // Excluding ActionA from conflict check → no conflict
    assert(!mgr.HasConflict('A', KeyMod::Ctrl, "ActionA"));
}

// ══════════════════════════════════════════════════════════════════
// Action dispatch
// ══════════════════════════════════════════════════════════════════

void test_kb_handle_key_press() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'T', KeyMod::Ctrl});

    int callCount = 0;
    mgr.RegisterCallback("TestAction", [&]() { ++callCount; });

    assert(mgr.HandleKeyPress('T', KeyMod::Ctrl));
    assert(callCount == 1);
}

void test_kb_handle_key_press_no_match() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'T', KeyMod::Ctrl});
    mgr.RegisterCallback("TestAction", [&]() {});

    // Wrong key
    assert(!mgr.HandleKeyPress('X', KeyMod::Ctrl));
    // Wrong mod
    assert(!mgr.HandleKeyPress('T', KeyMod::None));
}

void test_kb_disabled_binding_not_dispatched() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'T', KeyMod::None});
    int callCount = 0;
    mgr.RegisterCallback("TestAction", [&]() { ++callCount; });

    mgr.SetEnabled("TestAction", false);
    assert(!mgr.HandleKeyPress('T', KeyMod::None));
    assert(callCount == 0);
}

void test_kb_callback_replacement() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestAction", "Cat", 'T', KeyMod::None});
    int v = 0;
    mgr.RegisterCallback("TestAction", [&]() { v = 1; });
    mgr.RegisterCallback("TestAction", [&]() { v = 2; }); // replace
    mgr.HandleKeyPress('T', KeyMod::None);
    assert(v == 2);
}

// ══════════════════════════════════════════════════════════════════
// Display helpers
// ══════════════════════════════════════════════════════════════════

void test_kb_describe_binding() {
    assert(KeybindManager::DescribeBinding('Z', KeyMod::Ctrl) == "Ctrl+Z");
    assert(KeybindManager::DescribeBinding('S', KeyMod::Ctrl | KeyMod::Shift) == "Ctrl+Shift+S");
    assert(KeybindManager::DescribeBinding('G', KeyMod::None) == "G");
    assert(KeybindManager::DescribeBinding(32, KeyMod::None) == "Space");
    assert(KeybindManager::DescribeBinding(27, KeyMod::None) == "Escape");
}

void test_kb_describe_action() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"Save", "General", 'S', KeyMod::Ctrl});
    std::string desc = mgr.DescribeAction("Save");
    assert(desc.find("Ctrl+S") != std::string::npos);

    // Unbound action
    std::string unbound = mgr.DescribeAction("UnknownAction");
    assert(unbound.find("unbound") != std::string::npos);
}

// ══════════════════════════════════════════════════════════════════
// Default bindings validation
// ══════════════════════════════════════════════════════════════════

void test_kb_default_bindings() {
    KeybindManager mgr;
    // Check a few standard defaults
    assert(mgr.FindBinding("Undo") != nullptr);
    assert(mgr.FindBinding("Redo") != nullptr);
    assert(mgr.FindBinding("Save") != nullptr);
    assert(mgr.FindBinding("Translate") != nullptr);
    assert(mgr.FindBinding("Rotate") != nullptr);
    assert(mgr.FindBinding("Scale") != nullptr);
    assert(mgr.FindBinding("ToggleGrid") != nullptr);

    // Verify categories
    auto general  = mgr.BindingsInCategory("General");
    auto viewport = mgr.BindingsInCategory("Viewport");
    auto panels   = mgr.BindingsInCategory("Panels");
    assert(!general.empty());
    assert(!viewport.empty());
    assert(!panels.empty());
}

// ══════════════════════════════════════════════════════════════════
// Persistence (serialize / deserialize / file I/O)
// ══════════════════════════════════════════════════════════════════

void test_kb_serialize_roundtrip() {
    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"TestSave", "Cat", 'S', KeyMod::Ctrl, true});
    mgr.AddBinding({"TestDel",  "Cat", 127,  KeyMod::None, false});

    std::string json = mgr.SerializeToJSON();
    assert(!json.empty());
    assert(json.find("TestSave") != std::string::npos);
    assert(json.find("TestDel") != std::string::npos);

    KeybindManager mgr2;
    mgr2.Clear();
    bool ok = mgr2.DeserializeFromJSON(json);
    assert(ok);
    assert(mgr2.BindingCount() == 2);

    const Keybind* s = mgr2.FindBinding("TestSave");
    assert(s != nullptr);
    assert(s->key == 'S');
    assert(s->mods == KeyMod::Ctrl);
    assert(s->enabled == true);

    const Keybind* d = mgr2.FindBinding("TestDel");
    assert(d != nullptr);
    assert(d->key == 127);
    assert(d->enabled == false);
}

void test_kb_deserialize_empty_returns_false() {
    KeybindManager mgr;
    assert(!mgr.DeserializeFromJSON(""));
    assert(!mgr.DeserializeFromJSON("{}"));
}

void test_kb_file_roundtrip() {
    std::string path = "/tmp/atlas_test_keybinds.json";

    KeybindManager mgr;
    mgr.Clear();
    mgr.AddBinding({"FileSave", "General", 'F', KeyMod::Ctrl | KeyMod::Shift});

    bool saved = mgr.SaveToFile(path);
    assert(saved);

    KeybindManager mgr2;
    mgr2.Clear();
    bool loaded = mgr2.LoadFromFile(path);
    assert(loaded);
    assert(mgr2.BindingCount() == 1);
    const Keybind* kb = mgr2.FindBinding("FileSave");
    assert(kb != nullptr);
    assert(kb->key == 'F');
    assert(kb->mods == (KeyMod::Ctrl | KeyMod::Shift));

    std::remove(path.c_str());
}

void test_kb_load_nonexistent_returns_false() {
    KeybindManager mgr;
    assert(!mgr.LoadFromFile("/tmp/nonexistent_keybinds_test.json"));
}
