/**
 * Tests for ModelImportPanel:
 *   - Construction & defaults
 *   - Model CRUD (add/remove)
 *   - Category & search filtering
 *   - Selection
 *   - Category/tag updates
 *   - Validation
 *   - JSON export/import round-trip
 *   - Statistics
 *   - Directory scanning
 */

#include <cassert>
#include <string>
#include <fstream>
#include <filesystem>
#include "../editor/tools/ModelImportPanel.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// ModelImportPanel tests
// ══════════════════════════════════════════════════════════════════

void test_model_import_defaults() {
    ModelImportPanel panel;
    assert(panel.ModelCount() == 0);
    assert(panel.SelectedIndex() == -1);
    assert(panel.CategoryFilter().empty());
    assert(panel.SearchFilter().empty());
    assert(panel.TotalVertices() == 0);
    assert(panel.TotalFaces() == 0);
    assert(panel.ValidatedCount() == 0);
    assert(std::string(panel.Name()) == "Model Import");
}

void test_model_import_categories() {
    assert(ModelImportPanel::kCategoryCount == 7);
    assert(std::string(ModelImportPanel::kCategories[0]) == "hull");
    assert(std::string(ModelImportPanel::kCategories[1]) == "turret");
    assert(std::string(ModelImportPanel::kCategories[6]) == "prop");
}

void test_model_import_add() {
    ModelImportPanel panel;

    ModelCatalogEntry entry;
    entry.modelId     = "turret_m";
    entry.filename    = "turret_m.obj";
    entry.category    = "turret";
    entry.format      = "obj";
    entry.vertexCount = 120;
    entry.faceCount   = 80;
    entry.fileSize    = 4096;
    entry.validated   = true;

    assert(panel.AddModel(entry));
    assert(panel.ModelCount() == 1);
    assert(panel.GetModel("turret_m") != nullptr);
    assert(panel.GetModel("turret_m")->filename == "turret_m.obj");
    assert(panel.GetModel("turret_m")->category == "turret");
    assert(panel.TotalVertices() == 120);
    assert(panel.TotalFaces() == 80);
}

void test_model_import_add_duplicate() {
    ModelImportPanel panel;

    ModelCatalogEntry e1;
    e1.modelId = "turret_m";
    e1.filename = "turret_m.obj";
    panel.AddModel(e1);

    // Duplicate rejected
    ModelCatalogEntry e2;
    e2.modelId = "turret_m";
    e2.filename = "turret_m_v2.obj";
    assert(!panel.AddModel(e2));
    assert(panel.ModelCount() == 1);

    // Empty ID rejected
    ModelCatalogEntry e3;
    e3.modelId = "";
    assert(!panel.AddModel(e3));
}

void test_model_import_remove() {
    ModelImportPanel panel;

    ModelCatalogEntry e1; e1.modelId = "turret_m"; e1.filename = "turret_m.obj";
    ModelCatalogEntry e2; e2.modelId = "engine_s"; e2.filename = "engine_s.obj";
    panel.AddModel(e1);
    panel.AddModel(e2);

    assert(panel.RemoveModel("turret_m"));
    assert(panel.ModelCount() == 1);
    assert(panel.GetModel("turret_m") == nullptr);
    assert(panel.GetModel("engine_s") != nullptr);

    // Can't remove twice
    assert(!panel.RemoveModel("turret_m"));
    // Can't remove nonexistent
    assert(!panel.RemoveModel("nonexistent"));
}

void test_model_import_selection() {
    ModelImportPanel panel;

    ModelCatalogEntry e1; e1.modelId = "a"; e1.filename = "a.obj";
    ModelCatalogEntry e2; e2.modelId = "b"; e2.filename = "b.obj";
    panel.AddModel(e1);
    panel.AddModel(e2);

    panel.SelectModel(0);
    assert(panel.SelectedIndex() == 0);

    panel.SelectModel(1);
    assert(panel.SelectedIndex() == 1);

    // Out of range ignored
    panel.SelectModel(99);
    assert(panel.SelectedIndex() == 1);  // unchanged

    panel.ClearSelection();
    assert(panel.SelectedIndex() == -1);
}

