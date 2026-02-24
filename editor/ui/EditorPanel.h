#pragma once
#include <string>

namespace atlas::editor {

class EditorPanel {
public:
    virtual ~EditorPanel() = default;
    virtual const char* Name() const = 0;
    virtual void Draw() = 0;
    virtual bool IsVisible() const { return m_visible; }
    virtual void SetVisible(bool visible) { m_visible = visible; }

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

private:
    bool m_visible = true;
};

}
