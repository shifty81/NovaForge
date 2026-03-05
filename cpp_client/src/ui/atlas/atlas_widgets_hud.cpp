#include "ui/atlas/atlas_widgets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atlas {

// ── Ship Status Arcs ────────────────────────────────────────────────

void shipStatusArcs(AtlasContext& ctx, Vec2 centre, float outerR,
                    float shieldPct, float armorPct, float hullPct) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Arc layout (from screenshot):
    // - Arcs sweep the TOP half only (π to 0, i.e. left-to-right)
    // - Shield = outermost, Armor = middle, Hull = innermost
    // - Ring thickness ~8px each with ~2px gap
    float ringW = 8.0f;
    float gap = 2.0f;

    // Start/end angles: top semicircle = π to 0 (sweep right)
    float startAngle = static_cast<float>(M_PI);
    float endAngle   = 0.0f;

    struct ArcDef {
        float pct; Color full; Color empty; float innerR; float outerR;
    };

    float shieldInner = outerR - ringW;
    float armorOuter  = shieldInner - gap;
    float armorInner  = armorOuter - ringW;
    float hullOuter   = armorInner - gap;
    float hullInner   = hullOuter - ringW;

    ArcDef arcs[] = {
        {shieldPct, t.shield, t.bgSecondary.withAlpha(0.3f), shieldInner, outerR},
        {armorPct,  t.armor,  t.bgSecondary.withAlpha(0.3f), armorInner,  armorOuter},
        {hullPct,   t.hull,   t.bgSecondary.withAlpha(0.3f), hullInner,   hullOuter},
    };

    for (const auto& a : arcs) {
        // Draw empty (background) arc
        rr.drawArc(centre, a.innerR, a.outerR, startAngle, endAngle,
                   a.empty, 32);
        // Draw filled arc
        if (a.pct > 0.001f) {
            float fillEnd = startAngle + (endAngle - startAngle) * a.pct;
            rr.drawArc(centre, a.innerR, a.outerR, startAngle, fillEnd,
                       a.full, 32);
        }
    }

    // Percentage labels (left of arcs, stacked vertically)
    char buf[16];
    float labelX = centre.x - outerR - 40.0f;
    float labelBaseY = centre.y - outerR * 0.5f;

    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(shieldPct * 100));
    rr.drawText(buf, {labelX, labelBaseY}, t.shield);

    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(armorPct * 100));
    rr.drawText(buf, {labelX, labelBaseY + 16.0f}, t.armor);

    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(hullPct * 100));
    rr.drawText(buf, {labelX, labelBaseY + 32.0f}, t.hull);
}

// ── Capacitor Ring ──────────────────────────────────────────────────

void capacitorRing(AtlasContext& ctx, Vec2 centre,
                   float innerR, float outerR,
                   float fraction, int segments) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float gapAngle = 0.03f;  // small gap between segments
    float totalAngle = 2.0f * static_cast<float>(M_PI);
    float segAngle = totalAngle / segments - gapAngle;
    int filledCount = static_cast<int>(fraction * segments + 0.5f);

    for (int i = 0; i < segments; ++i) {
        float a0 = totalAngle * i / segments;
        float a1 = a0 + segAngle;
        bool filled = (i < filledCount);
        Color c = filled ? t.capacitor : t.bgSecondary.withAlpha(0.25f);
        rr.drawArc(centre, innerR, outerR, a0, a1, c, 4);
    }
}

// ── Module Slot ─────────────────────────────────────────────────────

