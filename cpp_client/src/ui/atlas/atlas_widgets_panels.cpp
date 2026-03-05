#include "ui/atlas/atlas_widgets.h"

#include <algorithm>
#include <cmath>

namespace atlas {

// Helper: truncate a string with "..." suffix if it exceeds maxWidth pixels
static std::string truncateText(AtlasRenderer& rr, const std::string& text, float maxWidth) {
    if (text.empty() || rr.measureText(text) <= maxWidth) return text;
    std::string result = text;
    while (result.size() > 1 && rr.measureText(result + "...") > maxWidth) {
        result.pop_back();
    }
    result += "...";
    return result;
}

// ── Panel ───────────────────────────────────────────────────────────

bool panelBegin(AtlasContext& ctx, const char* title,
                Rect& bounds, const PanelFlags& flags,
                bool* open) {
    const Theme& t = ctx.theme();
    auto& r = ctx.renderer();

    ctx.pushID(title);

    // Consume mouse if clicking within the panel body to prevent click-through
    if (ctx.isHovered(bounds) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
        ctx.consumeMouse();
    }

    // Photon-style panel: dark translucent fill with skeletal frame
    r.drawRect(bounds, t.bgPanel);

    // Frame border (thin, skeletal — not a heavy box)
    if (flags.drawBorder) {
        r.drawRectOutline(bounds, t.borderNormal, t.borderWidth);
    }

    // Header bar
    if (flags.showHeader) {
        float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
        Rect headerRect = {bounds.x, bounds.y, bounds.w, hh};
        r.drawRect(headerRect, t.bgHeader);

        // Title text (uppercase for headers per Photon guidelines)
        float textY = bounds.y + (hh - 13.0f) * 0.5f;
        r.drawText(title, {bounds.x + t.padding, textY}, t.textPrimary);

        // Close button (×)
        if (flags.showClose && open) {
            float btnSz = 14.0f;
            Rect closeRect = {bounds.right() - btnSz - 4.0f,
                              bounds.y + (hh - btnSz) * 0.5f,
                              btnSz, btnSz};
            WidgetID closeID = ctx.currentID("_close");
            bool hovered = ctx.isHovered(closeRect);
            if (hovered) ctx.setHot(closeID);
            Color closeBg = hovered ? t.danger.withAlpha(0.6f)
                                    : Color(0, 0, 0, 0);
            r.drawRect(closeRect, closeBg);
            r.drawText("x", {closeRect.x + 3.0f, closeRect.y + 1.0f},
                       hovered ? t.textPrimary : t.textSecondary);
            if (ctx.buttonBehavior(closeRect, closeID)) {
                *open = false;
            }
        }

        // Minimize button (—)
        if (flags.showMinimize) {
            float btnSz = 14.0f;
            float offset = (flags.showClose && open) ? 22.0f : 4.0f;
            Rect minRect = {bounds.right() - btnSz - offset,
                            bounds.y + (hh - btnSz) * 0.5f,
                            btnSz, btnSz};
            WidgetID minID = ctx.currentID("_min");
            bool hovered = ctx.isHovered(minRect);
            if (hovered) ctx.setHot(minID);
            Color minBg = hovered ? t.hover : Color(0, 0, 0, 0);
            r.drawRect(minRect, minBg);
            r.drawText("-", {minRect.x + 3.0f, minRect.y + 1.0f},
                       hovered ? t.textPrimary : t.textSecondary);
        }

        // Photon accent line under header (frame separator)
        r.drawRect({bounds.x, bounds.y + hh - 1.0f, bounds.w, 1.0f},
                   t.accentPrimary.withAlpha(0.3f));
    }

    return true;  // panel is open (minimize state not yet tracked)
}

void panelEnd(AtlasContext& ctx) {
    // Consume any leftover mouse clicks within the panel body so they
    // don't fall through to the 3D world or widgets behind the panel.
    Rect bounds = ctx.popPanelBounds();
    if (bounds.w > 0.0f && bounds.h > 0.0f) {
        if (ctx.isHovered(bounds) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
            ctx.consumeMouse();
        }
    }
    ctx.popID();
}

// ── Tooltip ─────────────────────────────────────────────────────────

void tooltip(AtlasContext& ctx, const std::string& text) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float tw = rr.measureText(text) + 16.0f;
    float th = 24.0f;
    Vec2 mouse = ctx.input().mousePos;
    // Position tooltip slightly below and to the right of cursor
    Rect tipRect = {mouse.x + 12.0f, mouse.y + 16.0f, tw, th};

