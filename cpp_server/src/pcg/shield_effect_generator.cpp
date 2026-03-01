#include "pcg/shield_effect_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Pattern selection ───────────────────────────────────────────────

ShieldPattern ShieldEffectGenerator::selectPattern(
        DeterministicRNG& rng, const std::string& faction) {

    if (faction == "Veyren" || faction == "Caldari") {
        // Tech / angular factions prefer hexagonal or lattice.
        return rng.chance(0.65f) ? ShieldPattern::Hexagonal
                                : ShieldPattern::Lattice;
    }
    if (faction == "Aurelian" || faction == "Gallente") {
        // Organic factions prefer smooth or ripple.
        return rng.chance(0.60f) ? ShieldPattern::Smooth
                                : ShieldPattern::Ripple;
    }
    if (faction == "Solari" || faction == "Amarr") {
        // Ornate factions prefer ornate or smooth.
        return rng.chance(0.70f) ? ShieldPattern::Ornate
                                : ShieldPattern::Smooth;
    }
    if (faction == "Keldari" || faction == "Minmatar") {
        // Industrial factions prefer lattice or hexagonal.
        return rng.chance(0.55f) ? ShieldPattern::Lattice
                                : ShieldPattern::Hexagonal;
    }

    // Default: random.
    float roll = rng.nextFloat();
    if      (roll < 0.25f) return ShieldPattern::Hexagonal;
    else if (roll < 0.50f) return ShieldPattern::Smooth;
    else if (roll < 0.70f) return ShieldPattern::Lattice;
    else if (roll < 0.85f) return ShieldPattern::Ornate;
    else                   return ShieldPattern::Ripple;
}

// ── Base shield colour ──────────────────────────────────────────────

void ShieldEffectGenerator::deriveBaseColor(
        DeterministicRNG& rng, const std::string& faction,
        float& r, float& g, float& b) {

    if (faction == "Solari" || faction == "Amarr") {
        r = rng.rangeFloat(0.85f, 1.00f);
        g = rng.rangeFloat(0.75f, 0.90f);
        b = rng.rangeFloat(0.30f, 0.50f);
    } else if (faction == "Veyren" || faction == "Caldari") {
        r = rng.rangeFloat(0.30f, 0.50f);
        g = rng.rangeFloat(0.55f, 0.75f);
        b = rng.rangeFloat(0.85f, 1.00f);
    } else if (faction == "Aurelian" || faction == "Gallente") {
        r = rng.rangeFloat(0.20f, 0.40f);
        g = rng.rangeFloat(0.80f, 1.00f);
        b = rng.rangeFloat(0.40f, 0.65f);
    } else if (faction == "Keldari" || faction == "Minmatar") {
        r = rng.rangeFloat(0.80f, 1.00f);
        g = rng.rangeFloat(0.45f, 0.65f);
        b = rng.rangeFloat(0.15f, 0.35f);
    } else {
        r = rng.rangeFloat(0.50f, 0.70f);
        g = rng.rangeFloat(0.60f, 0.80f);
        b = rng.rangeFloat(0.80f, 1.00f);
    }
}

// ── Hit flash colour ────────────────────────────────────────────────

void ShieldEffectGenerator::deriveHitFlashColor(
        DeterministicRNG& rng, const std::string& faction,
        float& r, float& g, float& b) {
    // Hit flash is brighter / whiter version of faction colour.
    deriveBaseColor(rng, faction, r, g, b);
    r = std::min(1.0f, r + 0.3f);
    g = std::min(1.0f, g + 0.3f);
    b = std::min(1.0f, b + 0.3f);
}

// ── Pattern scale from class ────────────────────────────────────────

float ShieldEffectGenerator::computePatternScale(HullClass hull) {
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       return 1.0f;
        case HullClass::Destroyer:     return 1.2f;
        case HullClass::Cruiser:       return 1.8f;
        case HullClass::Battlecruiser: return 2.2f;
        case HullClass::Battleship:    return 3.0f;
        case HullClass::Capital:       return 5.0f;
        default:                       return 1.5f;
    }
}

// ── Shield radius multiplier ────────────────────────────────────────

float ShieldEffectGenerator::computeShieldRadius(HullClass hull) {
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       return 1.1f;
        case HullClass::Destroyer:     return 1.15f;
        case HullClass::Cruiser:       return 1.2f;
        case HullClass::Battlecruiser: return 1.2f;
        case HullClass::Battleship:    return 1.25f;
        case HullClass::Capital:       return 1.3f;
        default:                       return 1.15f;
    }
}

// ── Sample impact ripples ───────────────────────────────────────────

std::vector<ShieldImpactRipple> ShieldEffectGenerator::generateSampleImpacts(
        DeterministicRNG& rng, int count) {

    std::vector<ShieldImpactRipple> impacts;
    impacts.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        ShieldImpactRipple imp{};
        imp.origin_x   = rng.nextFloat();
        imp.origin_y   = rng.rangeFloat(-1.0f, 1.0f);
        imp.intensity   = rng.rangeFloat(0.5f, 1.0f);
        imp.radius      = rng.rangeFloat(0.1f, 0.4f);
        imp.decay_rate  = rng.rangeFloat(1.0f, 4.0f);
        imp.speed       = rng.rangeFloat(0.3f, 1.0f);
        impacts.push_back(imp);
    }

    return impacts;
}

// ── Pattern name ────────────────────────────────────────────────────

std::string ShieldEffectGenerator::patternName(ShieldPattern pattern) {
    switch (pattern) {
        case ShieldPattern::Hexagonal: return "Hexagonal";
        case ShieldPattern::Smooth:    return "Smooth";
        case ShieldPattern::Lattice:   return "Lattice";
        case ShieldPattern::Ornate:    return "Ornate";
        case ShieldPattern::Ripple:    return "Ripple";
    }
    return "Unknown";
}

// ── Public API ──────────────────────────────────────────────────────

GeneratedShieldEffect ShieldEffectGenerator::generate(
        const PCGContext& ctx, HullClass hull,
        const std::string& faction) {

    DeterministicRNG rng(ctx.seed);

    GeneratedShieldEffect effect{};
    effect.ship_id = ctx.seed;

    // 1. Pattern.
    effect.pattern = selectPattern(rng, faction);

    // 2. Base colour.
    deriveBaseColor(rng, faction,
                    effect.base_color_r,
                    effect.base_color_g,
                    effect.base_color_b);

    // 3. Hit flash colour.
    deriveHitFlashColor(rng, faction,
                        effect.hit_flash_color_r,
                        effect.hit_flash_color_g,
                        effect.hit_flash_color_b);

    // 4. Idle opacity.
    effect.base_opacity = rng.rangeFloat(0.03f, 0.12f);

    // 5. Shimmer (idle animation).
    effect.shimmer_speed     = rng.rangeFloat(0.2f, 1.5f);
    effect.shimmer_amplitude = rng.rangeFloat(0.02f, 0.08f);

    // 6. Pattern scale.
    effect.pattern_scale = computePatternScale(hull);

    // 7. Fresnel.
    effect.fresnel_power = rng.rangeFloat(2.0f, 5.0f);

    // 8. Shield radius.
    effect.shield_radius = computeShieldRadius(hull);

    // 9. Sample impact ripples (3 per ship for testing / preview).
    effect.sample_impacts = generateSampleImpacts(rng, 3);

    // 10. Validate.
    effect.valid = (effect.base_opacity > 0.0f)
                && (effect.pattern_scale > 0.0f)
                && (effect.shield_radius >= 1.0f)
                && (effect.fresnel_power >= 1.0f);

    return effect;
}

} // namespace pcg
} // namespace atlas
