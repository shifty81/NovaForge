#include "ui/atlas/atlas_widgets.h"

namespace atlas {

// ── Menu Bar (Win32-style) ──────────────────────────────────────────

int menuBar(AtlasContext& ctx, const Rect& r,
            const std::vector<Menu>& menus, MenuBarState& state) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int result = -1;
    constexpr int kItemsPerMenu = 1000; // encoding: menuIndex * kItemsPerMenu + itemIndex

    // Bar background — solid dark header
    rr.drawRect(r, t.bgHeader);
    // Bottom accent line
    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f},
                t.borderNormal);

    // Consume mouse within the bar to prevent click-through
    if (ctx.isHovered(r) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
        ctx.consumeMouse();
    }

    float menuX = r.x + 4.0f;
    constexpr float itemH = 22.0f;

    for (int mi = 0; mi < static_cast<int>(menus.size()); ++mi) {
        const Menu& menu = menus[mi];
        float tw = rr.measureText(menu.label) + 16.0f;
        Rect labelRect = {menuX, r.y, tw, r.h};

        WidgetID menuID = ctx.currentID(menu.label.c_str());
        bool hovered = ctx.isHovered(labelRect);
        if (hovered) ctx.setHot(menuID);

        // Highlight if open or hovered
        bool isOpen = (state.openMenu == mi);
        if (isOpen) {
            rr.drawRect(labelRect, t.selection);
        } else if (hovered) {
            rr.drawRect(labelRect, t.hover);
        }

        // Draw top-level label
        float textY = r.y + (r.h - 13.0f) * 0.5f;
        rr.drawText(menu.label, {menuX + 8.0f, textY},
                     isOpen ? t.textPrimary : (hovered ? t.textPrimary : t.textSecondary));

        // Click toggles the menu open/close; hovering while another menu
        // is open switches to this menu (Win32 behavior)
        if (ctx.buttonBehavior(labelRect, menuID)) {
            state.openMenu = isOpen ? -1 : mi;
            isOpen = (state.openMenu == mi);
        } else if (hovered && state.openMenu >= 0 && state.openMenu != mi) {
            state.openMenu = mi;
            isOpen = true;
        }

        // Draw dropdown
        if (isOpen && !menu.items.empty()) {
            float dropW = 0.0f;
            for (auto& item : menu.items) {
                float iw = item.label.empty() ? 0.0f : rr.measureText(item.label);
                if (item.checked) iw += 18.0f; // space for check mark
                if (iw > dropW) dropW = iw;
            }
            dropW += 32.0f; // padding

            float dropH = 0.0f;
            for (auto& item : menu.items) {
                dropH += item.label.empty() ? 6.0f : itemH;
            }

            Rect dropRect = {menuX, r.bottom(), dropW, dropH};
            rr.drawRect(dropRect, t.bgPanel);
            rr.drawRectOutline(dropRect, t.borderNormal);

            // Consume mouse within dropdown
            if (ctx.isHovered(dropRect) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
                ctx.consumeMouse();
            }

            float iy = r.bottom();
            for (int ii = 0; ii < static_cast<int>(menu.items.size()); ++ii) {
                const MenuItem& item = menu.items[ii];

                // Separator
                if (item.label.empty()) {
                    rr.drawRect({menuX + 4.0f, iy + 2.0f, dropW - 8.0f, 1.0f},
                                t.borderSubtle);
                    iy += 6.0f;
                    continue;
                }

                Rect itemRect = {menuX, iy, dropW, itemH};
                WidgetID itemID = ctx.currentID(item.label.c_str());

                bool itemHovered = ctx.isHovered(itemRect);
                if (itemHovered && item.enabled) ctx.setHot(itemID);

                // Row highlight
                if (itemHovered && item.enabled) {
                    rr.drawRect(itemRect, t.hover);
                }

                // Check mark
                float textX = menuX + 8.0f;
                if (item.checked) {
                    float checkY = iy + (itemH - 13.0f) * 0.5f;
                    rr.drawText("*", {textX, checkY}, t.accentPrimary);
                    textX += 18.0f;
                }

                // Item text
                float itextY = iy + (itemH - 13.0f) * 0.5f;
                Color textColor = item.enabled ? t.textPrimary : t.textDisabled;
                rr.drawText(item.label, {textX, itextY}, textColor);

                // Click handling
                if (item.enabled && ctx.buttonBehavior(itemRect, itemID)) {
                    result = mi * kItemsPerMenu + ii;
                    state.openMenu = -1; // close menu after click
                }

                iy += itemH;
            }
        }

        menuX += tw + 2.0f;
    }

    // Close menu if clicking outside any menu/dropdown
    if (ctx.isMouseClicked() && state.openMenu >= 0 && result == -1) {
        bool insideAnyMenu = ctx.isHovered(r);
        // Also check if inside an open dropdown
        if (!insideAnyMenu && state.openMenu >= 0) {
            float checkX = r.x + 4.0f;
            for (int ci = 0; ci < static_cast<int>(menus.size()); ++ci) {
                float tw = rr.measureText(menus[ci].label) + 16.0f;
                if (ci == state.openMenu) {
                    float dropW = 0.0f;
                    for (auto& item : menus[ci].items) {
                        float iw = item.label.empty() ? 0.0f : rr.measureText(item.label);
                        if (item.checked) iw += 18.0f;
                        if (iw > dropW) dropW = iw;
                    }
                    dropW += 32.0f;
                    float dropH = 0.0f;
                    for (auto& item : menus[ci].items) {
                        dropH += item.label.empty() ? 6.0f : itemH;
                    }
                    Rect dropRect = {checkX, r.bottom(), dropW, dropH};
                    if (ctx.isHovered(dropRect)) insideAnyMenu = true;
                }
                checkX += tw + 2.0f;
            }
        }
        if (!insideAnyMenu) {
            state.openMenu = -1;
        }
    }

    return result;
}

} // namespace atlas