    // Clamp to window bounds
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);
    if (tipRect.right() > winW) tipRect.x = winW - tw;
    if (tipRect.bottom() > winH) tipRect.y = mouse.y - th - 4.0f;

    rr.drawRect(tipRect, t.bgTooltip);
    rr.drawRectOutline(tipRect, t.borderNormal);
    rr.drawText(text, {tipRect.x + 8.0f, tipRect.y + 5.0f}, t.textPrimary);
}

// ── Stateful Panel ──────────────────────────────────────────────────

bool panelBeginStateful(AtlasContext& ctx, const char* title,
                        PanelState& state, const PanelFlags& flags) {
    if (!state.open) return false;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    ctx.pushID(title);

    // Effective locked state: PanelFlags OR PanelState lock
    bool effectivelyLocked = flags.locked || state.locked;

    // Pre-compute header button rects so the drag handler can exclude them
    float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
    float btnSz = 14.0f;

    // Right-side buttons: [gear] [minimize] [close]  (right to left)
    Rect closeRect = {state.bounds.right() - btnSz - 4.0f,
                      state.bounds.y + (hh - btnSz) * 0.5f,
                      btnSz, btnSz};
    float minOffset = flags.showClose ? 22.0f : 4.0f;
    Rect minRect = {state.bounds.right() - btnSz - minOffset,
                    state.bounds.y + (hh - btnSz) * 0.5f,
                    btnSz, btnSz};
    float gearOffset = minOffset + (flags.showMinimize ? 18.0f : 0.0f);
    Rect gearRect = {state.bounds.right() - btnSz - gearOffset,
                     state.bounds.y + (hh - btnSz) * 0.5f,
                     btnSz, btnSz};

    // Left-side button: [lock] just after title
    float titleW = rr.measureText(title);
    Rect lockRect = {state.bounds.x + t.padding + titleW + 6.0f,
                     state.bounds.y + (hh - btnSz) * 0.5f,
                     btnSz, btnSz};

    // Process close/minimize/lock/gear buttons BEFORE drag, so they get priority
    bool closeBtnClicked = false;
    bool minBtnClicked = false;
    bool lockBtnClicked = false;
    bool gearBtnClicked = false;
    if (flags.showHeader && !ctx.isMouseConsumed()) {
        if (flags.showClose) {
            WidgetID closeID = ctx.currentID("_close");
            closeBtnClicked = ctx.buttonBehavior(closeRect, closeID);
            if (closeBtnClicked) {
                state.open = false;
                ctx.consumeMouse();
            }
        }
        if (flags.showMinimize && !ctx.isMouseConsumed()) {
            WidgetID minID = ctx.currentID("_min");
            minBtnClicked = ctx.buttonBehavior(minRect, minID);
            if (minBtnClicked) {
                state.minimized = !state.minimized;
                ctx.consumeMouse();
            }
        }
        // Lock button
        if (!ctx.isMouseConsumed()) {
            WidgetID lockID = ctx.currentID("_lock");
            lockBtnClicked = ctx.buttonBehavior(lockRect, lockID);
            if (lockBtnClicked) {
                state.locked = !state.locked;
                ctx.consumeMouse();
            }
        }
        // Gear/settings button
        if (!ctx.isMouseConsumed()) {
            WidgetID gearID = ctx.currentID("_gear");
            gearBtnClicked = ctx.buttonBehavior(gearRect, gearID);
            if (gearBtnClicked) {
                state.settingsOpen = !state.settingsOpen;
                ctx.consumeMouse();
            }
        }
    }

    // Handle dragging (header-bar drag to move) — skip if locked or a button was clicked
    bool anyBtnClicked = closeBtnClicked || minBtnClicked || lockBtnClicked || gearBtnClicked;
    if (!effectivelyLocked && flags.showHeader && !anyBtnClicked) {
        Rect headerHit = {state.bounds.x, state.bounds.y, state.bounds.w, hh};
        WidgetID dragID = ctx.currentID("_drag");

        if (!ctx.isMouseConsumed() && ctx.isHovered(headerHit) && ctx.isMouseClicked()) {
            // Don't start drag if mouse is over a header button
            Vec2 mp = ctx.input().mousePos;
            bool overButton = (flags.showClose && closeRect.contains(mp)) ||
                              (flags.showMinimize && minRect.contains(mp)) ||
                              lockRect.contains(mp) || gearRect.contains(mp);
            if (!overButton) {
                state.dragging = true;
                state.dragOffset = {mp.x - state.bounds.x, mp.y - state.bounds.y};
                ctx.setActive(dragID);
                ctx.consumeMouse();
            }
        }

        if (state.dragging) {
            ctx.consumeMouse();
            if (ctx.isMouseDown()) {
                state.bounds.x = ctx.input().mousePos.x - state.dragOffset.x;
                state.bounds.y = ctx.input().mousePos.y - state.dragOffset.y;

                // Clamp to window bounds (sidebar is the left border)
                float winW = static_cast<float>(ctx.input().windowW);
                float winH = static_cast<float>(ctx.input().windowH);
                float leftEdge = ctx.sidebarWidth();
                state.bounds.x = std::max(leftEdge, std::min(state.bounds.x, winW - state.bounds.w));
                state.bounds.y = std::max(0.0f, std::min(state.bounds.y, winH - t.headerHeight));

                // ── Astralis-style edge magnetism / snapping ────────────────
                // Panels snap to screen edges and the Nexcom sidebar
                // when within the snap threshold (Photon UI principle).
                constexpr float snapThreshold = 15.0f;

                // Snap left edge to sidebar
                if (std::fabs(state.bounds.x - leftEdge) < snapThreshold)
                    state.bounds.x = leftEdge;
                // Snap right edge to window right
                if (std::fabs(state.bounds.right() - winW) < snapThreshold)
                    state.bounds.x = winW - state.bounds.w;
                // Snap top edge to window top
                if (std::fabs(state.bounds.y) < snapThreshold)
                    state.bounds.y = 0.0f;
                // Snap bottom edge to window bottom
                if (std::fabs(state.bounds.bottom() - winH) < snapThreshold)
                    state.bounds.y = winH - state.bounds.h;
            } else {
                state.dragging = false;
                ctx.clearActive();
            }
        }
    }

    // Handle resizing (edge/corner drag) — skip if locked or minimized
    if (!effectivelyLocked && !state.minimized && !anyBtnClicked && !state.dragging) {
        constexpr float edgeGrab = 6.0f;  // grab zone in pixels
        const Rect& b = state.bounds;

        // Edge hit-test rects
        Rect leftEdge   = {b.x - edgeGrab, b.y + edgeGrab, edgeGrab * 2, b.h - edgeGrab * 2};
        Rect rightEdge  = {b.right() - edgeGrab, b.y + edgeGrab, edgeGrab * 2, b.h - edgeGrab * 2};
        Rect topEdge    = {b.x + edgeGrab, b.y - edgeGrab, b.w - edgeGrab * 2, edgeGrab * 2};
        Rect bottomEdge = {b.x + edgeGrab, b.bottom() - edgeGrab, b.w - edgeGrab * 2, edgeGrab * 2};

        // Corner hit-test rects
        Rect topLeft     = {b.x - edgeGrab, b.y - edgeGrab, edgeGrab * 2, edgeGrab * 2};
        Rect topRight    = {b.right() - edgeGrab, b.y - edgeGrab, edgeGrab * 2, edgeGrab * 2};
        Rect bottomLeft  = {b.x - edgeGrab, b.bottom() - edgeGrab, edgeGrab * 2, edgeGrab * 2};
        Rect bottomRight = {b.right() - edgeGrab, b.bottom() - edgeGrab, edgeGrab * 2, edgeGrab * 2};

        if (!ctx.isMouseConsumed() && ctx.isMouseClicked() && !state.resizing) {
            int edge = 0;
            Vec2 mp = ctx.input().mousePos;
            if (bottomRight.contains(mp))      edge = 2 | 8;
            else if (bottomLeft.contains(mp))  edge = 1 | 8;
            else if (topRight.contains(mp))    edge = 2 | 4;
            else if (topLeft.contains(mp))     edge = 1 | 4;
            else if (leftEdge.contains(mp))    edge = 1;
            else if (rightEdge.contains(mp))   edge = 2;
            else if (topEdge.contains(mp))     edge = 4;
            else if (bottomEdge.contains(mp))  edge = 8;

            if (edge != 0) {
                state.resizing = true;
                state.resizeEdge = edge;
                state.resizeAnchor = mp;
                state.resizeOrigBounds = state.bounds;
                ctx.consumeMouse();
            }
        }

        if (state.resizing) {
            ctx.consumeMouse();
            if (ctx.isMouseDown()) {
                Vec2 delta = ctx.input().mousePos - state.resizeAnchor;
                Rect nb = state.resizeOrigBounds;

                if (state.resizeEdge & 1) { // left
                    float newX = nb.x + delta.x;
                    float newW = nb.w - delta.x;
                    float leftEdge = ctx.sidebarWidth();
                    // Clamp left edge to sidebar boundary
                    if (newX < leftEdge) {
                        newW -= (leftEdge - newX);
                        newX = leftEdge;
                    }
                    if (newW >= state.minW) { state.bounds.x = newX; state.bounds.w = newW; }
                }
                if (state.resizeEdge & 2) { // right
                    float newW = nb.w + delta.x;
                    if (newW >= state.minW) { state.bounds.w = newW; }
                }
                if (state.resizeEdge & 4) { // top
                    float newY = nb.y + delta.y;
                    float newH = nb.h - delta.y;
                    if (newH >= state.minH) { state.bounds.y = newY; state.bounds.h = newH; }
                }
                if (state.resizeEdge & 8) { // bottom
                    float newH = nb.h + delta.y;
                    if (newH >= state.minH) { state.bounds.h = newH; }
                }
            } else {
                state.resizing = false;
                state.resizeEdge = 0;
            }
        }
    }

    // Push panel bounds so panelEnd can consume leftover clicks
    // (deferred to let content widgets process clicks first)
    ctx.pushPanelBounds(state.bounds);

    // Draw panel background (apply opacity)
    Rect drawBounds = state.bounds;
    if (state.minimized) {
        drawBounds.h = hh;
    }

    Color panelBg = t.bgPanel.withAlpha(t.bgPanel.a * state.opacity);
    rr.drawRect(drawBounds, panelBg);

    // Border (Photon skeletal frame)
    if (flags.drawBorder) {
        rr.drawRectOutline(drawBounds, t.borderNormal, t.borderWidth);
    }

    // Resize grip indicator (bottom-right corner dots when not locked)
    if (!effectivelyLocked && !state.minimized) {
        float gx = drawBounds.right() - 10.0f;
        float gy = drawBounds.bottom() - 10.0f;
        Color gripColor = t.textMuted.withAlpha(0.4f);
        rr.drawRect({gx + 4.0f, gy + 4.0f, 3.0f, 3.0f}, gripColor);
        rr.drawRect({gx,        gy + 4.0f, 3.0f, 3.0f}, gripColor);
        rr.drawRect({gx + 4.0f, gy,        3.0f, 3.0f}, gripColor);
    }

    // Header bar
    if (flags.showHeader) {
        Rect headerRect = {drawBounds.x, drawBounds.y, drawBounds.w, hh};
        rr.drawRect(headerRect, t.bgHeader.withAlpha(t.bgHeader.a * state.opacity));

        float textY = drawBounds.y + (hh - 13.0f) * 0.5f;
        rr.drawText(title, {drawBounds.x + t.padding, textY}, t.textPrimary);

        // Lock button (left side, after title) — padlock icon
        {
            WidgetID lockID = ctx.currentID("_lock");
            bool hovered = ctx.isHovered(lockRect);
            if (hovered) ctx.setHot(lockID);
            Color lockBg = hovered ? t.hover : Color(0, 0, 0, 0);
            rr.drawRect(lockRect, lockBg);
            const char* lockIcon = state.locked ? "L" : "U";
            Color lockColor = state.locked ? t.accentPrimary : t.textMuted;
            if (hovered) lockColor = t.textPrimary;
            rr.drawText(lockIcon, {lockRect.x + 2.0f, lockRect.y + 1.0f}, lockColor);
        }

        // Close button (×) — visual only, behavior handled above
        if (flags.showClose) {
            WidgetID closeID = ctx.currentID("_close");
            bool hovered = ctx.isHovered(closeRect);
            if (hovered) ctx.setHot(closeID);
            Color closeBg = hovered ? t.danger.withAlpha(0.6f) : Color(0, 0, 0, 0);
            rr.drawRect(closeRect, closeBg);
            rr.drawText("x", {closeRect.x + 3.0f, closeRect.y + 1.0f},
                        hovered ? t.textPrimary : t.textSecondary);
        }

        // Minimize button (—) — visual only, behavior handled above
        if (flags.showMinimize) {
            WidgetID minID = ctx.currentID("_min");
            bool hovered = ctx.isHovered(minRect);
            if (hovered) ctx.setHot(minID);
            Color minBg = hovered ? t.hover : Color(0, 0, 0, 0);
            rr.drawRect(minRect, minBg);
            rr.drawText("-", {minRect.x + 3.0f, minRect.y + 1.0f},
                        hovered ? t.textPrimary : t.textSecondary);
        }

        // Gear/settings button — visual only, behavior handled above
        {
            WidgetID gearID = ctx.currentID("_gear");
            bool hovered = ctx.isHovered(gearRect);
            if (hovered) ctx.setHot(gearID);
            Color gearBg = (hovered || state.settingsOpen) ? t.hover : Color(0, 0, 0, 0);
            rr.drawRect(gearRect, gearBg);
            Color gearColor = state.settingsOpen ? t.accentPrimary : (hovered ? t.textPrimary : t.textSecondary);
            rr.drawText("*", {gearRect.x + 3.0f, gearRect.y + 1.0f}, gearColor);
        }

        // Photon accent line under header
        rr.drawRect({drawBounds.x, drawBounds.y + hh - 1.0f, drawBounds.w, 1.0f},
                    t.accentPrimary.withAlpha(0.3f));
    }

    // Settings dropdown popup (below gear button)
    if (state.settingsOpen && flags.showHeader) {
        float popW = 180.0f;
        float popH = 72.0f;
        float popX = std::min(gearRect.x, static_cast<float>(ctx.input().windowW) - popW - 4.0f);
        float popY = state.bounds.y + hh + 2.0f;

        Rect popBg(popX, popY, popW, popH);
        rr.drawRect(popBg, t.bgPanel);
        rr.drawRectOutline(popBg, t.borderNormal);

        // Consume mouse within popup
        if (ctx.isHovered(popBg) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
            ctx.consumeMouse();
        }

        float py = popY + 4.0f;

        // Opacity slider — use 20–100 range for display; convert back to 0.0–1.0
        rr.drawText("Opacity", {popX + 6.0f, py + 1.0f}, t.textSecondary);
        py += 16.0f;
        Rect opacitySlider(popX + 6.0f, py, popW - 12.0f, 14.0f);
        float opacityPct = state.opacity * 100.0f;
        if (slider(ctx, "_opacity", opacitySlider, &opacityPct, 20.0f, 100.0f, "%.0f%%")) {
            state.opacity = opacityPct / 100.0f;
        }
        py += 22.0f;

        // Compact rows toggle
        Rect compactRect(popX + 6.0f, py, popW - 12.0f, 14.0f);
        checkbox(ctx, "Compact rows", compactRect, &state.compactRows);
    }

    // Close settings popup when clicking outside it
    if (state.settingsOpen && ctx.isMouseClicked()) {
        float popW = 180.0f;
        float popH = 72.0f;
        float popX = std::min(gearRect.x, static_cast<float>(ctx.input().windowW) - popW - 4.0f);
        float popY = state.bounds.y + hh + 2.0f;
        Rect popBg(popX, popY, popW, popH);
        if (!ctx.isHovered(popBg) && !ctx.isHovered(gearRect)) {
            state.settingsOpen = false;
        }
    }

    return !state.minimized;
}

} // namespace atlas
