#include "EditorLayout.h"
#include <algorithm>

namespace atlas::editor {

void EditorLayout::RegisterPanel(EditorPanel* panel) {
    m_panels.push_back(panel);
}

void EditorLayout::Draw() {
    DrawNode(m_root);
}

void EditorLayout::DrawNode(DockNode& node) {
    if (node.split == DockSplit::Tab) {
        if (!node.tabs.empty()) {
            int idx = std::clamp(node.activeTab, 0, static_cast<int>(node.tabs.size()) - 1);
            if (node.tabs[idx] && node.tabs[idx]->IsVisible()) {
                node.tabs[idx]->Draw();
            }
        }
        return;
    }

    if (node.split == DockSplit::None) {
        if (node.panel && node.panel->IsVisible()) {
            node.panel->Draw();
        }
        return;
    }

    if (node.a) DrawNode(*node.a);
    if (node.b) DrawNode(*node.b);
}

}
