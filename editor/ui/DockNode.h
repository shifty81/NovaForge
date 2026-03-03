#pragma once
#include <memory>
#include <vector>
#include "EditorPanel.h"

namespace atlas::editor {

enum class DockSplit {
    None,
    Horizontal,
    Vertical,
    Tab
};

struct DockNode {
    DockSplit split = DockSplit::None;
    float splitRatio = 0.5f;

    /** Computed layout bounds — set each frame by EditorLayout. */
    atlas::Rect bounds;

    std::unique_ptr<DockNode> a;
    std::unique_ptr<DockNode> b;

    EditorPanel* panel = nullptr;

    std::vector<EditorPanel*> tabs;
    int activeTab = 0;
};

}
