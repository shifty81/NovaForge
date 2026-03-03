/**
 * Tests for PrefabLibrary:
 *   - Default state (empty)
 *   - Add/remove prefabs
 *   - Duplicate name rejected
 *   - Empty name rejected
 *   - Get by name
 *   - Rename
 *   - Filter by type
 *   - Unique types
 *   - Properties stored correctly
 *   - Clear
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/prefab_library.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// PrefabLibrary tests
// ══════════════════════════════════════════════════════════════════

void test_prefab_library_defaults() {
    PrefabLibrary lib;
    assert(lib.Count() == 0);
    assert(lib.Entries().empty());
}

void test_prefab_library_add() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "PatrolFrigate";
    e.type = "ship";
    assert(lib.Add(e));
    assert(lib.Count() == 1);
}

void test_prefab_library_add_duplicate() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "PatrolFrigate";
    e.type = "ship";
    assert(lib.Add(e));
    assert(!lib.Add(e));
    assert(lib.Count() == 1);
}

void test_prefab_library_add_empty_name() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "";
    e.type = "ship";
    assert(!lib.Add(e));
    assert(lib.Count() == 0);
}

void test_prefab_library_remove() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "PatrolFrigate";
    e.type = "ship";
    lib.Add(e);
    assert(lib.Remove("PatrolFrigate"));
    assert(lib.Count() == 0);
    assert(!lib.Remove("PatrolFrigate"));
}

void test_prefab_library_get() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "PatrolFrigate";
    e.type = "ship";
    e.description = "A standard patrol vessel";
    lib.Add(e);

    const auto* p = lib.Get("PatrolFrigate");
    assert(p != nullptr);
    assert(p->name == "PatrolFrigate");
    assert(p->type == "ship");
    assert(p->description == "A standard patrol vessel");
    assert(lib.Get("Nonexistent") == nullptr);
}

void test_prefab_library_rename() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "OldName";
    e.type = "prop";
    lib.Add(e);

    assert(lib.Rename("OldName", "NewName"));
    assert(lib.Get("OldName") == nullptr);
    assert(lib.Get("NewName") != nullptr);
    assert(lib.Get("NewName")->type == "prop");
}

void test_prefab_library_rename_conflict() {
    PrefabLibrary lib;
    PrefabEntry a;
    a.name = "Alpha";
    a.type = "ship";
    PrefabEntry b;
    b.name = "Beta";
    b.type = "ship";
    lib.Add(a);
    lib.Add(b);

    assert(!lib.Rename("Alpha", "Beta")); // conflict
    assert(lib.Get("Alpha") != nullptr);
}

void test_prefab_library_rename_empty() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "Alpha";
    e.type = "ship";
    lib.Add(e);
    assert(!lib.Rename("Alpha", ""));
}

void test_prefab_library_rename_not_found() {
    PrefabLibrary lib;
    assert(!lib.Rename("Ghost", "NewName"));
}

void test_prefab_library_filter_by_type() {
    PrefabLibrary lib;
    PrefabEntry s1; s1.name = "Frigate"; s1.type = "ship";
    PrefabEntry s2; s2.name = "Destroyer"; s2.type = "ship";
    PrefabEntry p1; p1.name = "Crate"; p1.type = "prop";
    lib.Add(s1);
    lib.Add(s2);
    lib.Add(p1);

    auto ships = lib.FilterByType("ship");
    assert(ships.size() == 2);
    auto props = lib.FilterByType("prop");
    assert(props.size() == 1);
    auto chars = lib.FilterByType("character");
    assert(chars.empty());
}

void test_prefab_library_unique_types() {
    PrefabLibrary lib;
    PrefabEntry a; a.name = "A"; a.type = "ship";
    PrefabEntry b; b.name = "B"; b.type = "prop";
    PrefabEntry c; c.name = "C"; c.type = "ship";
    lib.Add(a);
    lib.Add(b);
    lib.Add(c);
    auto types = lib.UniqueTypes();
    assert(types.size() == 2);
}

void test_prefab_library_properties() {
    PrefabLibrary lib;
    PrefabEntry e;
    e.name = "HeavyFrigate";
    e.type = "ship";
    e.properties["hull"] = "frigate_mk2";
    e.properties["armor"] = "heavy_plate";
    e.properties["slots"] = "6";
    lib.Add(e);

    const auto* p = lib.Get("HeavyFrigate");
    assert(p != nullptr);
    assert(p->properties.size() == 3);
    assert(p->properties.at("hull") == "frigate_mk2");
    assert(p->properties.at("armor") == "heavy_plate");
    assert(p->properties.at("slots") == "6");
}

void test_prefab_library_clear() {
    PrefabLibrary lib;
    PrefabEntry e; e.name = "A"; e.type = "ship";
    lib.Add(e);
    lib.Clear();
    assert(lib.Count() == 0);
    assert(lib.Entries().empty());
}
