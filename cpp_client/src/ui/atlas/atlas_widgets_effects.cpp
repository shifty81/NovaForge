#include "ui/atlas/atlas_widgets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atlas {

// ── Notification Toast ──────────────────────────────────────────────

void notification(AtlasContext& ctx, const std::string& text,
                  const Color& color) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float winW = static_cast<float>(ctx.input().windowW);
    float notifW = rr.measureText(text) + 32.0f;
    float notifH = 28.0f;
    float notifX = (winW - notifW) * 0.5f;
    float notifY = 40.0f;

    // Background
    rr.drawRect({notifX, notifY, notifW, notifH}, t.bgPanel);
    rr.drawRectOutline({notifX, notifY, notifW, notifH}, t.borderSubtle);

    // Left accent bar
    Color accentCol = (color.a > 0.01f) ? color : t.accentPrimary;
    rr.drawRect({notifX, notifY, 3.0f, notifH}, accentCol);

    // Text
    float textY = notifY + (notifH - 13.0f) * 0.5f;
    rr.drawText(text, {notifX + 12.0f, textY}, t.textPrimary);
}

// ── Combat Log Widget ───────────────────────────────────────────────

void combatLogWidget(AtlasContext& ctx, const Rect& r,
                     const std::vector<std::string>& messages,
                     float& scrollOff, int maxVisible) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Panel background
    rr.drawRect(r, t.bgPanel);
    rr.drawRectOutline(r, t.borderSubtle, t.borderWidth);

    // Header
    float hh = 18.0f;
    rr.drawRect({r.x, r.y, r.w, hh}, t.bgHeader);
    rr.drawText("Combat Log", {r.x + 6.0f, r.y + 3.0f}, t.textSecondary);

    // Content area
    float contentY = r.y + hh + 2.0f;
    float contentH = r.h - hh - 2.0f;
    float rowH = 16.0f;
    int visRows = maxVisible > 0 ? maxVisible : static_cast<int>(contentH / rowH);
    if (visRows <= 0) visRows = 1;

    // Handle scrolling
    if (ctx.isHovered(r)) {
        scrollOff -= ctx.input().scrollY * rowH * 2.0f;
    }

    int total = static_cast<int>(messages.size());
    float maxScroll = std::max(0.0f, (total - visRows) * rowH);
    if (scrollOff < 0.0f) scrollOff = 0.0f;
    if (scrollOff > maxScroll) scrollOff = maxScroll;

    // Draw visible messages (newest at bottom)
    int firstRow = static_cast<int>(scrollOff / rowH);
    for (int i = 0; i < visRows + 1 && (firstRow + i) < total; ++i) {
        int msgIdx = firstRow + i;
        float y = contentY + i * rowH - std::fmod(scrollOff, rowH);
        if (y + rowH < contentY || y > r.bottom()) continue;

        // Fade older messages
        float age = static_cast<float>(total - 1 - msgIdx) / std::max(1.0f, static_cast<float>(total));
        float alpha = 1.0f - age * 0.4f;
        Color textCol = t.textPrimary.withAlpha(alpha);

        rr.drawText(messages[msgIdx], {r.x + 6.0f, y + 1.0f}, textCol);
    }

    // Scrollbar
    if (total > visRows) {
        Rect scrollTrack = {r.right() - t.scrollbarWidth, contentY,
                           t.scrollbarWidth, contentH};
        scrollbar(ctx, scrollTrack, scrollOff, total * rowH, contentH);
    }
}

// ── Damage Flash Overlay ────────────────────────────────────────────

void damageFlashOverlay(AtlasContext& ctx, Vec2 centre, float radius,
                        int layer, float intensity) {
    if (intensity <= 0.0f) return;

    const Theme& t = ctx.theme();
    Color flashColor;
    switch (layer) {
        case 0: flashColor = t.shield;  break;  // blue
        case 1: flashColor = t.armor;   break;  // gold
        case 2: flashColor = t.hull;    break;  // red
        default: flashColor = t.shield; break;
    }

    float alpha = intensity * 0.35f;

    // Draw concentric rings that fade outward
    int rings = 3;
    for (int i = 0; i < rings; ++i) {
        float innerR = radius + i * 15.0f;
        float outerR = innerR + 12.0f;
        float ringAlpha = alpha * (1.0f - static_cast<float>(i) / rings);
        Color ringColor = flashColor.withAlpha(ringAlpha);

        ctx.renderer().drawArc(centre, innerR, outerR,
                               0.0f, 2.0f * static_cast<float>(M_PI),
                               ringColor, 32);
    }
}

// ── Drone Status Bar ────────────────────────────────────────────────

void droneStatusBar(AtlasContext& ctx, const Rect& r,
                    int inSpace, int inBay,
                    int bandwidthUsed, int bandwidthMax) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Background
    rr.drawRect(r, t.bgPanel);
    rr.drawRectOutline(r, t.borderSubtle, t.borderWidth);

    float pad = 4.0f;
    float textY = r.y + (r.h - 13.0f) * 0.5f;

    // Drone counts
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Drones: %d/%d", inSpace, inSpace + inBay);
    rr.drawText(buf, {r.x + pad, textY}, t.textPrimary);

    // Bandwidth bar
    float barX = r.x + 120.0f;
    float barW = r.w - 120.0f - pad * 2 - 60.0f;
    float barH = 8.0f;
    float barY = r.y + (r.h - barH) * 0.5f;
    float fraction = bandwidthMax > 0
        ? static_cast<float>(bandwidthUsed) / static_cast<float>(bandwidthMax)
        : 0.0f;
    Color barColor = fraction > 0.9f ? t.danger : t.accentSecondary;
    rr.drawProgressBar({barX, barY, barW, barH}, fraction, barColor, t.bgSecondary);

    // Bandwidth label
    std::snprintf(buf, sizeof(buf), "%d/%d Mb", bandwidthUsed, bandwidthMax);
    rr.drawText(buf, {barX + barW + pad, textY}, t.textSecondary);
}

// ── Fleet Broadcast Banner ──────────────────────────────────────────

void fleetBroadcastBanner(AtlasContext& ctx, const Rect& r,
                          const std::vector<FleetBroadcast>& broadcasts) {
    if (broadcasts.empty()) return;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float rowH = 20.0f;
    int maxShow = static_cast<int>(r.h / rowH);
    if (maxShow <= 0) maxShow = 1;

    // Show most recent broadcasts (newest last → draw bottom to top)
    int start = std::max(0, static_cast<int>(broadcasts.size()) - maxShow);
    int count = static_cast<int>(broadcasts.size()) - start;

    for (int i = 0; i < count; ++i) {
        const auto& bc = broadcasts[start + i];
        float safeMaxAge = bc.maxAge > 0.0f ? bc.maxAge : 0.01f;
        float alpha = 1.0f - (bc.age / safeMaxAge);
        if (alpha <= 0.0f) continue;

        float y = r.y + i * rowH;

        // Accent left border
        Color accentFade = bc.color.withAlpha(alpha * 0.8f);
        rr.drawRect({r.x, y, 3.0f, rowH - 2.0f}, accentFade);

        // Background
        rr.drawRect({r.x + 3.0f, y, r.w - 3.0f, rowH - 2.0f},
                    t.bgPanel.withAlpha(alpha * 0.7f));

        // Sender + message
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s: %s",
                      bc.sender.c_str(), bc.message.c_str());
        rr.drawText(buf, {r.x + 8.0f, y + 3.0f},
                    t.textPrimary.withAlpha(alpha));
    }
}

} // namespace atlas