bool moduleSlot(AtlasContext& ctx, Vec2 centre, float radius,
                bool active, float cooldownPct, const Color& color) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    Rect hitbox = {centre.x - radius, centre.y - radius,
                   radius * 2, radius * 2};
    WidgetID id = hashID("mod") ^ static_cast<uint32_t>(centre.x * 1000)
                                ^ static_cast<uint32_t>(centre.y * 1000);
    bool clicked = ctx.buttonBehavior(hitbox, id);

    // Background circle
    Color bg = active ? color.withAlpha(0.4f) : t.bgSecondary.withAlpha(0.6f);
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawCircle(centre, radius, bg);

    // Border ring
    Color border = active ? color : t.borderNormal;
    rr.drawCircleOutline(centre, radius, border, 1.5f);

    // Cooldown sweep overlay (clockwise from top)
    if (cooldownPct > 0.001f && cooldownPct < 1.0f) {
        float startA = -static_cast<float>(M_PI) * 0.5f;  // top
        float sweepA = 2.0f * static_cast<float>(M_PI) * cooldownPct;
        rr.drawArc(centre, 0.0f, radius - 1.0f, startA, startA + sweepA,
                   Color(0, 0, 0, 0.5f), 16);
    }

    // Active glow dot in centre
    if (active) {
        rr.drawCircle(centre, radius * 0.3f, color);
    }

    return clicked;
}

// ── Module Slot with Overheat ───────────────────────────────────────

bool moduleSlotEx(AtlasContext& ctx, Vec2 centre, float radius,
                  bool active, float cooldownPct, const Color& color,
                  float overheatPct, float time) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    Rect hitbox = {centre.x - radius, centre.y - radius,
                   radius * 2, radius * 2};
    WidgetID id = hashID("modex") ^ static_cast<uint32_t>(centre.x * 1000)
                                  ^ static_cast<uint32_t>(centre.y * 1000);
    bool clicked = ctx.buttonBehavior(hitbox, id);

    // Background circle
    Color bg = active ? color.withAlpha(0.4f) : t.bgSecondary.withAlpha(0.6f);
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawCircle(centre, radius, bg);

    // Overheat glow ring: pulsing red/orange border when overheated
    if (overheatPct > 0.01f) {
        // Pulse frequency increases with heat level
        float pulseRate = 2.0f + overheatPct * 4.0f;
        float pulse = 0.5f + 0.5f * std::sin(time * pulseRate * 2.0f * static_cast<float>(M_PI));
        float alpha = 0.3f + overheatPct * 0.7f * pulse;

        // Lerp from orange to red as heat increases
        Color heatColor = {1.0f, 0.5f * (1.0f - overheatPct), 0.0f, alpha};
        rr.drawCircleOutline(centre, radius + 1.0f, heatColor, 2.0f);

        // Inner heat tint
        rr.drawCircle(centre, radius - 1.0f, heatColor.withAlpha(alpha * 0.15f));
    }

    // Normal border ring
    Color border = active ? color : t.borderNormal;
    if (overheatPct >= 1.0f) {
        border = Color(0.5f, 0.1f, 0.1f, 0.8f);  // burnt out: dim red
    }
    rr.drawCircleOutline(centre, radius, border, 1.5f);

    // Cooldown sweep overlay (clockwise from top)
    if (cooldownPct > 0.001f && cooldownPct < 1.0f) {
        float startA = -static_cast<float>(M_PI) * 0.5f;
        float sweepA = 2.0f * static_cast<float>(M_PI) * cooldownPct;
        rr.drawArc(centre, 0.0f, radius - 1.0f, startA, startA + sweepA,
                   Color(0, 0, 0, 0.5f), 16);
    }

    // Active glow dot in centre
    if (active) {
        rr.drawCircle(centre, radius * 0.3f, color);
    }

    return clicked;
}

// ── Capacitor Ring Animated ─────────────────────────────────────────

void capacitorRingAnimated(AtlasContext& ctx, Vec2 centre,
                           float innerR, float outerR,
                           float targetFrac, float& displayFrac,
                           float dt, int segments, float lerpSpeed) {
    // Exponential ease toward target
    float diff = targetFrac - displayFrac;
    displayFrac += diff * std::min(1.0f, lerpSpeed * dt);
    // Snap when close enough to avoid floating imprecision
    if (std::fabs(diff) < 0.001f) displayFrac = targetFrac;

    // Clamp
    displayFrac = std::max(0.0f, std::min(1.0f, displayFrac));

    // Draw using the existing capacitorRing with smoothed value
    capacitorRing(ctx, centre, innerR, outerR, displayFrac, segments);
}

