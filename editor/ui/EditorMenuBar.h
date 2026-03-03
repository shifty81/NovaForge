#pragma once

#include "EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace atlas::editor {

/**
 * Win32-style menu bar for the Atlas Editor.
 *
 * Provides top-level menus:
 *   File        — Save Layout, Load Layout, Exit
 *   View        — Toggle visibility of each registered panel
 *   PCG Content — Quick-switch to PCG editing panels
 *
 * The menu bar occupies a thin strip at the top of the editor window.
 * EditorLayout draws it before the dock tree and offsets dock content
 * below the menu bar height.
 */
class EditorMenuBar {
public:
    static constexpr float MenuBarHeight = 22.0f;

    /** Register a panel whose visibility can be toggled from the View menu. */
    void RegisterPanel(EditorPanel* panel);

    /** Build/rebuild the menu structure from registered panels.
     *  Call after all panels are registered and before the first draw. */
    void Build();

    /**
     * Draw the menu bar and handle clicks.
     *
     * @param ctx     Atlas UI context (may be null in headless/test mode).
     * @param windowW Current window width.
     * @return Height in pixels consumed by the menu bar.
     */
    float Draw(atlas::AtlasContext* ctx, float windowW);

    /** Read-only access to registered panels. */
    const std::vector<EditorPanel*>& Panels() const { return m_panels; }

    /** Read-only access to the built menus (for testing). */
    const std::vector<atlas::Menu>& Menus() const { return m_menus; }

    /** Read-only access to persistent menu bar state (for testing). */
    const atlas::MenuBarState& State() const { return m_state; }
    atlas::MenuBarState& State() { return m_state; }

    // ── Callbacks ────────────────────────────────────────────────────

    /** Called when "Save Layout" is selected. */
    std::function<void()> onSaveLayout;

    /** Called when "Load Layout" is selected. */
    std::function<void()> onLoadLayout;

    /** Called when "Exit" is selected. */
    std::function<void()> onExit;

    /** Called when a PCG content panel is selected.
     *  Receives the panel name. */
    std::function<void(const std::string&)> onPCGContentSelected;

private:
    std::vector<EditorPanel*> m_panels;
    std::vector<atlas::Menu>  m_menus;
    atlas::MenuBarState       m_state;

    // Fast panel lookup by name — built once during Build()
    std::unordered_map<std::string, EditorPanel*> m_panelMap;

    // Panel names that belong to the PCG Content menu
    static bool IsPCGPanel(const std::string& name);
};

} // namespace atlas::editor
