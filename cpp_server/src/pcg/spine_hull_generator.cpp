#include "pcg/spine_hull_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Spine selection ─────────────────────────────────────────────────

SpineType SpineHullGenerator::selectSpine(DeterministicRNG& rng,
                                           HullClass hull) {
    // Hull class biases the spine archetype.
    switch (hull) {
        case HullClass::Frigate:
        case HullClass::Destroyer:
            // Fast, nimble → favour Needle / Wedge.
            return rng.chance(0.5f) ? SpineType::Needle : SpineType::Wedge;
        case HullClass::Cruiser:
        case HullClass::Battlecruiser: {
            float r = rng.nextFloat();
            if (r < 0.3f) return SpineType::Wedge;
            if (r < 0.6f) return SpineType::Hammer;
            return SpineType::Slab;
        }
        case HullClass::Battleship:
            return rng.chance(0.6f) ? SpineType::Hammer : SpineType::Slab;
        case HullClass::Capital:
            return rng.chance(0.4f) ? SpineType::Slab : SpineType::Ring;
    }
    return SpineType::Wedge;
}

// ── Width profile derivation ────────────────────────────────────────

SpineProfile SpineHullGenerator::deriveProfile(DeterministicRNG& rng,
                                                SpineType spine,
                                                HullClass hull) {
    SpineProfile p{};

    // Base length by hull class.
    switch (hull) {
        case HullClass::Frigate:       p.length = rng.rangeFloat(30.0f, 60.0f);      break;
        case HullClass::Destroyer:     p.length = rng.rangeFloat(60.0f, 100.0f);     break;
        case HullClass::Cruiser:       p.length = rng.rangeFloat(100.0f, 200.0f);    break;
        case HullClass::Battlecruiser: p.length = rng.rangeFloat(200.0f, 350.0f);    break;
        case HullClass::Battleship:    p.length = rng.rangeFloat(350.0f, 600.0f);    break;
        case HullClass::Capital:       p.length = rng.rangeFloat(800.0f, 2000.0f);   break;
    }

    // Width ratios driven by spine archetype.
    float base_w = p.length * rng.rangeFloat(0.08f, 0.20f);

    switch (spine) {
        case SpineType::Needle:
            p.width_fwd = base_w * rng.rangeFloat(0.3f, 0.5f);
            p.width_mid = base_w * rng.rangeFloat(0.5f, 0.7f);
            p.width_aft = base_w * rng.rangeFloat(0.4f, 0.6f);
            break;
        case SpineType::Wedge:
            p.width_fwd = base_w * rng.rangeFloat(0.8f, 1.2f);
            p.width_mid = base_w * rng.rangeFloat(0.5f, 0.8f);
            p.width_aft = base_w * rng.rangeFloat(0.3f, 0.5f);
            break;
        case SpineType::Hammer:
            p.width_fwd = base_w * rng.rangeFloat(1.0f, 1.5f);
            p.width_mid = base_w * rng.rangeFloat(0.5f, 0.7f);
            p.width_aft = base_w * rng.rangeFloat(0.4f, 0.6f);
            break;
        case SpineType::Slab:
            p.width_fwd = base_w * rng.rangeFloat(0.8f, 1.0f);
            p.width_mid = base_w * rng.rangeFloat(0.9f, 1.1f);
            p.width_aft = base_w * rng.rangeFloat(0.8f, 1.0f);
            break;
        case SpineType::Ring:
            p.width_fwd = base_w * rng.rangeFloat(0.9f, 1.1f);
            p.width_mid = base_w * rng.rangeFloat(1.2f, 1.6f);
            p.width_aft = base_w * rng.rangeFloat(0.9f, 1.1f);
            break;
    }

    return p;
}

// ── Functional zone layout ──────────────────────────────────────────