// ── Speed Indicator ─────────────────────────────────────────────────

int speedIndicator(AtlasContext& ctx, Vec2 pos,
                    float currentSpeed, float maxSpeed) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int result = 0;

    // Background bar
    float barW = 120.0f, barH = 20.0f;
    Rect bar = {pos.x - barW * 0.5f, pos.y, barW, barH};
    rr.drawRect(bar, t.bgSecondary.withAlpha(0.7f));
    rr.drawRectOutline(bar, t.borderSubtle);

    // Fill based on speed fraction
    float frac = (maxSpeed > 0.0f) ? currentSpeed / maxSpeed : 0.0f;
    frac = std::max(0.0f, std::min(1.0f, frac));
    if (frac > 0.0f) {
        rr.drawRect({bar.x, bar.y, bar.w * frac, bar.h},
                    t.accentDim.withAlpha(0.5f));
    }

    // Speed text
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f m/s", currentSpeed);
    float tw = rr.measureText(buf);
    rr.drawText(buf, {pos.x - tw * 0.5f, pos.y + 3.0f}, t.textPrimary);

    // +/- buttons
    float btnSz = 16.0f;
    Rect minus = {bar.x - btnSz - 4.0f, pos.y + 2.0f, btnSz, btnSz};
    Rect plus  = {bar.right() + 4.0f,   pos.y + 2.0f, btnSz, btnSz};
    if (button(ctx, "-", minus)) result = -1;
    if (button(ctx, "+", plus))  result =  1;

    return result;
}

// ── Warp Progress Indicator ────────────────────────────────────────

void warpProgressIndicator(AtlasContext& ctx, Vec2 pos,
                           int phase, float progress, float speedAU) {
    if (phase <= 0) return;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float barW = 200.0f, barH = 22.0f;
    float totalH = barH + 18.0f;  // bar + text row
    Rect bar = {pos.x - barW * 0.5f, pos.y, barW, barH};

    // Phase-dependent accent colour
    Color phaseColor;
    const char* phaseLabel = "";
    switch (phase) {
        case 1:
            phaseColor = {0.3f, 0.8f, 0.8f, 0.9f};  // teal
            phaseLabel = "ALIGNING";
            break;
        case 2:
            phaseColor = {0.3f, 0.5f, 1.0f, 0.9f};  // blue
            phaseLabel = "ACCELERATING";
            break;
        case 3:
            phaseColor = {0.4f, 0.6f, 1.0f, 0.95f};  // bright blue
            phaseLabel = "WARP";
            break;
        case 4:
            phaseColor = {0.5f, 0.5f, 0.8f, 0.8f};  // fading purple-blue
            phaseLabel = "DECELERATING";
            break;
        default:
            phaseColor = t.accentPrimary;
            phaseLabel = "WARP";
            break;
    }

    // Background
    rr.drawRoundedRect(bar, t.bgPanel.withAlpha(0.85f), 3.0f);
    rr.drawRoundedRectOutline(bar, phaseColor.withAlpha(0.6f), 3.0f);

    // Fill bar based on progress
    float frac = std::max(0.0f, std::min(1.0f, progress));
    if (frac > 0.0f) {
        Rect fill = {bar.x + 2.0f, bar.y + 2.0f,
                     (bar.w - 4.0f) * frac, bar.h - 4.0f};
        rr.drawRoundedRect(fill, phaseColor.withAlpha(0.45f), 2.0f);
    }

    // Phase label (left-aligned inside bar)
    rr.drawText(phaseLabel,
        {bar.x + 8.0f, bar.y + (barH - 13.0f) * 0.5f}, phaseColor);

    // Warp speed readout (right-aligned inside bar)
    if (speedAU > 0.01f) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f AU/s", speedAU);
        float tw = rr.measureText(buf);
        rr.drawText(buf,
            {bar.right() - tw - 8.0f, bar.y + (barH - 13.0f) * 0.5f},
            t.textPrimary);
    }

    // Progress percentage below bar
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f%%", progress * 100.0f);
        float tw = rr.measureText(buf);
        rr.drawText(buf,
            {pos.x - tw * 0.5f, bar.bottom() + 2.0f},
            t.textSecondary);
    }
}

