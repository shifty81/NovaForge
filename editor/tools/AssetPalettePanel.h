#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Represents one browsable asset in the palette.
 *
 * Assets are organised by category and subcategory.  A comma-separated
 * @c tags string provides additional free-form metadata for search.
 */
struct AssetEntry {
    std::string assetId;
    std::string name;
    std::string category;       // "ship", "module", "prop", etc.
    std::string subcategory;    // e.g. "frigate", "weapon", "crate", "asteroid"
    std::string tags;           // comma-separated tags for filtering
    bool  isPrefab     = false;
    float previewScale = 1.0f;
};

/**
 * @brief AssetPalettePanel — categorised library of reusable asset prefabs.
 *
 * Lets designers browse, search, and place assets from a categorised
 * library.  Assets can be promoted to user-saved prefabs and the full
 * palette can be round-tripped through JSON.
 *
 * Usage workflow:
 *   1. AddAsset() to populate the palette
 *   2. SetCategoryFilter() / SetSearchFilter() to narrow the view
 *   3. Select an entry to preview / place it
 *   4. SaveAsPrefab() to mark an entry as a user prefab
 *   5. ExportToJson() / ImportFromJson() for serialisation
 */
class AssetPalettePanel : public EditorPanel {
public:
    AssetPalettePanel();
    ~AssetPalettePanel() override = default;

    const char* Name() const override { return "Asset Palette"; }
    void Draw() override;

    // ── Category constants ───────────────────────────────────────

    static constexpr const char* kCategories[] = {
        "ship", "module", "prop", "station",
        "character", "rig", "environment"
    };
    static constexpr int kCategoryCount = 7;

    // ── Asset management ─────────────────────────────────────────

    size_t AddAsset(const AssetEntry& entry);
    bool   RemoveAsset(size_t index);
    size_t AssetCount() const { return m_assets.size(); }

    const AssetEntry& GetAsset(size_t index) const { return m_assets[index]; }
    const std::vector<AssetEntry>& Assets() const { return m_assets; }

    // ── Category filter ──────────────────────────────────────────

    void SetCategoryFilter(const std::string& category);
    const std::string& CategoryFilter() const { return m_categoryFilter; }
    size_t FilteredCount() const;

    // ── Tag / name search ────────────────────────────────────────

    void SetSearchFilter(const std::string& search);
    const std::string& SearchFilter() const { return m_searchFilter; }

    // ── Selection ────────────────────────────────────────────────

    void SelectAsset(int index);
    void ClearSelection();
    int  SelectedAsset() const { return m_selectedIndex; }

    // ── Prefab support ───────────────────────────────────────────

    bool   SaveAsPrefab(size_t index);
    size_t PrefabCount() const;

    // ── Export / Import ──────────────────────────────────────────

    std::string ExportToJson() const;
    size_t      ImportFromJson(const std::string& json);

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<AssetEntry> m_assets;
    int m_selectedIndex = -1;
    std::string m_categoryFilter;
    std::string m_searchFilter;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    bool matchesFilters(const AssetEntry& entry) const;
};

} // namespace atlas::editor
