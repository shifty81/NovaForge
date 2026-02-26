#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/generation_style.h"
#include "../../cpp_server/include/pcg/pcg_asset_style.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Editor panel for creating and editing PCG generation styles.
 *
 * Gives designers a visual workflow to author generation blueprints:
 *   1. Choose a style type (Ship, Station, Interior, …)
 *   2. Add placement entries (pin specific content at positions)
 *   3. Tune parameter overrides (knobs the generator respects)
 *   4. Optionally attach an AssetStyle for shape/visual modification
 *   5. Preview the generated result
 *   6. Save / load styles for reuse
 *
 * The panel owns a PCGManager so it can generate previews in isolation.
 */
class GenerationStylePanel : public EditorPanel {
public:
    GenerationStylePanel();

    const char* Name() const override { return "Generation Style"; }
    void Draw() override;

    // ── Style management ────────────────────────────────────────────

    /// Create a new blank style of the given type.
    void NewStyle(pcg::GenerationStyleType type,
                  const std::string& name = "");

    /// Get the current style being edited.
    const pcg::GenerationStyle& GetStyle() const { return m_style; }

    /// Replace the current style wholesale.
    void SetStyle(const pcg::GenerationStyle& style) { m_style = style; }

    // ── Placement editing ───────────────────────────────────────────

    /// Add a designer placement.
    void AddPlacement(const pcg::PlacementEntry& entry);

    /// Remove the placement at the given slot index.
    bool RemovePlacement(uint32_t slotIndex);

    /// Number of current placements.
    size_t PlacementCount() const { return m_style.placements.size(); }

    // ── Parameter editing ───────────────────────────────────────────

    /// Set an existing parameter's value (by name).
    bool SetParameter(const std::string& name, float value);

    /// Enable or disable a parameter override.
    bool EnableParameter(const std::string& name, bool enabled);

    // ── Asset style attachment ───────────────────────────────────────

    /// Attach an asset style to apply after generation.
    void AttachAssetStyle(const pcg::AssetStyle& style);

    /// Detach any attached asset style.
    void DetachAssetStyle();

    /// Whether an asset style is currently attached.
    bool HasAssetStyle() const { return m_hasAssetStyle; }

    /// Get the attached asset style.
    const pcg::AssetStyle& GetAttachedAssetStyle() const {
        return m_assetStyle;
    }

    // ── Generation ──────────────────────────────────────────────────

    /// Validate, generate, and optionally apply asset style.
    void Generate();

    /// Get the last generation result.
    const pcg::StyleGenerationResult& GetResult() const { return m_result; }

    // ── Serialisation ───────────────────────────────────────────────

    /// Serialise the current style to a string.
    std::string SaveStyleToString() const;

    /// Load a style from a serialised string.
    void LoadStyleFromString(const std::string& data);

    /// Save the current style to a file.
    bool SaveStyleToFile(const std::string& path = "data/generation_style.json");

    /// Load a style from a file.
    bool LoadStyleFromFile(const std::string& path = "data/generation_style.json");

    // ── Log ─────────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    pcg::GenerationStyle         m_style;
    pcg::AssetStyle              m_assetStyle;
    bool                         m_hasAssetStyle = false;
    pcg::PCGManager              m_pcgManager;
    pcg::StyleGenerationResult   m_result;
    std::vector<std::string>     m_log;

    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
};

}
