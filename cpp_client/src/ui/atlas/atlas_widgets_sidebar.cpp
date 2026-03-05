#include "ui/atlas/atlas_widgets.h"

#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atlas {

// ── Sidebar Icon Shape Helpers ──────────────────────────────────────
// Draw shape-based icons modelled after Astralis's Nexcom Photon UI.
// Each icon uses simple geometric primitives (rect, circle, line, arc)
// to create a distinctive silhouette recognizable at a glance.

static void drawIconInventory(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Open box: rectangle body with angled lid
    float hs = sz * 0.38f;
    rr.drawRectOutline({c.x - hs, c.y - hs * 0.6f, hs * 2, hs * 1.6f}, col, 1.5f);
    // Lid top line
    rr.drawLine({c.x - hs * 1.1f, c.y - hs * 0.6f},
                {c.x + hs * 1.1f, c.y - hs * 0.6f}, col, 1.5f);
    // Handle tab at top center
    rr.drawLine({c.x - hs * 0.3f, c.y - hs * 0.6f},
                {c.x - hs * 0.3f, c.y - hs * 0.95f}, col, 1.5f);
    rr.drawLine({c.x + hs * 0.3f, c.y - hs * 0.6f},
                {c.x + hs * 0.3f, c.y - hs * 0.95f}, col, 1.5f);
    rr.drawLine({c.x - hs * 0.3f, c.y - hs * 0.95f},
                {c.x + hs * 0.3f, c.y - hs * 0.95f}, col, 1.5f);
}

static void drawIconFitting(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Wrench: diagonal tool shape
    float hs = sz * 0.38f;
    // Shaft (diagonal line)
    rr.drawLine({c.x - hs * 0.7f, c.y + hs * 0.7f},
                {c.x + hs * 0.4f, c.y - hs * 0.4f}, col, 2.0f);
    // Jaw (U shape at top-right end)
    rr.drawArc(c + Vec2(hs * 0.5f, -hs * 0.5f),
               hs * 0.2f, hs * 0.45f,
               -0.5f, 2.1f, col, 6);
    // Handle end (small rect at bottom-left)
    rr.drawCircle(c + Vec2(-hs * 0.7f, hs * 0.7f), hs * 0.15f, col, 6);
}

static void drawIconMarket(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Bar chart: three vertical bars ascending left-to-right
    float hs = sz * 0.35f;
    float bw = hs * 0.45f;
    float gap = hs * 0.15f;
    float baseY = c.y + hs;
    // Bar 1 (short)
    float x0 = c.x - hs;
    rr.drawRect({x0, baseY - hs * 0.8f, bw, hs * 0.8f}, col.withAlpha(0.7f));
    // Bar 2 (medium)
    float x1 = x0 + bw + gap;
    rr.drawRect({x1, baseY - hs * 1.3f, bw, hs * 1.3f}, col.withAlpha(0.85f));
    // Bar 3 (tall)
    float x2 = x1 + bw + gap;
    rr.drawRect({x2, baseY - hs * 1.8f, bw, hs * 1.8f}, col);
    // Baseline
    rr.drawLine({c.x - hs * 1.1f, baseY}, {c.x + hs * 1.1f, baseY}, col.withAlpha(0.5f), 1.0f);
}

static void drawIconMission(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Journal/document: rectangle with horizontal text lines
    float hs = sz * 0.35f;
    Rect doc = {c.x - hs, c.y - hs * 1.1f, hs * 2, hs * 2.2f};
    rr.drawRectOutline(doc, col, 1.5f);
    // Text lines inside
    float lineX = doc.x + hs * 0.3f;
    float lineW = hs * 1.4f;
    for (int i = 0; i < 4; ++i) {
        float ly = doc.y + hs * 0.4f + i * hs * 0.45f;
        rr.drawLine({lineX, ly}, {lineX + lineW * (i == 3 ? 0.6f : 1.0f), ly},
                    col.withAlpha(0.6f), 1.0f);
    }
}

static void drawIconProxscan(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Radar sweep: circle outline with a sweep wedge
    float r = sz * 0.38f;
    rr.drawCircleOutline(c, r, col.withAlpha(0.5f), 1.0f, 16);
    // Sweep arc (bright wedge)
    rr.drawArc(c, 0.0f, r, -0.3f, 0.8f, col.withAlpha(0.7f), 8);
    // Centre dot
    rr.drawCircle(c, r * 0.12f, col, 6);
    // Cross-hairs
    rr.drawLine({c.x, c.y - r * 0.5f}, {c.x, c.y + r * 0.5f}, col.withAlpha(0.3f), 1.0f);
    rr.drawLine({c.x - r * 0.5f, c.y}, {c.x + r * 0.5f, c.y}, col.withAlpha(0.3f), 1.0f);
}