// ── Mode Indicator ──────────────────────────────────────────────────

void modeIndicator(AtlasContext& ctx, Vec2 pos,
                   const char* modeText, const Color& color) {
    if (!modeText || modeText[0] == '\0') return;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float tw = rr.measureText(modeText);
    float padding = 12.0f;
    float w = tw + padding * 2.0f;
    float h = 26.0f;
    float x = pos.x - w * 0.5f;
    float y = pos.y;

    Color accentCol = (color.a > 0.01f) ? color : t.accentPrimary;

    // Dark background pill
    rr.drawRoundedRect({x, y, w, h}, t.bgPanel.withAlpha(0.85f), 4.0f);
    rr.drawRoundedRectOutline({x, y, w, h}, accentCol.withAlpha(0.6f), 4.0f);

    // Left accent bar
    rr.drawRect({x, y + 2.0f, 3.0f, h - 4.0f}, accentCol);

    // Text
    float textY = y + (h - 13.0f) * 0.5f;
    rr.drawText(modeText, {x + padding, textY}, accentCol);
}

// ── Capacitor Vertical Bar ─────────────────────────────────────────

void capacitorVerticalBar(AtlasContext& ctx, const Rect& r,
                          float fraction, int segments) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    fraction = std::max(0.0f, std::min(1.0f, fraction));
    if (segments < 1) segments = 1;

    // Background
    rr.drawRect(r, t.bgSecondary.withAlpha(0.4f));
    rr.drawRectOutline(r, t.borderSubtle);

    float gapPx = 1.0f;
    float totalGap = gapPx * (segments - 1);
    float segH = (r.h - totalGap) / segments;
    int filledCount = static_cast<int>(fraction * segments + 0.5f);

    // Draw segments bottom-to-top
    for (int i = 0; i < segments; ++i) {
        float sy = r.y + r.h - (i + 1) * (segH + gapPx) + gapPx;
        Rect seg = {r.x + 1.0f, sy, r.w - 2.0f, segH};
        bool filled = (i < filledCount);
        Color c = filled ? t.capacitor : t.bgSecondary.withAlpha(0.15f);
        rr.drawRect(seg, c);
    }

    // Percentage label centered
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(fraction * 100));
    float tw = rr.measureText(buf);
    rr.drawText(buf, {r.x + (r.w - tw) * 0.5f, r.y + r.h + 2.0f},
                t.textSecondary);
}

// ── Velocity Arc ───────────────────────────────────────────────────

void velocityArc(AtlasContext& ctx, Vec2 centre,
                 float innerR, float outerR,
                 float speedFrac, int movementMode) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    speedFrac = std::max(0.0f, std::min(1.0f, speedFrac));

    // Movement mode colors
    Color modeColor;
    switch (movementMode) {
        case 1:  modeColor = t.success;          break;  // approach = green
        case 2:  modeColor = t.armor;            break;  // orbit = gold
        case 3:  modeColor = t.accentSecondary;  break;  // keep-at-range = cyan
        case 4:  modeColor = t.accentPrimary;    break;  // warp = blue
        default: modeColor = t.accentDim;        break;  // stopped/default
    }

    // Arc sweeps the BOTTOM half: 0 to π (right-to-left through bottom)
    float startAngle = 0.0f;
    float fullEnd = static_cast<float>(M_PI);

    // Background arc (empty)
    rr.drawArc(centre, innerR, outerR, startAngle, fullEnd,
               t.bgSecondary.withAlpha(0.25f), 32);

    // Filled arc proportional to speed
    if (speedFrac > 0.001f) {
        float fillEnd = startAngle + (fullEnd - startAngle) * speedFrac;
        rr.drawArc(centre, innerR, outerR, startAngle, fillEnd,
                   modeColor, 32);
    }
}

} // namespace atlas
