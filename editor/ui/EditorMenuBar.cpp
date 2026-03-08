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
    m_panelMap.clear();

    // Build name→panel lookup for O(1) per-frame state refresh
    for (auto* panel : m_panels) {
        if (panel) m_panelMap[panel->Name()] = panel;
    }

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
    // Refresh checked state using O(1) map lookup instead of O(n²) nested loop
    if (m_menus.size() >= 2) {
        auto& viewItems = m_menus[1].items;
        for (auto& item : viewItems) {
            auto it = m_panelMap.find(item.label);
            if (it != m_panelMap.end()) {
                item.checked = it->second->IsVisible();
            }
        }
        if (m_menus.size() >= 3) {
            auto& pcgItems = m_menus[2].items;
            for (auto& item : pcgItems) {
                auto it = m_panelMap.find(item.label);
                if (it != m_panelMap.end()) {
                    item.checked = it->second->IsVisible();
                }
            }
        }
    }

    // In headless/test mode, skip actual rendering
    if (!ctx) return MenuBarHeight;

    atlas::Rect barRect = {0.0f, 0.0f, windowW, MenuBarHeight};
    atlas::menuBar(*ctx, barRect, m_menus, m_state);

    // Dropdown clicks are now handled in DrawDropdown() which runs
    // after dock panels so the dropdown draws on top.

    return MenuBarHeight;
}

void EditorMenuBar::DrawDropdown(atlas::AtlasContext* ctx, float windowW) {
    if (!ctx) return;
    if (m_state.openMenu < 0) return;

    atlas::Rect barRect = {0.0f, 0.0f, windowW, MenuBarHeight};
    int clicked = atlas::menuBarDropdown(*ctx, barRect, m_menus, m_state);

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
}

} // namespace atlas::editor
