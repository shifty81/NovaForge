#pragma once
#include "DockNode.h"
#include <vector>
#include <memory>
#include <string>

namespace atlas::editor {

class EditorLayout {
public:
    void RegisterPanel(EditorPanel* panel);
    void Draw();

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
