#include "pcg/weapon_socket.h"
#include "pcg/hash_utils.h"

#include <algorithm>

namespace atlas {
namespace pcg {

// ── Static helpers ──────────────────────────────────────────────────

float WeaponSocketUtil::normalizeDeg(float deg) {
    deg = std::fmod(deg, 360.0f);
    if (deg < 0.0f) deg += 360.0f;
    return deg;
}

bool WeaponSocketUtil::canHitTarget(float angularVelocity, float tracking) {
    return angularVelocity <= tracking;
}

bool WeaponSocketUtil::isInArc(float bearingDeg, const WeaponSocket& socket) {
    float halfArc = socket.arcDeg * 0.5f;
    float diff    = normalizeDeg(bearingDeg - socket.directionDeg + 180.0f) - 180.0f;
    if (diff < -180.0f) diff += 360.0f;
    return std::fabs(diff) <= halfArc;
}

float WeaponSocketUtil::arcOverlap(const WeaponSocket& a,
                                    const WeaponSocket& b) {
    float halfA = a.arcDeg * 0.5f;
    float halfB = b.arcDeg * 0.5f;

    float diff = normalizeDeg(a.directionDeg - b.directionDeg + 180.0f)
                 - 180.0f;
    if (diff < -180.0f) diff += 360.0f;

    float dist = std::fabs(diff);
    float overlapDeg = (halfA + halfB) - dist;
    if (overlapDeg <= 0.0f) return 0.0f;

    float minArc = std::min(a.arcDeg, b.arcDeg);
    if (minArc <= 0.0f) return 0.0f;

    return std::min(overlapDeg / minArc, 1.0f);
}

// ── Layout generation ───────────────────────────────────────────────

WeaponLayout WeaponSocketUtil::generateLayout(const PCGContext& ctx,
                                               const GeneratedShip& ship) {
    DeterministicRNG rng(ctx.seed);
    WeaponLayout layout{};
    layout.shipId = ship.shipId;

    int totalTurrets = ship.turretSlots;

    // Determine arc style from hull class (EVE-style roles).
    float baseArc      = 0.0f;
    float baseTracking = 0.0f;

    switch (ship.hullClass) {
        case HullClass::Frigate:
        case HullClass::Destroyer:
            // Escort / tackle — wide arcs, fast tracking.
            baseArc      = rng.rangeFloat(90.0f, 180.0f);
            baseTracking = rng.rangeFloat(0.08f, 0.15f);
            break;
        case HullClass::Cruiser:
        case HullClass::Battlecruiser:
            // Line ships — moderate arcs.
            baseArc      = rng.rangeFloat(60.0f, 120.0f);
            baseTracking = rng.rangeFloat(0.04f, 0.08f);
            break;
        case HullClass::Battleship:
            // Brawlers — forward batteries, slow tracking.
            baseArc      = rng.rangeFloat(40.0f, 90.0f);
            baseTracking = rng.rangeFloat(0.02f, 0.05f);
            break;
        case HullClass::Capital:
            // Capitals — very limited traverse.
            baseArc      = rng.rangeFloat(20.0f, 60.0f);
            baseTracking = rng.rangeFloat(0.005f, 0.02f);
            break;
    }

    // Distribute turrets evenly across the forward hemisphere (180°).
    static constexpr float FORWARD_HEMISPHERE_DEG = 180.0f;
    float spacing = (totalTurrets > 1)
                    ? FORWARD_HEMISPHERE_DEG / static_cast<float>(totalTurrets - 1)
                    : 0.0f;
    float startAngle = -(FORWARD_HEMISPHERE_DEG * 0.5f) + (spacing * 0.5f);
    if (totalTurrets == 1) startAngle = 0.0f;

    float totalDPS   = 0.0f;
    float forwardDPS = 0.0f;

    for (int i = 0; i < totalTurrets; ++i) {
        WeaponSocket s{};
        s.id          = static_cast<uint32_t>(i);
        s.size        = ship.maxWeaponSize;
        s.directionDeg = startAngle + spacing * static_cast<float>(i);
        s.arcDeg      = baseArc + rng.rangeFloat(-10.0f, 10.0f);
        s.tracking    = baseTracking * rng.rangeFloat(0.9f, 1.1f);
        s.occupied    = true;

        // Estimate DPS contribution (size-dependent base).
        float dps = 0.0f;
        switch (s.size) {
            case WeaponSize::Small:  dps = rng.rangeFloat(30.0f, 60.0f);   break;
            case WeaponSize::Medium: dps = rng.rangeFloat(100.0f, 200.0f); break;
            case WeaponSize::Large:  dps = rng.rangeFloat(300.0f, 600.0f); break;
        }
        totalDPS += dps;
        if (isInArc(0.0f, s)) forwardDPS += dps;

        layout.sockets.push_back(s);
    }

    layout.totalDPS   = totalDPS;
    layout.forwardDPS = forwardDPS;

    // Compute average pairwise turret overlap.
    float overlapSum = 0.0f;
    int   pairs      = 0;
    for (size_t a = 0; a < layout.sockets.size(); ++a) {
        for (size_t b = a + 1; b < layout.sockets.size(); ++b) {
            overlapSum += arcOverlap(layout.sockets[a], layout.sockets[b]);
            ++pairs;
        }
    }
    layout.turretOverlap = (pairs > 0) ? overlapSum / static_cast<float>(pairs)
                                        : 0.0f;

    return layout;
}

} // namespace pcg
} // namespace atlas