void test_model_import_category_filter() {
    ModelImportPanel panel;

    ModelCatalogEntry e1; e1.modelId = "turret_m"; e1.category = "turret";
    ModelCatalogEntry e2; e2.modelId = "engine_s"; e2.category = "engine";
    ModelCatalogEntry e3; e3.modelId = "core_m";   e3.category = "core";
    panel.AddModel(e1);
    panel.AddModel(e2);
    panel.AddModel(e3);

    panel.SetCategoryFilter("turret");
    assert(panel.FilteredCount() == 1);

    panel.SetCategoryFilter("engine");
    assert(panel.FilteredCount() == 1);

    panel.SetCategoryFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_model_import_search_filter() {
    ModelImportPanel panel;

    ModelCatalogEntry e1; e1.modelId = "turret_m"; e1.tags = "combat,medium";
    ModelCatalogEntry e2; e2.modelId = "engine_s"; e2.tags = "propulsion,small";
    panel.AddModel(e1);
    panel.AddModel(e2);

    panel.SetSearchFilter("turret");
    assert(panel.FilteredCount() == 1);

    panel.SetSearchFilter("combat");
    assert(panel.FilteredCount() == 1);

    panel.SetSearchFilter("TURRET");  // case-insensitive
    assert(panel.FilteredCount() == 1);

    panel.SetSearchFilter("");
    assert(panel.FilteredCount() == 2);
}

void test_model_import_set_category() {
    ModelImportPanel panel;

    ModelCatalogEntry e1; e1.modelId = "turret_m"; e1.category = "turret";
    panel.AddModel(e1);

    assert(panel.SetCategory("turret_m", "weapon"));
    assert(panel.GetModel("turret_m")->category == "weapon");

    assert(!panel.SetCategory("nonexistent", "hull"));
}

void test_model_import_set_tags() {
    ModelImportPanel panel;

    ModelCatalogEntry e1; e1.modelId = "turret_m"; e1.tags = "";
    panel.AddModel(e1);

    assert(panel.SetTags("turret_m", "combat,heavy,railgun"));
    assert(panel.GetModel("turret_m")->tags == "combat,heavy,railgun");

    assert(!panel.SetTags("nonexistent", "test"));
}

void test_model_import_validate() {
    ModelImportPanel panel;

    ModelCatalogEntry e1;
    e1.modelId = "turret_m";
    e1.filename = "turret_m.obj";
    e1.fileSize = 4096;
    panel.AddModel(e1);

    ModelCatalogEntry e2;
    e2.modelId = "empty";
    e2.filename = "";
    e2.fileSize = 0;
    panel.AddModel(e2);

    assert(panel.ValidateModel("turret_m"));
    assert(!panel.ValidateModel("empty"));

    int valid = panel.ValidateAll();
    assert(valid == 1);
    assert(panel.ValidatedCount() == 1);
}

void test_model_import_json_roundtrip() {
    ModelImportPanel panel;

    ModelCatalogEntry e1;
    e1.modelId = "turret_m";
    e1.filename = "turret_m.obj";
    e1.category = "turret";
    e1.format = "obj";
    e1.tags = "combat,medium";
    e1.vertexCount = 120;
    e1.faceCount = 80;
    e1.fileSize = 4096;
    e1.validated = true;
    panel.AddModel(e1);

    ModelCatalogEntry e2;
    e2.modelId = "engine_s";
    e2.filename = "engine_s.obj";
    e2.category = "engine";
    e2.format = "obj";
    e2.vertexCount = 60;
    e2.faceCount = 40;
    e2.fileSize = 2048;
    panel.AddModel(e2);

    std::string json = panel.ExportToJson();
    assert(!json.empty());
    assert(json.find("turret_m") != std::string::npos);
    assert(json.find("engine_s") != std::string::npos);

    // Import into a fresh panel
    ModelImportPanel panel2;
    int imported = panel2.ImportFromJson(json);
    assert(imported == 2);
    assert(panel2.ModelCount() == 2);
    assert(panel2.GetModel("turret_m") != nullptr);
    assert(panel2.GetModel("engine_s") != nullptr);
    assert(panel2.GetModel("turret_m")->category == "turret");
    assert(panel2.GetModel("turret_m")->vertexCount == 120);
}

void test_model_import_scan_directory() {
    ModelImportPanel panel;

    // Create temp directory with test OBJ files
    namespace fs = std::filesystem;
    std::string tmpDir = "/tmp/novaforge_test_models";
    fs::create_directories(tmpDir);

    // Write minimal OBJ files
    {
        std::ofstream f(tmpDir + "/test_hull.obj");
        f << "# test hull\n";
        f << "v 0 0 0\nv 1 0 0\nv 0 1 0\n";
        f << "f 1 2 3\n";
    }
    {
        std::ofstream f(tmpDir + "/test_turret.obj");
        f << "# test turret\n";
        f << "v 0 0 0\nv 1 0 0\n";
        f << "f 1 2 1\n";
    }

    int found = panel.ScanDirectory(tmpDir);
    assert(found == 2);
    assert(panel.ModelCount() == 2);

    // Check auto-categorisation
    const auto* hull = panel.GetModel("test_hull");
    const auto* turret = panel.GetModel("test_turret");
    assert(hull != nullptr);
    assert(turret != nullptr);
    assert(hull->vertexCount == 3);
    assert(hull->faceCount == 1);
    assert(turret->category == "turret");  // auto-detected from name

    // Re-scan doesn't duplicate
    int found2 = panel.ScanDirectory(tmpDir);
    assert(found2 == 0);
    assert(panel.ModelCount() == 2);

    // Nonexistent directory returns 0
    assert(panel.ScanDirectory("/tmp/nonexistent_dir_xyz") == 0);

    // Cleanup
    fs::remove_all(tmpDir);
}

void test_model_import_statistics() {
    ModelImportPanel panel;

    ModelCatalogEntry e1;
    e1.modelId = "a"; e1.vertexCount = 100; e1.faceCount = 50;
    e1.filename = "a.obj"; e1.fileSize = 1024; e1.validated = true;
    panel.AddModel(e1);

    ModelCatalogEntry e2;
    e2.modelId = "b"; e2.vertexCount = 200; e2.faceCount = 100;
    e2.filename = "b.obj"; e2.fileSize = 2048; e2.validated = true;
    panel.AddModel(e2);

    assert(panel.TotalVertices() == 300);
    assert(panel.TotalFaces() == 150);
    assert(panel.ValidatedCount() == 2);
}

void test_model_import_headless_draw() {
    ModelImportPanel panel;
    // Draw without context should not crash
    panel.Draw();
    assert(panel.ModelCount() == 0);  // no side effects
}
