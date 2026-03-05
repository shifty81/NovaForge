#include "ui/atlas/atlas_widgets.h"

#include <cstdio>

static constexpr float METERS_PER_AU = 149597870700.0f;

namespace atlas {

// ── Target Card ─────────────────────────────────────────────────────

bool targetCard(AtlasContext& ctx, const Rect& r,
                const TargetCardInfo& info) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = hashID(info.name.c_str());
    bool clicked = ctx.buttonBehavior(r, id);

    // Card background
    Color bg = t.bgPanel;
    if (info.isActive) bg = t.selection;
    else if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(r, bg);

    // Border (red if hostile, blue if friendly)
    Color borderColor = info.isHostile ? t.hostile : t.borderNormal;
    if (info.isActive) borderColor = t.accentPrimary;
    rr.drawRectOutline(r, borderColor, 2.0f);

    // Mini shield/armor/hull bars (horizontal, stacked at bottom)
    float barH = 3.0f;
    float barW = r.w - 8.0f;
    float barX = r.x + 4.0f;
    float barY = r.bottom() - 16.0f;

    rr.drawProgressBar({barX, barY, barW, barH}, info.shieldPct,
                       t.shield, t.bgSecondary.withAlpha(0.3f));
    rr.drawProgressBar({barX, barY + barH + 1.0f, barW, barH}, info.armorPct,
                       t.armor, t.bgSecondary.withAlpha(0.3f));
    rr.drawProgressBar({barX, barY + 2*(barH + 1.0f), barW, barH}, info.hullPct,
                       t.hull, t.bgSecondary.withAlpha(0.3f));

    // Name (truncated to fit)
    std::string displayName = info.name;
    if (rr.measureText(displayName) > r.w - 8.0f) {
        while (displayName.size() > 3 &&
               rr.measureText(displayName + "..") > r.w - 8.0f) {
            displayName.pop_back();
        }
        displayName += "..";
    }
    rr.drawText(displayName, {r.x + 4.0f, r.bottom() - 30.0f},
                t.textPrimary);

    // Distance
    char distBuf[32];
    if (info.distance >= 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f km", info.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f m", info.distance);
    }
    float tw = rr.measureText(distBuf);
    rr.drawText(distBuf, {r.x + (r.w - tw) * 0.5f, r.y + 4.0f},
                t.textSecondary);

    return clicked;
}

// ── Selected Item Panel ─────────────────────────────────────────────

void selectedItemPanel(AtlasContext& ctx, const Rect& r,
                       const SelectedItemInfo& info) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    rr.drawRect(r, t.bgPanel);
    rr.drawRectOutline(r, t.borderSubtle);

    // Header: "Selected Item"
    rr.drawRect({r.x, r.y, r.w, t.headerHeight}, t.bgHeader);
    rr.drawText("Selected Item", {r.x + t.padding, r.y + 7.0f},
                t.textSecondary);
    rr.drawRect({r.x, r.y + t.headerHeight - 1.0f, r.w, 1.0f},
                t.accentDim);

    // Name
    float textY = r.y + t.headerHeight + 8.0f;
    rr.drawText(info.name, {r.x + t.padding, textY}, t.textPrimary);

    // Distance
    textY += 18.0f;
    char distBuf[64];
    std::snprintf(distBuf, sizeof(distBuf), "Distance: %.0f %s",
                  info.distance, info.distanceUnit.c_str());
    rr.drawText(distBuf, {r.x + t.padding, textY}, t.textSecondary);

    // Action buttons row (orbit, approach, warp, look at, info)
    float btnY = textY + 24.0f;
    float btnSz = 24.0f;
    float btnGap = 6.0f;
    float btnX = r.x + t.padding;
    const char* actions[] = {"O", ">>", "W", "i"};
    for (int i = 0; i < 4; ++i) {
        Rect btn = {btnX, btnY, btnSz, btnSz};
        button(ctx, actions[i], btn);
        btnX += btnSz + btnGap;
    }
}

// ── Info Panel ──────────────────────────────────────────────────────

void infoPanelDraw(AtlasContext& ctx, PanelState& state,
                   const InfoPanelData& data) {
    if (!state.open || data.isEmpty()) return;

    PanelFlags flags;
    flags.showHeader   = true;
    flags.showClose    = true;
    flags.showMinimize = true;
    flags.drawBorder   = true;

    if (!panelBeginStateful(ctx, "Show Info", state, flags)) {
        panelEnd(ctx);
        return;
    }

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();
    float hh = t.headerHeight;
    float x = state.bounds.x + t.padding;
    float y = state.bounds.y + hh + 8.0f;
    float contentW = state.bounds.w - t.padding * 2.0f;

    // Entity name
    rr.drawText(data.name, {x, y}, t.accentPrimary);
    y += 18.0f;

    // Type
    char typeBuf[128];
    std::snprintf(typeBuf, sizeof(typeBuf), "Type: %s", data.type.c_str());
    rr.drawText(typeBuf, {x, y}, t.textSecondary);
    y += 16.0f;

    // Faction
    if (!data.faction.empty()) {
        char facBuf[128];
        std::snprintf(facBuf, sizeof(facBuf), "Faction: %s", data.faction.c_str());
        rr.drawText(facBuf, {x, y}, t.textSecondary);
        y += 16.0f;
    }

    // Separator
    rr.drawRect({x, y, contentW, 1.0f}, t.borderSubtle);
    y += 8.0f;

    // Distance
    char distBuf[64];
    if (data.distance < 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.0f m", data.distance);
    } else if (data.distance < 1000000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.1f km", data.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.2f AU", data.distance / METERS_PER_AU);
    }
    rr.drawText(distBuf, {x, y}, t.textPrimary);
    y += 16.0f;

    // Velocity
    if (data.velocity > 0.0f) {
        char velBuf[64];
        std::snprintf(velBuf, sizeof(velBuf), "Velocity: %.0f m/s", data.velocity);
        rr.drawText(velBuf, {x, y}, t.textPrimary);
        y += 16.0f;
    }

    // Signature radius
    if (data.signature > 0.0f) {
        char sigBuf[64];
        std::snprintf(sigBuf, sizeof(sigBuf), "Signature: %.0f m", data.signature);
        rr.drawText(sigBuf, {x, y}, t.textPrimary);
        y += 16.0f;
    }

    // Health bars
    if (data.hasHealth) {
        rr.drawRect({x, y, contentW, 1.0f}, t.borderSubtle);
        y += 6.0f;

        float barW = contentW - 20.0f;
        float barH = 12.0f;
        Color bgBar = t.bgSecondary.withAlpha(0.4f);

        rr.drawText("S", {x, y + 1.0f}, t.textSecondary);
        rr.drawProgressBar({x + 14.0f, y, barW, barH}, data.shieldPct,
                           t.shield, bgBar);
        y += barH + 4.0f;

        rr.drawText("A", {x, y + 1.0f}, t.textSecondary);
        rr.drawProgressBar({x + 14.0f, y, barW, barH}, data.armorPct,
                           t.armor, bgBar);
        y += barH + 4.0f;

        rr.drawText("H", {x, y + 1.0f}, t.textSecondary);
        rr.drawProgressBar({x + 14.0f, y, barW, barH}, data.hullPct,
                           t.hull, bgBar);
        y += barH + 4.0f;
    }

    panelEnd(ctx);
}

} // namespace atlas
