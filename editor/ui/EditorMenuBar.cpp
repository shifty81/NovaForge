#include "EditorMenuBar.h"
#include <algorithm>

namespace atlas::editor {

static constexpr int kItemsPerMenu = 1000;

void EditorMenuBar::RegisterPanel(EditorPanel* panel) {
    if (panel) m_panels.push_back(panel);
}

bool EditorMenuBar::IsPCGPanel(const std::string& name) {
    return name == "PCG Preview"
        || name == "Ship Archetype"
        || name == "Generation Style"
        || name == "Asset Style"
        || name == "Galaxy Map"
        || name == "Fleet Formation";
}

void EditorMenuBar::Build() {
    m_menus.clear();

    // ── File menu ────────────────────────────────────────────────
    atlas::Menu fileMenu;
    fileMenu.label = "File";
    fileMenu.items.push_back({"Save Layout", true, false});
    fileMenu.items.push_back({"Load Layout", true, false});
    fileMenu.items.push_back({"", true, false});  // separator
    fileMenu.items.push_back({"Exit", true, false});
    m_menus.push_back(std::move(fileMenu));

    // ── View menu (panel toggles) ────────────────────────────────
    atlas::Menu viewMenu;
    viewMenu.label = "View";
    for (auto* panel : m_panels) {
        if (!panel) continue;
        viewMenu.items.push_back({panel->Name(), true, panel->IsVisible()});
    }
    m_menus.push_back(std::move(viewMenu));

    // ── PCG Content menu ─────────────────────────────────────────
    atlas::Menu pcgMenu;
    pcgMenu.label = "PCG Content";
    for (auto* panel : m_panels) {
        if (!panel) continue;
        if (IsPCGPanel(panel->Name())) {
            pcgMenu.items.push_back({panel->Name(), true, panel->IsVisible()});
        }
    }
    if (!pcgMenu.items.empty()) {
        m_menus.push_back(std::move(pcgMenu));
    }
}

float EditorMenuBar::Draw(atlas::AtlasContext* ctx, float windowW) {
    // Rebuild the View and PCG Content menus each frame so that the
    // checked state stays in sync with panel visibility.
    if (m_menus.size() >= 2) {
        // View menu (index 1): refresh checked state by matching panel name
        auto& viewItems = m_menus[1].items;
        for (auto& item : viewItems) {
            for (auto* panel : m_panels) {
                if (panel && item.label == panel->Name()) {
                    item.checked = panel->IsVisible();
                    break;
                }
            }
        }
        // PCG Content menu (index 2 if present)
        if (m_menus.size() >= 3) {
            auto& pcgItems = m_menus[2].items;
            for (auto& item : pcgItems) {
                for (auto* panel : m_panels) {
                    if (panel && item.label == panel->Name()) {
                        item.checked = panel->IsVisible();
                        break;
                    }
                }
            }
        }
    }

    // In headless/test mode, skip actual rendering
    if (!ctx) return MenuBarHeight;

    atlas::Rect barRect = {0.0f, 0.0f, windowW, MenuBarHeight};
    int clicked = atlas::menuBar(*ctx, barRect, m_menus, m_state);

    if (clicked >= 0) {
        int menuIdx = clicked / kItemsPerMenu;
        int itemIdx = clicked % kItemsPerMenu;

        if (menuIdx == 0) {
            // File menu
            if (itemIdx == 0 && onSaveLayout) onSaveLayout();
            else if (itemIdx == 1 && onLoadLayout) onLoadLayout();
            else if (itemIdx == 3 && onExit) onExit();
        } else if (menuIdx == 1) {
            // View menu — toggle panel visibility
            if (itemIdx >= 0 && itemIdx < static_cast<int>(m_panels.size())) {
                auto* panel = m_panels[itemIdx];
                if (panel) panel->SetVisible(!panel->IsVisible());
            }
        } else if (menuIdx == 2) {
            // PCG Content menu — toggle panel visibility + notify callback
            size_t pi = 0;
            for (auto* panel : m_panels) {
                if (!panel || !IsPCGPanel(panel->Name())) continue;
                if (static_cast<int>(pi) == itemIdx) {
                    panel->SetVisible(!panel->IsVisible());
                    if (onPCGContentSelected) onPCGContentSelected(panel->Name());
                    break;
                }
                ++pi;
            }
        }
    }

    return MenuBarHeight;
}

} // namespace atlas::editor
