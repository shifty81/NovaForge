#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/pcg_asset_style.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Editor panel for modifying PCG asset shapes and visual styles.
 *
 * Designers use this panel to:
 *   1. Define shape profiles (control points that deform geometry)
 *   2. Create style palettes (colors, materials, surface treatments)
 *   3. Preview the effect on generated assets in real-time
 *   4. Save asset styles to the library for reuse
 *
 * This panel works in tandem with the GenerationStylePanel — designers
 * create their generation blueprints there, then attach asset styles
 * authored here to control the look and shape of the output.
 */
class AssetStylePanel : public EditorPanel {
public:
    AssetStylePanel();

    const char* Name() const override { return "Asset Style"; }
    void Draw() override;

    // ── Style management ────────────────────────────────────────────

    /// Start editing a new asset style for the given target type.
    void NewStyle(const std::string& name,
                  pcg::GenerationStyleType targetType);

    /// Get the current style being edited.
    const pcg::AssetStyle& GetCurrentStyle() const { return m_currentStyle; }

    /// Replace the current style wholesale.
    void SetCurrentStyle(const pcg::AssetStyle& style);

    // ── Shape editing ───────────────────────────────────────────────

    /// Create a new empty shape profile.
    void NewShapeProfile(const std::string& name);

    /// Add a control point to the current shape profile.
    void AddControlPoint(const pcg::ShapeControlPoint& point);

    /// Remove a control point by index.
    bool RemoveControlPoint(size_t index);

    /// Update an existing control point by index.
    bool UpdateControlPoint(size_t index,
                            const pcg::ShapeControlPoint& point);

    /// Set symmetry mirroring on the shape profile.
    void SetMirror(bool mirrorX, bool mirrorY);

    /// Set smoothing factor for shape interpolation.
    void SetSmoothing(float smoothing);

    /// Number of control points in the current shape.
    size_t ControlPointCount() const {
        return m_currentStyle.shape.controlPoints.size();
    }

    // ── Palette editing ─────────────────────────────────────────────

    /// Create a new empty style palette.
    void NewPalette(const std::string& name);

    /// Add a color to the palette.
    void AddColor(const pcg::StyleColor& color);

    /// Update an existing color by index.
    bool SetColor(size_t index, float r, float g, float b, float a);

    /// Remove a color by index.
    bool RemoveColor(size_t index);

    /// Add a material to the palette.
    void AddMaterial(const pcg::StyleMaterial& material);

    /// Update an existing material by index.
    bool SetMaterial(size_t index, const pcg::StyleMaterial& material);

    /// Remove a material by index.
    bool RemoveMaterial(size_t index);

    /// Set the surface treatment.
    void SetSurfaceTreatment(pcg::SurfaceTreatment treatment);

    /// Set the detail level for surface decorations.
    void SetDetailLevel(float level);

    /// Number of colors in the palette.
    size_t ColorCount() const {
        return m_currentStyle.palette.colors.size();
    }

    /// Number of materials in the palette.
    size_t MaterialCount() const {
        return m_currentStyle.palette.materials.size();
    }

    // ── Preview ─────────────────────────────────────────────────────

    /// Generate a preview asset and apply the current style to it.
    void ApplyAndPreview();

    /// Get the preview ship (populated after ApplyAndPreview for ship styles).
    const pcg::GeneratedShip& GetPreviewShip() const {
        return m_previewShip;
    }

    /// Get the preview station.
    const pcg::GeneratedStation& GetPreviewStation() const {
        return m_previewStation;
    }

    /// Whether a preview has been generated.
    bool HasPreview() const { return m_hasPreview; }

    // ── Library ─────────────────────────────────────────────────────

    /// Save the current style to the in-memory library.
    void SaveToLibrary();

    /// Load a style from the library by name.
    bool LoadFromLibrary(const std::string& name);

    /// Get the library.
    const pcg::AssetStyleLibrary& GetLibrary() const { return m_library; }
    pcg::AssetStyleLibrary&       GetLibrary()       { return m_library; }

    // ── Serialisation ───────────────────────────────────────────────

    std::string SaveToString() const;
    void LoadFromString(const std::string& data);

    // ── Log ─────────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    pcg::AssetStyle        m_currentStyle;
    pcg::AssetStyleLibrary m_library;
    pcg::PCGManager        m_pcgManager;

    pcg::GeneratedShip     m_previewShip;
    pcg::GeneratedStation  m_previewStation;
    bool                   m_hasPreview = false;

    std::vector<std::string> m_log;

    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
};

}