static void drawIconOverview(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Table/list: horizontal lines in a frame
    float hs = sz * 0.35f;
    Rect frame = {c.x - hs, c.y - hs * 0.9f, hs * 2, hs * 1.8f};
    rr.drawRectOutline(frame, col.withAlpha(0.6f), 1.0f);
    // Header line
    rr.drawRect({frame.x, frame.y, frame.w, hs * 0.35f}, col.withAlpha(0.3f));
    // Data rows
    for (int i = 0; i < 3; ++i) {
        float ly = frame.y + hs * 0.5f + i * hs * 0.45f;
        rr.drawLine({frame.x + 2, ly}, {frame.right() - 2, ly},
                    col.withAlpha(0.4f), 1.0f);
    }
}

static void drawIconChat(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Speech bubble: rounded rect with a tail
    float hs = sz * 0.35f;
    Rect bubble = {c.x - hs, c.y - hs * 0.8f, hs * 2, hs * 1.3f};
    rr.drawRectOutline(bubble, col, 1.5f);
    // Tail (triangle pointing down-left)
    float tx = bubble.x + hs * 0.4f;
    float ty = bubble.bottom();
    rr.drawLine({tx, ty}, {tx - hs * 0.3f, ty + hs * 0.5f}, col, 1.5f);
    rr.drawLine({tx - hs * 0.3f, ty + hs * 0.5f}, {tx + hs * 0.3f, ty}, col, 1.5f);
    // Dots (ellipsis inside bubble)
    float dotY = bubble.center().y;
    for (int i = 0; i < 3; ++i) {
        float dx = c.x - hs * 0.4f + i * hs * 0.4f;
        rr.drawCircle({dx, dotY}, 1.5f, col.withAlpha(0.7f), 4);
    }
}

static void drawIconDrone(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Drone: disc body with symmetric wing struts
    float r = sz * 0.2f;
    rr.drawCircle(c, r, col.withAlpha(0.6f), 8);
    rr.drawCircleOutline(c, r, col, 1.0f, 8);
    // Wing struts (4 arms extending outward)
    float armLen = sz * 0.35f;
    float angles[] = {0.6f, 2.5f, 3.7f, 5.6f};
    for (float a : angles) {
        Vec2 tip = c + Vec2(std::cos(a) * armLen, std::sin(a) * armLen);
        rr.drawLine(c + Vec2(std::cos(a) * r, std::sin(a) * r), tip, col, 1.5f);
        rr.drawCircle(tip, r * 0.35f, col.withAlpha(0.5f), 4);
    }
}

// Dispatch: draw the shape icon for a given sidebar slot index.
static void drawSidebarIconShape(AtlasRenderer& rr, int index,
                                  Vec2 centre, float slotSz,
                                  const Color& col) {
    switch (index) {
        case 0: drawIconInventory(rr, centre, slotSz, col); break;
        case 1: drawIconFitting(rr, centre, slotSz, col);   break;
        case 2: drawIconMarket(rr, centre, slotSz, col);    break;
        case 3: drawIconMission(rr, centre, slotSz, col);   break;
        case 4: drawIconProxscan(rr, centre, slotSz, col);     break;
        case 5: drawIconOverview(rr, centre, slotSz, col);  break;
        case 6: drawIconChat(rr, centre, slotSz, col);      break;
        case 7: drawIconDrone(rr, centre, slotSz, col);     break;
        default: {
            // Fallback: draw "?" letter
            float tw = rr.measureText("?");
            rr.drawText("?", {centre.x - tw * 0.5f, centre.y - 6.5f}, col);
            break;
        }
    }
}

// ── Sidebar Bar ─────────────────────────────────────────────────────

