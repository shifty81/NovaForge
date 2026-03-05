#include "ui/atlas/atlas_widgets.h"

#include <cstdio>

static constexpr float METERS_PER_AU = 149597870700.0f;

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

// ── Overview ────────────────────────────────────────────────────────

void overviewHeader(AtlasContext& ctx, const Rect& r,
                    const std::vector<std::string>& tabs,
                    int activeTab) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Header background
    rr.drawRect(r, t.bgHeader);

    // Tabs
    float tabX = r.x + 4.0f;
    float tabH = r.h - 4.0f;
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i) {
        float tw = rr.measureText(tabs[i]) + 16.0f;
        Rect tabRect = {tabX, r.y + 2.0f, tw, tabH};

        if (i == activeTab) {
            rr.drawRect(tabRect, t.selection);
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else {
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textSecondary);
        }
        tabX += tw + 2.0f;
    }

    // Bottom border
    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f}, t.borderSubtle);
}

bool overviewRow(AtlasContext& ctx, const Rect& r,
                 const OverviewEntry& entry, bool isAlternate) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = hashID(entry.name.c_str());
    bool clicked = ctx.buttonBehavior(r, id);

    // Photon-style: neutral row background (alternating for scanability)
    Color bg = isAlternate ? t.bgSecondary.withAlpha(0.2f)
                           : Color(0, 0, 0, 0);
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(r, bg);

    // Photon selection: thin 2px accent bar on the left (not row fill)
    if (entry.selected) {
        rr.drawRect({r.x, r.y, t.selectionBarWidth, r.h}, t.accentPrimary);
    }

    // Standing/threat icon indicator (small square — color the icon, not the row)
    float iconEnd = r.x + 16.0f;
    rr.drawRect({r.x + 4.0f, r.y + (r.h - 8.0f) * 0.5f, 8.0f, 8.0f},
                entry.standingColor);

    // Columns: Distance | Name | Type — proportional to row width
    float textY = r.y + (r.h - 13.0f) * 0.5f;
    float usableW = r.w - 16.0f;  // subtract icon area
    float distColW = usableW * 0.28f;
    float nameColW = usableW * 0.40f;
    float typeColW = usableW * 0.32f;

    float distColStart = iconEnd;
    float nameColStart = distColStart + distColW;
    float typeColStart = nameColStart + nameColW;

    // Distance (right-aligned in its column for readability)
    char distBuf[32];
    if (entry.distance >= METERS_PER_AU * 0.01f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.1f AU", entry.distance / METERS_PER_AU);
    } else if (entry.distance >= 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f km", entry.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f m", entry.distance);
    }
    float distW = rr.measureText(distBuf);
    float distEnd = nameColStart - 4.0f;
    rr.drawText(distBuf, {distEnd - distW, textY}, t.textSecondary);

    // Name — truncate with "..." if it exceeds column width
    float nameMaxW = nameColW - 6.0f;
    std::string displayName = truncateText(rr, entry.name, nameMaxW);
    rr.drawText(displayName, {nameColStart + 2.0f, textY}, t.textPrimary);

    // Type — truncate with "..." if it exceeds column width
    float typeMaxW = typeColW - 4.0f;
    std::string displayType = truncateText(rr, entry.type, typeMaxW);
    rr.drawText(displayType, {typeColStart + 2.0f, textY}, t.textSecondary);

    return clicked;
}

// ── Overview Header Interactive ─────────────────────────────────────

int overviewHeaderInteractive(AtlasContext& ctx, const Rect& r,
                              const std::vector<std::string>& tabs,
                              int activeTab) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int clickedTab = -1;

    // Header background
    rr.drawRect(r, t.bgHeader);

    // Photon-style tabs: text-only with accent underline on active
    float tabX = r.x + 4.0f;
    float tabH = r.h - 4.0f;
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i) {
        float tw = rr.measureText(tabs[i]) + 16.0f;
        Rect tabRect = {tabX, r.y + 2.0f, tw, tabH};

        WidgetID tabID = hashID("ovtab") ^ static_cast<uint32_t>(i);
        bool clicked = ctx.buttonBehavior(tabRect, tabID);
        if (clicked) clickedTab = i;

        bool hovered = ctx.isHot(tabID);

        if (i == activeTab) {
            // Active tab: bright text + accent underline indicator
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
            rr.drawRect({tabX + 4.0f, tabRect.bottom() - 2.0f, tw - 8.0f, 2.0f},
                        t.accentPrimary);
        } else if (hovered) {
            rr.drawRect(tabRect, t.hover);
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else {
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textSecondary);
        }
        tabX += tw + 2.0f;
    }

    // Bottom border
    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f}, t.borderSubtle);

    return clickedTab;
}


// ── Tab Bar ─────────────────────────────────────────────────────────

int tabBar(AtlasContext& ctx, const Rect& r,
           const std::vector<std::string>& labels, int activeIdx) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int clicked = -1;

    // Background
    rr.drawRect(r, t.bgHeader);

    float tabX = r.x + 4.0f;
    float tabH = r.h - 4.0f;
    for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
        float tw = rr.measureText(labels[i]) + 16.0f;
        Rect tabRect = {tabX, r.y + 2.0f, tw, tabH};

        WidgetID tabID = hashID("tab") ^ static_cast<uint32_t>(i);
        if (ctx.buttonBehavior(tabRect, tabID)) clicked = i;

        bool hovered = ctx.isHot(tabID);

        if (i == activeIdx) {
            rr.drawText(labels[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
            rr.drawRect({tabX + 4.0f, tabRect.bottom() - 2.0f, tw - 8.0f, 2.0f},
                        t.accentPrimary);
        } else if (hovered) {
            rr.drawRect(tabRect, t.hover);
            rr.drawText(labels[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else {
            rr.drawText(labels[i], {tabX + 8.0f, r.y + 5.0f}, t.textSecondary);
        }
        tabX += tw + 2.0f;
    }

    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f}, t.borderSubtle);
    return clicked;
}

} // namespace atlas
