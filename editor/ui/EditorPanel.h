#pragma once
#include <string>
#include "../../cpp_client/include/ui/atlas/atlas_types.h"

namespace atlas { class AtlasContext; }

namespace atlas::editor {

class EditorPanel {
public:
    virtual ~EditorPanel() = default;
    virtual const char* Name() const = 0;
    virtual void Draw() = 0;
    virtual bool IsVisible() const { return m_visible; }
    virtual void SetVisible(bool visible) { m_visible = visible; }

    /** Set the UI context used for rendering.  Null in headless/test mode. */
    void SetContext(atlas::AtlasContext* ctx) { m_ctx = ctx; }

    /** Get the UI context (may be null). */
    atlas::AtlasContext* GetContext() const { return m_ctx; }

    // ── Dock layout bounds ──────────────────────────────────────────

    /** Called by EditorLayout to assign the panel's render area based on
     *  the dock tree split ratios.  Panels should not be dragged/resized
     *  individually when docked. */
    void SetDockBounds(const atlas::Rect& bounds) {
        m_dockBounds = bounds;
        m_hasDockBounds = true;
    }

    /** Returns the dock-assigned bounds. */
    const atlas::Rect& DockBounds() const { return m_dockBounds; }

    /** True when the layout engine has assigned bounds to this panel. */
    bool HasDockBounds() const { return m_hasDockBounds; }

    /**
     * Called by the editor when a watched asset or data file changes on disk.
     * Panels that cache generated content (e.g. PCG Preview, Viewport) should
     * override this to regenerate / refresh their preview.  The default
     * implementation is a no-op so existing panels are unaffected.
     *
     * @param assetId  Identifier of the changed asset (stem of the file name).
     * @param path     Full filesystem path to the changed file.
     */
    virtual void OnAssetReloaded(const std::string& /*assetId*/,
                                 const std::string& /*path*/) {}

protected:
    /** Copy dock bounds into a PanelState and lock it so the panel
     *  renders in its dock-assigned region instead of at (0,0). */
    void ApplyDockBounds(atlas::PanelState& state) const {
        if (m_hasDockBounds) {
            state.bounds = m_dockBounds;
            state.locked = true;
        }
    }

private:
    bool m_visible = true;
    atlas::AtlasContext* m_ctx = nullptr;
    atlas::Rect m_dockBounds;
    bool m_hasDockBounds = false;
};

}
