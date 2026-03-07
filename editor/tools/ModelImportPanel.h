#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief Represents one reference 3D model in the import catalogue.
 *
 * The PCG ship/station generators use OBJ seed meshes from the
 * reference_models directory.  This entry tracks metadata for each
 * model so designers can browse, categorise, and manage them without
 * leaving the editor.
 */
struct ModelCatalogEntry {
    std::string modelId;         ///< Unique identifier (e.g. "turret_m")
    std::string filename;        ///< File name (e.g. "turret_m.obj")
    std::string category;        ///< "hull", "turret", "engine", "core", "wing", "weapon", "prop"
    std::string format;          ///< "obj", "gltf", "glb"
    std::string tags;            ///< Comma-separated free-form tags
    int vertexCount  = 0;        ///< Approximate vertex count (from header scan)
    int faceCount    = 0;        ///< Approximate face count
    int64_t fileSize = 0;        ///< File size in bytes
    bool validated   = false;    ///< True if file exists and header is parseable
};

/**
 * ModelImportPanel — Reference 3D model catalogue for the PCG pipeline.
 *
 * Designers can:
 *   - Scan the reference_models directory for OBJ/GLTF files.
 *   - Browse all discovered models with metadata (verts, faces, size).
 *   - Categorise each model (hull, turret, engine, core, wing, weapon, prop).
 *   - Add new model entries (will be picked up by the PCG generators).
 *   - Remove model entries from the catalogue.
 *   - Filter by category or search by name/tags.
 *   - Validate that model files exist and parse correctly.
 *   - Export the catalogue to JSON for version control.
 *
 * The catalogue is the bridge between raw 3D model files on disk and the
 * procedural generation system that assembles ships and stations from them.
 *
 * Headless-safe: Draw() is a no-op when no AtlasContext is set.
 */
class ModelImportPanel : public EditorPanel {
public:
    ModelImportPanel();
    ~ModelImportPanel() override = default;

    const char* Name() const override { return "Model Import"; }
    void Draw() override;

    // ── Category constants ───────────────────────────────────────
    static constexpr const char* kCategories[] = {
        "hull", "turret", "engine", "core", "wing", "weapon", "prop"
    };
    static constexpr int kCategoryCount = 7;

    // ── Scanning ─────────────────────────────────────────────────

    /** Scan a directory for OBJ/GLTF files and populate the catalogue. */
    int ScanDirectory(const std::string& directory);

    // ── Catalogue management ─────────────────────────────────────

    bool AddModel(const ModelCatalogEntry& entry);
    bool RemoveModel(const std::string& modelId);
    int  ModelCount() const { return static_cast<int>(m_models.size()); }

    const ModelCatalogEntry* GetModel(const std::string& modelId) const;
    const std::vector<ModelCatalogEntry>& Models() const { return m_models; }

    // ── Category update ──────────────────────────────────────────

    bool SetCategory(const std::string& modelId, const std::string& category);
    bool SetTags(const std::string& modelId, const std::string& tags);

    // ── Filtering ────────────────────────────────────────────────

    void SetCategoryFilter(const std::string& category);
    const std::string& CategoryFilter() const { return m_categoryFilter; }

    void SetSearchFilter(const std::string& search);
    const std::string& SearchFilter() const { return m_searchFilter; }

    int FilteredCount() const;

    // ── Selection ────────────────────────────────────────────────

    void SelectModel(int index);
    void ClearSelection();
    int  SelectedIndex() const { return m_selectedIndex; }

    // ── Validation ───────────────────────────────────────────────

    /** Validate that the file for the given model exists and is parseable. */
    bool ValidateModel(const std::string& modelId);

    /** Validate all models in the catalogue. Returns count of valid models. */
    int ValidateAll();

    // ── Export / Import ──────────────────────────────────────────

    std::string ExportToJson() const;
    int ImportFromJson(const std::string& json);

    // ── Statistics ────────────────────────────────────────────────

    int TotalVertices() const;
    int TotalFaces() const;
    int ValidatedCount() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<ModelCatalogEntry> m_models;
    int m_selectedIndex = -1;
    std::string m_categoryFilter;
    std::string m_searchFilter;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    bool matchesFilters(const ModelCatalogEntry& entry) const;
    ModelCatalogEntry* findModel(const std::string& modelId);
    const ModelCatalogEntry* findModel(const std::string& modelId) const;
};

} // namespace atlas::editor
