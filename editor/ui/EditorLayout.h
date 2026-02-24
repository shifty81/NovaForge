#pragma once
#include "DockNode.h"
#include <vector>
#include <memory>
#include <string>

namespace atlas { class AtlasContext; }

namespace atlas::editor {

class EditorLayout {
public:
    void RegisterPanel(EditorPanel* panel);
    void Draw();

    /** Set the UI context for all registered panels.
     *  Propagates the context pointer to every panel via
     *  EditorPanel::SetContext().  Pass nullptr to clear. */
    void SetContext(atlas::AtlasContext* ctx);

    /** Notify every registered panel that an asset changed on disk. */
    void BroadcastAssetReload(const std::string& assetId,
                              const std::string& path);

    DockNode& Root() { return m_root; }
    const std::vector<EditorPanel*>& Panels() const { return m_panels; }

private:
    DockNode m_root;
    std::vector<EditorPanel*> m_panels;

    static void DrawNode(DockNode& node);
};

}
