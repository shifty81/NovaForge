#include "pcg/turret_placement_system.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Helpers ─────────────────────────────────────────────────────────

static float normalizeDeg(float deg) {
    deg = std::fmod(deg, 360.0f);
    if (deg < 0.0f) deg += 360.0f;
    return deg;
}

static float pairwiseOverlap(const TurretMount& a, const TurretMount& b) {
    float halfA = a.arc_deg * 0.5f;
    float halfB = b.arc_deg * 0.5f;
    float diff  = normalizeDeg(a.direction_deg - b.direction_deg + 180.0f)
                  - 180.0f;
    if (diff < -180.0f) diff += 360.0f;
    float dist       = std::fabs(diff);
    float overlapDeg = (halfA + halfB) - dist;
    if (overlapDeg <= 0.0f) return 0.0f;
    float minArc = std::min(a.arc_deg, b.arc_deg);
    if (minArc <= 0.0f) return 0.0f;
    return std::min(overlapDeg / minArc, 1.0f);
}

// ── Size for hull ───────────────────────────────────────────────────

TurretSize TurretPlacementSystem::sizeForHull(DeterministicRNG& rng,
                                               HullClass hull) {
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:
        case HullClass::Destroyer:
            return TurretSize::Small;
        case HullClass::Cruiser:
        case HullClass::Battlecruiser:
            return rng.chance(0.7f) ? TurretSize::Medium : TurretSize::Small;
        case HullClass::Battleship:
            return rng.chance(0.6f) ? TurretSize::Large : TurretSize::Medium;
        case HullClass::Capital:
            return TurretSize::Capital;
        default: // Safety fallback for invalid enum values.
            return TurretSize::Small;
    }
}

// ── Mount distribution ──────────────────────────────────────────────

void TurretPlacementSystem::distributeMounts(DeterministicRNG& rng,
                                              TurretPlacement& placement,
                                              HullClass hull,
                                              int count,
                                              TurretSize size) {
    if (count <= 0) return;

    // Role-dependent base arc and spread.
    float baseArc = 0.0f;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:
        case HullClass::Destroyer:
            baseArc = rng.rangeFloat(100.0f, 180.0f);
            break;
        case HullClass::Cruiser:
        case HullClass::Battlecruiser:
            baseArc = rng.rangeFloat(60.0f, 110.0f);
            break;
        case HullClass::Battleship:
            baseArc = rng.rangeFloat(50.0f, 100.0f);
            break;
        case HullClass::Capital:
            baseArc = rng.rangeFloat(30.0f, 70.0f);
            break;
        default: // Safety fallback for invalid enum values.
            baseArc = rng.rangeFloat(60.0f, 110.0f);
            break;
    }

    // Distribute turrets along the hull spine and around the centreline.
    float spacing = (count > 1) ? 180.0f / static_cast<float>(count - 1) : 0.0f;
    float startAngle = (count > 1) ? -90.0f : 0.0f;

    for (int i = 0; i < count; ++i) {
        TurretMount m{};
        m.socket_id     = static_cast<uint32_t>(i);
        m.size          = size;
        m.direction_deg = startAngle + spacing * static_cast<float>(i);
        m.arc_deg       = baseArc + rng.rangeFloat(-10.0f, 10.0f);

        // Position along spine: turrets spread from 20% to 80% of hull.
        float t = (count > 1)
                  ? 0.2f + 0.6f * static_cast<float>(i) / static_cast<float>(count - 1)
                  : 0.5f;
        m.x_offset = (t - 0.5f) * 100.0f;  // Normalised range.

        // Alternate dorsal / ventral placement.
        m.y_offset = (i % 2 == 0) ? rng.rangeFloat(0.1f, 0.4f)
                                    : rng.rangeFloat(-0.4f, -0.1f);
        m.z_offset = rng.rangeFloat(0.1f, 0.5f);

        placement.mounts.push_back(m);
    }
}

// ── Faction rules ───────────────────────────────────────────────────

void TurretPlacementSystem::applyFactionRules(DeterministicRNG& rng,
                                               TurretPlacement& placement,
                                               const std::string& faction) {
    if (faction == "Solari") {
        // Forward-biased placement, elegant narrow arcs.
        for (auto& m : placement.mounts) {
            m.direction_deg *= 0.8f;  // Compress toward forward.
            m.arc_deg       *= 0.9f;
        }
    } else if (faction == "Veyren") {
        // Wider arcs, utilitarian spread.
        for (auto& m : placement.mounts) {
            m.arc_deg += rng.rangeFloat(5.0f, 15.0f);
        }
    } else if (faction == "Aurelian") {
        // Balanced, slight dorsal bias.
        for (auto& m : placement.mounts) {
            m.z_offset += 0.1f;
        }
    } else if (faction == "Keldari") {
        // Wider lateral spread for broadside.
        for (auto& m : placement.mounts) {
            m.y_offset *= 1.2f;
            m.arc_deg  += rng.rangeFloat(0.0f, 10.0f);
        }
    }
}

// ── Coverage computation ────────────────────────────────────────────

float TurretPlacementSystem::computeCoverage(
    const std::vector<TurretMount>& mounts) {
    if (mounts.empty()) return 0.0f;

    // Sample 360 degrees in 1-degree steps.
    int covered = 0;
    for (int deg = 0; deg < 360; ++deg) {
        float bearing = static_cast<float>(deg);
        for (const auto& m : mounts) {
            float halfArc = m.arc_deg * 0.5f;
            float diff    = normalizeDeg(bearing - m.direction_deg + 180.0f) - 180.0f;
            if (diff < -180.0f) diff += 360.0f;
            if (std::fabs(diff) <= halfArc) {
                covered++;
                break;
            }
        }
    }
    return static_cast<float>(covered) / 360.0f;
}

float TurretPlacementSystem::computeMaxOverlap(
    const std::vector<TurretMount>& mounts) {
    float maxOv = 0.0f;
    for (size_t a = 0; a < mounts.size(); ++a) {
        for (size_t b = a + 1; b < mounts.size(); ++b) {
            float ov = pairwiseOverlap(mounts[a], mounts[b]);
            if (ov > maxOv) maxOv = ov;
        }
    }
    return maxOv;
}

// ── Public API ──────────────────────────────────────────────────────

TurretPlacement TurretPlacementSystem::place(const PCGContext& ctx,
                                              HullClass hull,
                                              int turretSlots,
                                              const std::string& faction) {
    DeterministicRNG rng(ctx.seed);

    TurretPlacement result{};
    result.ship_id = ctx.seed;

    // 1. Determine turret size for this hull.
    TurretSize size = sizeForHull(rng, hull);

    // 2. Distribute mounts.
    distributeMounts(rng, result, hull, turretSlots, size);

    // 3. Apply faction rules.
    if (!faction.empty()) {
        applyFactionRules(rng, result, faction);
    }

    // 4. Compute metrics.
    result.coverage_score = computeCoverage(result.mounts);
    result.max_overlap    = computeMaxOverlap(result.mounts);

    // 5. Validate: overlap must be < 30%.
    result.valid = (result.max_overlap < 0.30f) || (turretSlots <= 1);

    return result;
}

} // namespace pcg
} // namespace atlas