std::vector<HullZone> SpineHullGenerator::layoutZones(DeterministicRNG& rng,
                                                       HullClass hull) {
    std::vector<HullZone> zones(3);

    // Zones are always Command → MidHull → Engineering.
    zones[0].zone = FunctionalZone::Command;
    zones[1].zone = FunctionalZone::MidHull;
    zones[2].zone = FunctionalZone::Engineering;

    // Fraction of spine length per zone varies by hull class.
    float cmd_frac = 0.0f, mid_frac = 0.0f, eng_frac = 0.0f;

    switch (hull) {
        case HullClass::Frigate:
        case HullClass::Destroyer:
            cmd_frac = rng.rangeFloat(0.20f, 0.30f);
            eng_frac = rng.rangeFloat(0.25f, 0.35f);
            break;
        case HullClass::Cruiser:
        case HullClass::Battlecruiser:
            cmd_frac = rng.rangeFloat(0.15f, 0.25f);
            eng_frac = rng.rangeFloat(0.20f, 0.30f);
            break;
        case HullClass::Battleship:
            cmd_frac = rng.rangeFloat(0.10f, 0.20f);
            eng_frac = rng.rangeFloat(0.20f, 0.30f);
            break;
        case HullClass::Capital:
            cmd_frac = rng.rangeFloat(0.08f, 0.15f);
            eng_frac = rng.rangeFloat(0.15f, 0.25f);
            break;
    }
    mid_frac = 1.0f - cmd_frac - eng_frac;

    zones[0].length_fraction = cmd_frac;
    zones[1].length_fraction = mid_frac;
    zones[2].length_fraction = eng_frac;

    // Greeble counts scale with hull class (cosmetic only).
    int base_greeble = 0;
    switch (hull) {
        case HullClass::Frigate:       base_greeble = 2; break;
        case HullClass::Destroyer:     base_greeble = 3; break;
        case HullClass::Cruiser:       base_greeble = 5; break;
        case HullClass::Battlecruiser: base_greeble = 7; break;
        case HullClass::Battleship:    base_greeble = 10; break;
        case HullClass::Capital:       base_greeble = 15; break;
    }

    zones[0].greeble_count = rng.range(1, base_greeble);
    zones[1].greeble_count = rng.range(base_greeble / 2, base_greeble * 2);
    zones[2].greeble_count = rng.range(1, base_greeble);

    return zones;
}

// ── Engine cluster ──────────────────────────────────────────────────

int SpineHullGenerator::generateEngineCluster(DeterministicRNG& rng,
                                               HullClass hull) {
    switch (hull) {
        case HullClass::Frigate:       return rng.range(1, 2);
        case HullClass::Destroyer:     return rng.range(2, 3);
        case HullClass::Cruiser:       return rng.range(2, 4);
        case HullClass::Battlecruiser: return rng.range(3, 5);
        case HullClass::Battleship:    return rng.range(4, 6);
        case HullClass::Capital:       return rng.range(6, 12);
    }
    return 2;
}

// ── Faction shape language ──────────────────────────────────────────

void SpineHullGenerator::applyFactionStyle(DeterministicRNG& rng,
                                            GeneratedSpineHull& hull,
                                            const std::string& faction) {
    hull.faction_style = faction;

    if (faction == "Solari") {
        // Golden, elegant spires — elongate and narrow.
        hull.profile.width_mid *= 0.85f;
        hull.profile.length   *= 1.1f;
        // Extra greeble for ornate spires.
        for (auto& z : hull.zones) z.greeble_count += rng.range(1, 3);
    } else if (faction == "Veyren") {
        // Angular, utilitarian — widen, fewer cosmetic details.
        hull.profile.width_mid *= 1.1f;
        hull.profile.width_fwd *= 1.05f;
        for (auto& z : hull.zones) z.greeble_count = std::max(1, z.greeble_count - 1);
    } else if (faction == "Aurelian") {
        // Sleek, organic — smooth curves, moderate greeble.
        hull.profile.width_fwd *= 0.9f;
        hull.profile.width_aft *= 0.95f;
    } else if (faction == "Keldari") {
        // Rugged, industrial — wider, extra greeble (visible internals).
        hull.profile.width_mid *= 1.15f;
        hull.profile.width_aft *= 1.1f;
        for (auto& z : hull.zones) z.greeble_count += rng.range(2, 4);
    }
}

// ── Aspect ratio clamping ───────────────────────────────────────────

