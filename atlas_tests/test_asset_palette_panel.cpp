/**
 * Tests for AssetPalettePanel:
 *   - Construction & defaults
 *   - Asset CRUD
 *   - Category & search filtering
 *   - Selection
 *   - Prefab support
 *   - JSON export/import round-trip
 */

#include <cassert>
#include <string>
#include "../editor/tools/AssetPalettePanel.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// AssetPalettePanel tests
// ══════════════════════════════════════════════════════════════════

void test_asset_palette_defaults() {
    AssetPalettePanel panel;
    assert(panel.AssetCount() == 0);
    assert(panel.SelectedAsset() == -1);
    assert(panel.CategoryFilter().empty());
    assert(panel.SearchFilter().empty());
    assert(panel.PrefabCount() == 0);
    assert(std::string(panel.Name()) == "Asset Palette");
}

void test_asset_palette_categories_count() {
    assert(AssetPalettePanel::kCategoryCount == 7);
    assert(std::string(AssetPalettePanel::kCategories[0]) == "ship");
    assert(std::string(AssetPalettePanel::kCategories[6]) == "environment");
}

void test_asset_palette_add_asset() {
    AssetPalettePanel panel;

    AssetEntry entry;
    entry.assetId     = "fang_frigate";
    entry.name        = "Fang Frigate";
    entry.category    = "ship";
    entry.subcategory = "frigate";
    entry.tags        = "combat,fast";

    size_t idx = panel.AddAsset(entry);
    assert(idx == 0);
    assert(panel.AssetCount() == 1);
    assert(panel.GetAsset(0).assetId == "fang_frigate");
    assert(panel.GetAsset(0).name == "Fang Frigate");
    assert(panel.GetAsset(0).category == "ship");
}

void test_asset_palette_remove_asset() {
    AssetPalettePanel panel;

    AssetEntry e1; e1.assetId = "a"; e1.name = "Alpha";
    AssetEntry e2; e2.assetId = "b"; e2.name = "Beta";
    panel.AddAsset(e1);
    panel.AddAsset(e2);
    assert(panel.AssetCount() == 2);

    bool removed = panel.RemoveAsset(0);
    assert(removed);
    assert(panel.AssetCount() == 1);
    assert(panel.GetAsset(0).assetId == "b");

    // Out-of-bounds removal fails
    assert(!panel.RemoveAsset(99));
}

void test_asset_palette_selection() {
    AssetPalettePanel panel;

    AssetEntry e1; e1.assetId = "a"; e1.name = "Alpha";
    AssetEntry e2; e2.assetId = "b"; e2.name = "Beta";
    panel.AddAsset(e1);
    panel.AddAsset(e2);

    panel.SelectAsset(1);
    assert(panel.SelectedAsset() == 1);

    panel.ClearSelection();
    assert(panel.SelectedAsset() == -1);

    // Out-of-bounds selection is ignored
    panel.SelectAsset(99);
    assert(panel.SelectedAsset() == -1);
}

void test_asset_palette_remove_adjusts_selection() {
    AssetPalettePanel panel;

    AssetEntry e1; e1.assetId = "a"; e1.name = "A";
    AssetEntry e2; e2.assetId = "b"; e2.name = "B";
    AssetEntry e3; e3.assetId = "c"; e3.name = "C";
    panel.AddAsset(e1);
    panel.AddAsset(e2);
    panel.AddAsset(e3);

    panel.SelectAsset(2);
    assert(panel.SelectedAsset() == 2);

    // Remove item before selection — selection should shift down
    panel.RemoveAsset(0);
    assert(panel.SelectedAsset() == 1);
}