void sidebarBar(AtlasContext& ctx, float x, float width, float height,
               int icons, const std::function<void(int)>& callback,
               const bool* activeIcons, float skillQueuePct) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // ── Background: dark semi-transparent panel ─────────────────────
    Rect bar = {x, 0, width, height};
    rr.drawRect(bar, t.bgPanel.withAlpha(0.92f));
    // Right edge border (thin skeletal frame)
    rr.drawRect({x + width - 1.0f, 0, 1.0f, height}, t.borderSubtle);

    float iconY = 4.0f;
    float slotSz = width - 6.0f;
    float iconGap = 2.0f;
    float pad = 3.0f;

    // ── "A" Menu Button (top of sidebar) ────────────────────────────
    {
        Rect eRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID eID = hashID("sidebar_emenu");

        bool hovered = ctx.isHovered(eRect);
        if (hovered) ctx.setHot(eID);
        bool active = ctx.isActive(eID);

        // Accent-coloured background for the A button
        Color eBg = active  ? t.accentPrimary.withAlpha(0.5f) :
                    hovered ? t.accentPrimary.withAlpha(0.3f) :
                              t.accentPrimary.withAlpha(0.12f);
        rr.drawRect(eRect, eBg);
        rr.drawRectOutline(eRect, t.accentPrimary.withAlpha(0.4f));

        // "A" letter centered (Atlas menu icon)
        float tw = rr.measureText("A");
        float tx = eRect.x + (eRect.w - tw) * 0.5f;
        float ty = eRect.y + (eRect.h - 13.0f) * 0.5f;
        rr.drawText("A", {tx, ty}, t.accentPrimary);

        // Click behavior (currently informational — master menu)
        ctx.buttonBehavior(eRect, eID);
        if (ctx.isHot(eID)) {
            tooltip(ctx, "Atlas Menu");
        }

        iconY += slotSz + iconGap;
    }

    // ── Character Portrait Area ─────────────────────────────────────
    {
        float portraitSz = slotSz;
        Rect portraitRect = {x + pad, iconY, portraitSz, portraitSz};

        // Dark recessed area for portrait
        rr.drawRect(portraitRect, t.bgSecondary.withAlpha(0.8f));
        rr.drawRectOutline(portraitRect, t.borderSubtle);

        // Placeholder silhouette (draw a simple head/shoulders shape)
        Vec2 pCentre = portraitRect.center();
        float headR = portraitSz * 0.2f;
        rr.drawCircle(pCentre + Vec2(0, -headR * 0.5f), headR,
                       t.textMuted.withAlpha(0.4f), 12);
        // Shoulders arc
        rr.drawArc(pCentre + Vec2(0, headR * 0.8f),
                    headR * 0.8f, headR * 1.6f,
                    static_cast<float>(M_PI), 2.0f * static_cast<float>(M_PI),
                    t.textMuted.withAlpha(0.3f), 8);

        if (ctx.isHovered(portraitRect)) {
            tooltip(ctx, "Character Sheet");
        }

        iconY += portraitSz + 2.0f;
    }

    // ── Skill Queue Progress Bar ────────────────────────────────────
    {
        float barH = 3.0f;
        Rect sqRect = {x + pad, iconY, slotSz, barH};
        rr.drawRect(sqRect, t.bgSecondary.withAlpha(0.6f));
        float fill = std::max(0.0f, std::min(1.0f, skillQueuePct));
        if (fill > 0.0f) {
            rr.drawRect({sqRect.x, sqRect.y, sqRect.w * fill, barH},
                         t.accentSecondary.withAlpha(0.8f));
        }
        rr.drawRectOutline(sqRect, t.borderSubtle.withAlpha(0.4f), 1.0f);

        if (ctx.isHovered(sqRect)) {
            tooltip(ctx, "Skill Queue");
        }

        iconY += barH + 6.0f;
    }

    // ── Separator ───────────────────────────────────────────────────
    rr.drawRect({x + 6.0f, iconY, width - 12.0f, 1.0f},
                t.borderSubtle.withAlpha(0.6f));
    iconY += 5.0f;

    // ── Service icon definitions ────────────────────────────────────
    static const char* sidebarTooltips[] = {
        "Inventory",
        "Ship Fitting",
        "Market",
        "Missions",
        "Directional Scan",
        "Overview",
        "Chat",
        "Drones",
    };
    // Per-icon accent colours (Astralis lets users colour-code icons)
    static const Color iconAccentColors[] = {
        Color(0.35f, 0.75f, 0.45f, 1.0f),  // Inventory: green
        Color(0.85f, 0.50f, 0.25f, 1.0f),  // Fitting: orange
        Color(0.40f, 0.65f, 0.90f, 1.0f),  // Market: blue
        Color(0.80f, 0.75f, 0.30f, 1.0f),  // Missions: gold
        Color(0.60f, 0.80f, 0.90f, 1.0f),  // Proxscan: light cyan
        Color(0.50f, 0.70f, 0.85f, 1.0f),  // Overview: steel blue
        Color(0.65f, 0.55f, 0.85f, 1.0f),  // Chat: purple
        Color(0.45f, 0.80f, 0.65f, 1.0f),  // Drones: teal
    };

    // ── Top group: Inventory & Fitting (icons 0–1) ──────────────────
    int topGroup = std::min(icons, 2);
    for (int i = 0; i < topGroup; ++i) {
        Rect iconRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID iconID = hashID("sidebar") ^ static_cast<uint32_t>(i);

        bool isOpen = activeIcons ? activeIcons[i] : false;
        bool hovered = ctx.isHovered(iconRect);
        if (hovered) ctx.setHot(iconID);
        bool pressing = ctx.isActive(iconID);

        // Background: show persistent highlight when panel is open
        Color bg(0, 0, 0, 0);
        Color accentCol = (i < 8) ? iconAccentColors[i] : t.accentPrimary;
        if (pressing) {
            bg = accentCol.withAlpha(0.35f);
        } else if (isOpen) {
            bg = accentCol.withAlpha(0.18f);
        } else if (hovered) {
            bg = t.hover;
        }
        rr.drawRect(iconRect, bg);

        // Left accent bar: bright when open, subtle on hover
        if (isOpen) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol);
        } else if (hovered) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol.withAlpha(0.5f));
        }

        // Shape-based icon (Astralis Photon style)
        Color iconCol = isOpen ? accentCol : (hovered ? t.textPrimary : t.textSecondary);
        drawSidebarIconShape(rr, i, iconRect.center(), slotSz, iconCol);

        if (ctx.buttonBehavior(iconRect, iconID)) {
            if (callback) callback(i);
        }

        if (ctx.isHot(iconID) && i < 8) {
            tooltip(ctx, sidebarTooltips[i]);
        }

        iconY += slotSz + iconGap;
    }

    // ── Separator ───────────────────────────────────────────────────
    if (topGroup > 0 && icons > topGroup) {
        iconY += 2.0f;
        rr.drawRect({x + 6.0f, iconY, width - 12.0f, 1.0f},
                    t.borderSubtle.withAlpha(0.6f));
        iconY += 5.0f;
    }

    // ── Middle group: Market, Missions, Proxscan, Overview (icons 2–5) ─
    int midGroup = std::min(icons, 6);
    for (int i = topGroup; i < midGroup; ++i) {
        Rect iconRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID iconID = hashID("sidebar") ^ static_cast<uint32_t>(i);

        bool isOpen = activeIcons ? activeIcons[i] : false;
        bool hovered = ctx.isHovered(iconRect);
        if (hovered) ctx.setHot(iconID);
        bool pressing = ctx.isActive(iconID);

        Color accentCol = (i < 8) ? iconAccentColors[i] : t.accentPrimary;
        Color bg(0, 0, 0, 0);
        if (pressing) {
            bg = accentCol.withAlpha(0.35f);
        } else if (isOpen) {
            bg = accentCol.withAlpha(0.18f);
        } else if (hovered) {
            bg = t.hover;
        }
        rr.drawRect(iconRect, bg);

        if (isOpen) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h}, accentCol);
        } else if (hovered) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol.withAlpha(0.5f));
        }

        // Shape-based icon (Astralis Photon style)
        Color iconCol = isOpen ? accentCol : (hovered ? t.textPrimary : t.textSecondary);
        drawSidebarIconShape(rr, i, iconRect.center(), slotSz, iconCol);

        if (ctx.buttonBehavior(iconRect, iconID)) {
            if (callback) callback(i);
        }

        if (ctx.isHot(iconID) && i < 8) {
            tooltip(ctx, sidebarTooltips[i]);
        }

        iconY += slotSz + iconGap;
    }

    // ── Separator before bottom group ───────────────────────────────
    if (midGroup > topGroup && icons > midGroup) {
        iconY += 2.0f;
        rr.drawRect({x + 6.0f, iconY, width - 12.0f, 1.0f},
                    t.borderSubtle.withAlpha(0.6f));
        iconY += 5.0f;
    }

    // ── Bottom group: Chat, Drones (icons 6+) ───────────────────────
    for (int i = midGroup; i < icons; ++i) {
        Rect iconRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID iconID = hashID("sidebar") ^ static_cast<uint32_t>(i);

        bool isOpen = activeIcons ? activeIcons[i] : false;
        bool hovered = ctx.isHovered(iconRect);
        if (hovered) ctx.setHot(iconID);
        bool pressing = ctx.isActive(iconID);

        Color accentCol = (i < 8) ? iconAccentColors[i] : t.accentPrimary;
        Color bg(0, 0, 0, 0);
        if (pressing) {
            bg = accentCol.withAlpha(0.35f);
        } else if (isOpen) {
            bg = accentCol.withAlpha(0.18f);
        } else if (hovered) {
            bg = t.hover;
        }
        rr.drawRect(iconRect, bg);

        if (isOpen) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h}, accentCol);
        } else if (hovered) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol.withAlpha(0.5f));
        }

        // Shape-based icon (Astralis Photon style)
        Color iconCol = isOpen ? accentCol : (hovered ? t.textPrimary : t.textSecondary);
        drawSidebarIconShape(rr, i, iconRect.center(), slotSz, iconCol);

        if (ctx.buttonBehavior(iconRect, iconID)) {
            if (callback) callback(i);
        }

        if (ctx.isHot(iconID) && i < 8) {
            tooltip(ctx, sidebarTooltips[i]);
        }

        iconY += slotSz + iconGap;
    }

    // Consume mouse clicks within the sidebar AFTER icon processing,
    // to prevent click-through to the 3D world while still allowing
    // icon buttons to receive their clicks first
    if (ctx.isHovered(bar) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
        ctx.consumeMouse();
    }
}

} // namespace atlas