void SpineHullGenerator::clampAspectRatio(GeneratedSpineHull& hull) {
    float max_width = std::max({hull.profile.width_fwd,
                                hull.profile.width_mid,
                                hull.profile.width_aft});
    if (max_width <= 0.0f) max_width = 1.0f;
    hull.aspect_ratio = hull.profile.length / max_width;

    // Clamp to prevent degenerate shapes (max elongation 1.5× hull class norm).
    static constexpr float MAX_ASPECT = 15.0f;
    static constexpr float MIN_ASPECT = 1.5f;
    if (hull.aspect_ratio > MAX_ASPECT) {
        hull.profile.length = max_width * MAX_ASPECT;
        hull.aspect_ratio   = MAX_ASPECT;
    }
    if (hull.aspect_ratio < MIN_ASPECT) {
        hull.profile.length = max_width * MIN_ASPECT;
        hull.aspect_ratio   = MIN_ASPECT;
    }
}

// ── Validation ──────────────────────────────────────────────────────

bool SpineHullGenerator::validate(const GeneratedSpineHull& hull) {
    if (hull.profile.length <= 0.0f) return false;
    if (hull.profile.width_mid <= 0.0f) return false;
    if (hull.engine_cluster_count <= 0) return false;
    if (hull.zones.size() != 3) return false;

    // Zone fractions must sum to ~1.0.
    float total = 0.0f;
    for (const auto& z : hull.zones) total += z.length_fraction;
    if (std::fabs(total - 1.0f) > 0.01f) return false;

    // Aspect ratio within bounds.
    if (hull.aspect_ratio < 1.5f || hull.aspect_ratio > 15.0f) return false;

    return true;
}

// ── Public API ──────────────────────────────────────────────────────

std::string SpineHullGenerator::spineTypeName(SpineType spine) {
    switch (spine) {
        case SpineType::Needle: return "Needle";
        case SpineType::Wedge:  return "Wedge";
        case SpineType::Hammer: return "Hammer";
        case SpineType::Slab:   return "Slab";
        case SpineType::Ring:   return "Ring";
    }
    return "Unknown";
}

GeneratedSpineHull SpineHullGenerator::generate(const PCGContext& ctx) {
    DeterministicRNG rng(ctx.seed);
    // Auto-select hull class using same probabilities as ShipGenerator.
    float r = rng.nextFloat();
    HullClass hull;
    if      (r < 0.30f) hull = HullClass::Frigate;
    else if (r < 0.50f) hull = HullClass::Destroyer;
    else if (r < 0.70f) hull = HullClass::Cruiser;
    else if (r < 0.85f) hull = HullClass::Battlecruiser;
    else if (r < 0.97f) hull = HullClass::Battleship;
    else                 hull = HullClass::Capital;

    return generate(ctx, hull);
}

GeneratedSpineHull SpineHullGenerator::generate(const PCGContext& ctx,
                                                 HullClass hull) {
    return generate(ctx, hull, "");
}

GeneratedSpineHull SpineHullGenerator::generate(const PCGContext& ctx,
                                                 HullClass hull,
                                                 const std::string& faction) {
    DeterministicRNG rng(ctx.seed);

    GeneratedSpineHull result{};
    result.hull_id    = ctx.seed;
    result.hull_class = hull;

    // 1. Select spine archetype.
    result.spine = selectSpine(rng, hull);

    // 2. Derive width profile.
    result.profile = deriveProfile(rng, result.spine, hull);

    // 3. Layout functional zones.
    result.zones = layoutZones(rng, hull);

    // 4. Engine cluster.
    result.engine_cluster_count = generateEngineCluster(rng, hull);

    // 5. Apply faction style (modifies profile + greeble).
    if (!faction.empty()) {
        applyFactionStyle(rng, result, faction);
    }

    // 6. Bilateral symmetry (always enforced for ships).
    result.bilateral_symmetry = true;

    // 7. Aspect ratio clamping.
    clampAspectRatio(result);

    // 8. Total greeble count.
    result.total_greeble_count = 0;
    for (const auto& z : result.zones) {
        result.total_greeble_count += z.greeble_count;
    }

    // 9. Validate.
    result.valid = validate(result);

    return result;
}

} // namespace pcg
} // namespace atlas