void test_asset_palette_category_filter() {
    AssetPalettePanel panel;

    AssetEntry ship; ship.assetId = "s1"; ship.name = "Ship"; ship.category = "ship";
    AssetEntry prop; prop.assetId = "p1"; prop.name = "Crate"; prop.category = "prop";
    AssetEntry mod;  mod.assetId  = "m1"; mod.name  = "Laser"; mod.category  = "module";
    panel.AddAsset(ship);
    panel.AddAsset(prop);
    panel.AddAsset(mod);

    assert(panel.FilteredCount() == 3); // no filter

    panel.SetCategoryFilter("ship");
    assert(panel.FilteredCount() == 1);

    panel.SetCategoryFilter("prop");
    assert(panel.FilteredCount() == 1);

    panel.SetCategoryFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_asset_palette_search_filter() {
    AssetPalettePanel panel;

    AssetEntry e1;
    e1.assetId = "s1"; e1.name = "Fang Frigate";
    e1.category = "ship"; e1.subcategory = "frigate"; e1.tags = "combat,fast";
    panel.AddAsset(e1);

    AssetEntry e2;
    e2.assetId = "p1"; e2.name = "Metal Crate";
    e2.category = "prop"; e2.subcategory = "cargo"; e2.tags = "storage";
    panel.AddAsset(e2);

    // Search by name (case-insensitive)
    panel.SetSearchFilter("fang");
    assert(panel.FilteredCount() == 1);

    // Search by tag
    panel.SetSearchFilter("storage");
    assert(panel.FilteredCount() == 1);

    // Search by subcategory
    panel.SetSearchFilter("frigate");
    assert(panel.FilteredCount() == 1);

    // No match
    panel.SetSearchFilter("xyz");
    assert(panel.FilteredCount() == 0);

    panel.SetSearchFilter("");
    assert(panel.FilteredCount() == 2);
}

void test_asset_palette_combined_filters() {
    AssetPalettePanel panel;

    AssetEntry e1;
    e1.assetId = "s1"; e1.name = "Fang Frigate";
    e1.category = "ship"; e1.tags = "combat";
    panel.AddAsset(e1);

    AssetEntry e2;
    e2.assetId = "s2"; e2.name = "Viper Destroyer";
    e2.category = "ship"; e2.tags = "combat,heavy";
    panel.AddAsset(e2);

    AssetEntry e3;
    e3.assetId = "p1"; e3.name = "Fuel Barrel";
    e3.category = "prop"; e3.tags = "fuel";
    panel.AddAsset(e3);

    // Category + search combined
    panel.SetCategoryFilter("ship");
    panel.SetSearchFilter("combat");
    assert(panel.FilteredCount() == 2);

    panel.SetSearchFilter("viper");
    assert(panel.FilteredCount() == 1);

    panel.SetCategoryFilter("prop");
    panel.SetSearchFilter("viper");
    assert(panel.FilteredCount() == 0);
}

void test_asset_palette_prefab() {
    AssetPalettePanel panel;

    AssetEntry e1; e1.assetId = "s1"; e1.name = "Ship";
    AssetEntry e2; e2.assetId = "p1"; e2.name = "Prop";
    panel.AddAsset(e1);
    panel.AddAsset(e2);

    assert(panel.PrefabCount() == 0);
    assert(!panel.GetAsset(0).isPrefab);

    bool ok = panel.SaveAsPrefab(0);
    assert(ok);
    assert(panel.GetAsset(0).isPrefab);
    assert(panel.PrefabCount() == 1);

    // Out of bounds
    assert(!panel.SaveAsPrefab(99));
}

void test_asset_palette_export_import_json() {
    AssetPalettePanel panel;

    AssetEntry e1;
    e1.assetId = "s1"; e1.name = "Fang";
    e1.category = "ship"; e1.subcategory = "frigate";
    e1.tags = "combat"; e1.isPrefab = true; e1.previewScale = 2.0f;
    panel.AddAsset(e1);

    AssetEntry e2;
    e2.assetId = "p1"; e2.name = "Crate";
    e2.category = "prop"; e2.subcategory = "cargo";
    e2.tags = "storage"; e2.isPrefab = false; e2.previewScale = 0.5f;
    panel.AddAsset(e2);

    std::string json = panel.ExportToJson();
    assert(!json.empty());
    assert(json.find("assetPalette") != std::string::npos);
    assert(json.find("Fang") != std::string::npos);
    assert(json.find("Crate") != std::string::npos);

    // Import into fresh panel
    AssetPalettePanel panel2;
    size_t count = panel2.ImportFromJson(json);
    assert(count == 2);
    assert(panel2.AssetCount() == 2);
    assert(panel2.GetAsset(0).assetId == "s1");
    assert(panel2.GetAsset(0).name == "Fang");
    assert(panel2.GetAsset(0).isPrefab == true);
    assert(panel2.GetAsset(1).assetId == "p1");
    assert(panel2.GetAsset(1).name == "Crate");
}
