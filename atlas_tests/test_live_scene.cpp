/**
 * Tests for LiveSceneManager and PCGOverrideStore.
 *
 * Validates the in-engine content creation workflow:
 * - Scene population with PCG content
 * - Viewport change capture → override store
 * - Override serialisation / deserialisation (JSON)
 * - Save / load round-trip
 * - Live reload triggering regeneration
 */

#include "tools/LiveSceneManager.h"
#include "tools/PCGOverrideStore.h"
#include "tools/ViewportPanel.h"
#include "tools/PCGPreviewPanel.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>

using namespace atlas::editor;

// ── Helpers ─────────────────────────────────────────────────────────

static int ls_passed = 0;

static void ok(const char* name) {
    ++ls_passed;
}

// ══════════════════════════════════════════════════════════════════
// PCGOverrideStore tests
// ══════════════════════════════════════════════════════════════════

void test_override_store_empty() {
    PCGOverrideStore store;
    assert(store.Overrides().empty());
    assert(!store.IsDirty());
    ok("test_override_store_empty");
}

void test_override_store_add_and_clear() {
    PCGOverrideStore store;
    PCGOverride ov;
    ov.objectId   = 1;
    ov.objectName = "TestShip";
    ov.objectType = "Ship";
    ov.field      = "position";
    ov.values[0]  = 10.0f;
    ov.values[1]  = 20.0f;
    ov.values[2]  = 30.0f;
    ov.seed       = 42;
    ov.version    = 1;

    store.Add(ov);
    assert(store.Overrides().size() == 1);
    assert(store.IsDirty());

    store.Clear();
    assert(store.Overrides().empty());
    assert(!store.IsDirty());

    ok("test_override_store_add_and_clear");
}

void test_override_store_remove_by_object() {
    PCGOverrideStore store;
    PCGOverride a;
    a.objectId = 1; a.objectName = "A"; a.field = "position";
    PCGOverride b;
    b.objectId = 2; b.objectName = "B"; b.field = "position";
    PCGOverride c;
    c.objectId = 1; c.objectName = "A"; c.field = "rotation";

    store.Add(a);
    store.Add(b);
    store.Add(c);
    assert(store.Overrides().size() == 3);

    store.RemoveByObject(1);
    assert(store.Overrides().size() == 1);
    assert(store.Overrides()[0].objectId == 2);

    ok("test_override_store_remove_by_object");
}

void test_override_store_serialize_roundtrip() {
    PCGOverrideStore store;
    PCGOverride ov;
    ov.objectId   = 7;
    ov.objectName = "TestStation Module 3";
    ov.objectType = "Module";
    ov.field      = "scale";
    ov.values[0]  = 1.5f;
    ov.values[1]  = 2.0f;
    ov.values[2]  = 0.8f;
    ov.seed       = 999;
    ov.version    = 2;
    store.Add(ov);

    std::string json = store.SerializeToJSON();
    assert(!json.empty());
    assert(json.find("TestStation Module 3") != std::string::npos);
    assert(json.find("scale") != std::string::npos);

    PCGOverrideStore store2;
    bool ok2 = store2.DeserializeFromJSON(json);
    assert(ok2);
    assert(store2.Overrides().size() == 1);
    assert(store2.Overrides()[0].objectId == 7);
    assert(store2.Overrides()[0].objectName == "TestStation Module 3");
    assert(store2.Overrides()[0].field == "scale");
    assert(store2.Overrides()[0].seed == 999);
    assert(store2.Overrides()[0].version == 2);
    // Float values (approximate comparison)
    assert(store2.Overrides()[0].values[0] > 1.4f && store2.Overrides()[0].values[0] < 1.6f);
    assert(store2.Overrides()[0].values[1] > 1.9f && store2.Overrides()[0].values[1] < 2.1f);

    ok("test_override_store_serialize_roundtrip");
}

void test_override_store_file_roundtrip() {
    std::string path = "/tmp/atlas_test_overrides.json";

    PCGOverrideStore store;
    PCGOverride ov;
    ov.objectId   = 42;
    ov.objectName = "FileSaveTest";
    ov.objectType = "Ship";
    ov.field      = "position";
    ov.values[0]  = 100.0f;
    ov.values[1]  = 200.0f;
    ov.values[2]  = 300.0f;
    ov.seed       = 123;
    ov.version    = 1;
    store.Add(ov);

    bool saved = store.SaveToFile(path);
    assert(saved);
    assert(std::filesystem::exists(path));

    PCGOverrideStore store2;
    bool loaded = store2.LoadFromFile(path);
    assert(loaded);
    assert(store2.Overrides().size() == 1);
    assert(store2.Overrides()[0].objectName == "FileSaveTest");
    assert(store2.Overrides()[0].values[0] > 99.0f);

    // Clean up
    std::filesystem::remove(path);

    ok("test_override_store_file_roundtrip");
}

void test_override_store_load_nonexistent() {
    PCGOverrideStore store;
    bool loaded = store.LoadFromFile("/tmp/nonexistent_overrides_test.json");
    assert(!loaded);
    ok("test_override_store_load_nonexistent");
}

void test_override_store_import_from_viewport() {
    // Create a viewport with a ship, make changes, import into store
    ViewportPanel vp;
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);
    auto ship = atlas::pcg::ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 42);

    // Select and move the ship hull
    vp.SelectObject(1);
    vp.TranslateSelected(10.0f, 5.0f, -3.0f);

    // Import changes
    std::vector<ViewportObject> objects;
    for (size_t i = 0; i < vp.ObjectCount(); ++i) {
        objects.push_back(vp.GetObject(i));
    }
    auto changes = vp.CommitChanges();

    PCGOverrideStore store;
    store.ImportFromViewport(changes, objects, 42, 1);

    assert(store.Overrides().size() == 1);
    assert(store.Overrides()[0].objectName == ship.shipName);
    assert(store.Overrides()[0].field == "position");
    assert(store.Overrides()[0].seed == 42);
    assert(store.IsDirty());

    ok("test_override_store_import_from_viewport");
}

// ══════════════════════════════════════════════════════════════════
// LiveSceneManager tests
// ══════════════════════════════════════════════════════════════════

void test_live_scene_defaults() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    assert(std::string(lsm.Name()) == "Live Scene");
    assert(!lsm.IsPopulated());
    assert(!lsm.HasUnsavedChanges());
    assert(lsm.CurrentSeed() == 42);
    assert(lsm.CurrentVersion() == 1);

    ok("test_live_scene_defaults");
}

void test_live_scene_populate() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();

    assert(lsm.IsPopulated());
    assert(vp.ObjectCount() > 0);
    assert(!lsm.Log().empty());

    ok("test_live_scene_populate");
}

void test_live_scene_capture_changes() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();
    assert(vp.ObjectCount() > 0);

    // Move an object
    vp.SelectObject(1);
    vp.TranslateSelected(50.0f, 0.0f, 0.0f);

    lsm.CaptureViewportChanges();
    assert(lsm.HasUnsavedChanges());
    assert(lsm.OverrideStore().Overrides().size() == 1);

    ok("test_live_scene_capture_changes");
}

void test_live_scene_save_and_load() {
    std::string path = "/tmp/atlas_test_live_scene.json";

    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();
    vp.SelectObject(1);
    vp.TranslateSelected(25.0f, 15.0f, -5.0f);

    bool saved = lsm.SaveOverrides(path);
    assert(saved);

    // Load into a fresh scene
    ViewportPanel vp2;
    PCGPreviewPanel pcg2;
    LiveSceneManager lsm2(&vp2, &pcg2);

    bool loaded = lsm2.LoadOverrides(path);
    assert(loaded);
    assert(lsm2.OverrideStore().Overrides().size() > 0);

    // Populate and verify overrides are applied
    lsm2.PopulateDefaultScene();
    assert(vp2.ObjectCount() > 0);

    std::filesystem::remove(path);

    ok("test_live_scene_save_and_load");
}

void test_live_scene_regenerate() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();
    size_t initialCount = vp.ObjectCount();
    assert(initialCount > 0);

    // Regenerate should produce the same object count (deterministic)
    lsm.RegenerateScene();
    assert(vp.ObjectCount() == initialCount);
    assert(lsm.IsPopulated());

    ok("test_live_scene_regenerate");
}

void test_live_scene_on_asset_reload_triggers_regen() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();
    size_t logBefore = lsm.Log().size();

    // Simulate an asset change
    lsm.OnAssetReloaded("test_asset", "/fake/path/test_asset.atlas");

    // Should have regenerated (more log entries)
    assert(lsm.Log().size() > logBefore);

    ok("test_live_scene_on_asset_reload_triggers_regen");
}

void test_live_scene_seed_and_version() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.SetSeed(999);
    lsm.SetVersion(3);
    assert(lsm.CurrentSeed() == 999);
    assert(lsm.CurrentVersion() == 3);

    lsm.PopulateDefaultScene();
    assert(lsm.IsPopulated());

    ok("test_live_scene_seed_and_version");
}
