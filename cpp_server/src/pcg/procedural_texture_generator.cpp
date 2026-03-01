#include "pcg/procedural_texture_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Faction base palettes ───────────────────────────────────────────

FactionColorPalette ProceduralTextureGenerator::basePalette(
        const std::string& faction) {
    if (faction == "Solari" || faction == "Amarr") {
        return { 0.85f, 0.72f, 0.35f,   // golden primary
                 0.65f, 0.55f, 0.25f,   // darker gold secondary
                 0.95f, 0.90f, 0.70f,   // pale gold accent
                 1.00f, 0.85f, 0.40f }; // warm emissive
    }
    if (faction == "Veyren" || faction == "Caldari") {
        return { 0.55f, 0.58f, 0.65f,   // steel blue-grey
                 0.35f, 0.38f, 0.45f,   // dark slate
                 0.75f, 0.80f, 0.90f,   // light blue
                 0.40f, 0.70f, 1.00f }; // blue emissive
    }
    if (faction == "Aurelian" || faction == "Gallente") {
        return { 0.35f, 0.50f, 0.38f,   // dark green-grey
                 0.25f, 0.40f, 0.30f,   // forest accent
                 0.55f, 0.70f, 0.58f,   // pale green
                 0.30f, 1.00f, 0.60f }; // green emissive
    }
    if (faction == "Keldari" || faction == "Minmatar") {
        return { 0.50f, 0.35f, 0.25f,   // rust brown
                 0.40f, 0.28f, 0.18f,   // dark brown
                 0.70f, 0.55f, 0.40f,   // tan
                 1.00f, 0.50f, 0.20f }; // orange emissive
    }
    // Default / unknown faction: neutral grey.
    return { 0.50f, 0.50f, 0.50f,
             0.35f, 0.35f, 0.35f,
             0.70f, 0.70f, 0.70f,
             0.80f, 0.80f, 0.80f };
}

// ── Per-ship palette variation ──────────────────────────────────────

FactionColorPalette ProceduralTextureGenerator::derivepalette(
        DeterministicRNG& rng, const std::string& faction) {

    FactionColorPalette pal = basePalette(faction);

    // Add subtle per-ship hue / brightness variation.
    auto vary = [&](float& r, float& g, float& b, float amount) {
        r = std::max(0.0f, std::min(1.0f, r + rng.rangeFloat(-amount, amount)));
        g = std::max(0.0f, std::min(1.0f, g + rng.rangeFloat(-amount, amount)));
        b = std::max(0.0f, std::min(1.0f, b + rng.rangeFloat(-amount, amount)));
    };

    vary(pal.primary_r,   pal.primary_g,   pal.primary_b,   0.06f);
    vary(pal.secondary_r, pal.secondary_g, pal.secondary_b, 0.04f);
    vary(pal.accent_r,    pal.accent_g,    pal.accent_b,    0.05f);
    // Emissive stays closer to base.
    vary(pal.emissive_r,  pal.emissive_g,  pal.emissive_b,  0.02f);

    return pal;
}

// ── PBR material derivation ─────────────────────────────────────────

HullMaterialParams ProceduralTextureGenerator::deriveMaterial(
        DeterministicRNG& rng, HullClass hull, const std::string& faction) {

    HullMaterialParams mat{};

    // Base metalness by faction.
    if (faction == "Solari" || faction == "Amarr") {
        mat.metalness = rng.rangeFloat(0.75f, 0.90f);  // Highly metallic gold.
        mat.roughness = rng.rangeFloat(0.20f, 0.40f);   // Polished.
    } else if (faction == "Veyren" || faction == "Caldari") {
        mat.metalness = rng.rangeFloat(0.60f, 0.80f);
        mat.roughness = rng.rangeFloat(0.35f, 0.55f);   // Industrial matte.
    } else if (faction == "Aurelian" || faction == "Gallente") {
        mat.metalness = rng.rangeFloat(0.40f, 0.60f);   // Semi-organic.
        mat.roughness = rng.rangeFloat(0.25f, 0.45f);   // Smooth.
    } else if (faction == "Keldari" || faction == "Minmatar") {
        mat.metalness = rng.rangeFloat(0.55f, 0.75f);
        mat.roughness = rng.rangeFloat(0.50f, 0.75f);   // Rough, industrial.
    } else {
        mat.metalness = rng.rangeFloat(0.50f, 0.70f);
        mat.roughness = rng.rangeFloat(0.40f, 0.60f);
    }

    // Wear increases with hull class (bigger ships accumulate more).
    float classWear = 0.0f;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       classWear = 0.05f; break;
        case HullClass::Destroyer:     classWear = 0.08f; break;
        case HullClass::Cruiser:       classWear = 0.12f; break;
        case HullClass::Battlecruiser: classWear = 0.15f; break;
        case HullClass::Battleship:    classWear = 0.20f; break;
        case HullClass::Capital:       classWear = 0.25f; break;
        default:                       classWear = 0.10f; break;
    }
    mat.wear = std::min(1.0f, classWear + rng.rangeFloat(0.0f, 0.15f));

    // Panel groove depth scales with class.
    mat.panel_depth = rng.rangeFloat(0.2f, 0.5f)
                    + (baseHullClass(hull) == HullClass::Capital ? 0.15f : 0.0f);
    mat.panel_depth = std::min(1.0f, mat.panel_depth);

    return mat;
}

// ── Hull markings ───────────────────────────────────────────────────

std::vector<HullMarking> ProceduralTextureGenerator::generateMarkings(
        DeterministicRNG& rng, HullClass hull,
        const FactionColorPalette& pal) {

    // Marking count scales with ship class.
    int baseCount = 1;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       baseCount = 1; break;
        case HullClass::Destroyer:     baseCount = 2; break;
        case HullClass::Cruiser:       baseCount = 2; break;
        case HullClass::Battlecruiser: baseCount = 3; break;
        case HullClass::Battleship:    baseCount = 4; break;
        case HullClass::Capital:       baseCount = 5; break;
        default:                       baseCount = 2; break;
    }

    std::vector<HullMarking> markings;
    markings.reserve(static_cast<size_t>(baseCount));

    for (int i = 0; i < baseCount; ++i) {
        HullMarking m{};

        // Select marking type.
        float roll = rng.nextFloat();
        if      (roll < 0.25f) m.type = MarkingType::StripeHorizontal;
        else if (roll < 0.45f) m.type = MarkingType::StripeVertical;
        else if (roll < 0.65f) m.type = MarkingType::RegistrationCode;
        else if (roll < 0.85f) m.type = MarkingType::FactionInsignia;
        else                   m.type = MarkingType::WarningHazard;

        m.position_x = rng.nextFloat();
        m.position_y = rng.rangeFloat(-0.8f, 0.8f);
        m.width      = rng.rangeFloat(0.05f, 0.25f);
        m.height     = rng.rangeFloat(0.02f, 0.15f);

        // Use the accent colour with slight variation.
        m.color_r = std::max(0.0f, std::min(1.0f,
                    pal.accent_r + rng.rangeFloat(-0.1f, 0.1f)));
        m.color_g = std::max(0.0f, std::min(1.0f,
                    pal.accent_g + rng.rangeFloat(-0.1f, 0.1f)));
        m.color_b = std::max(0.0f, std::min(1.0f,
                    pal.accent_b + rng.rangeFloat(-0.1f, 0.1f)));

        markings.push_back(m);
    }

    return markings;
}

// ── Engine glow ─────────────────────────────────────────────────────

EngineGlowParams ProceduralTextureGenerator::generateEngineGlow(
        DeterministicRNG& rng, const std::string& faction) {

    EngineGlowParams glow{};

    // Faction-specific glow colour.
    if (faction == "Solari" || faction == "Amarr") {
        glow.color_r = 1.0f;
        glow.color_g = rng.rangeFloat(0.75f, 0.90f);
        glow.color_b = rng.rangeFloat(0.20f, 0.40f);
    } else if (faction == "Veyren" || faction == "Caldari") {
        glow.color_r = rng.rangeFloat(0.20f, 0.40f);
        glow.color_g = rng.rangeFloat(0.50f, 0.70f);
        glow.color_b = 1.0f;
    } else if (faction == "Aurelian" || faction == "Gallente") {
        glow.color_r = rng.rangeFloat(0.10f, 0.30f);
        glow.color_g = 1.0f;
        glow.color_b = rng.rangeFloat(0.40f, 0.60f);
    } else if (faction == "Keldari" || faction == "Minmatar") {
        glow.color_r = 1.0f;
        glow.color_g = rng.rangeFloat(0.35f, 0.55f);
        glow.color_b = rng.rangeFloat(0.05f, 0.20f);
    } else {
        glow.color_r = 0.8f; glow.color_g = 0.8f; glow.color_b = 1.0f;
    }

    glow.intensity   = rng.rangeFloat(0.7f, 1.0f);
    glow.core_radius = rng.rangeFloat(0.2f, 0.4f);
    glow.halo_radius = rng.rangeFloat(0.5f, 0.8f);
    glow.pulse_rate  = rng.rangeFloat(0.0f, 2.0f);

    return glow;
}

// ── Window / running lights ─────────────────────────────────────────

std::vector<WindowLight> ProceduralTextureGenerator::generateWindowLights(
        DeterministicRNG& rng, HullClass hull) {

    int count = 0;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       count = rng.range(2, 4);  break;
        case HullClass::Destroyer:     count = rng.range(3, 6);  break;
        case HullClass::Cruiser:       count = rng.range(5, 10); break;
        case HullClass::Battlecruiser: count = rng.range(8, 14); break;
        case HullClass::Battleship:    count = rng.range(12, 20); break;
        case HullClass::Capital:       count = rng.range(20, 40); break;
        default:                       count = rng.range(4, 8);  break;
    }

    std::vector<WindowLight> lights;
    lights.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        WindowLight w{};
        w.position_x = rng.nextFloat();
        w.position_y = rng.rangeFloat(-0.9f, 0.9f);
        w.size       = rng.rangeFloat(0.005f, 0.02f);

        // Warm white / pale yellow.
        w.color_r = rng.rangeFloat(0.85f, 1.0f);
        w.color_g = rng.rangeFloat(0.80f, 0.95f);
        w.color_b = rng.rangeFloat(0.60f, 0.85f);

        // Zone from spine position.
        if      (w.position_x < 0.30f) w.zone_index = 0;
        else if (w.position_x < 0.75f) w.zone_index = 1;
        else                           w.zone_index = 2;

        lights.push_back(w);
    }

    return lights;
}

// ── UV panel tiling ─────────────────────────────────────────────────

int ProceduralTextureGenerator::computePanelTiling(HullClass hull) {
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       return 4;
        case HullClass::Destroyer:     return 6;
        case HullClass::Cruiser:       return 8;
        case HullClass::Battlecruiser: return 10;
        case HullClass::Battleship:    return 14;
        case HullClass::Capital:       return 20;
        default:                       return 8;
    }
}

// ── Marking type name ───────────────────────────────────────────────

std::string ProceduralTextureGenerator::markingTypeName(MarkingType type) {
    switch (type) {
        case MarkingType::StripeHorizontal: return "StripeHorizontal";
        case MarkingType::StripeVertical:   return "StripeVertical";
        case MarkingType::RegistrationCode: return "RegistrationCode";
        case MarkingType::FactionInsignia:  return "FactionInsignia";
        case MarkingType::WarningHazard:    return "WarningHazard";
    }
    return "Unknown";
}

// ── Public API ──────────────────────────────────────────────────────

GeneratedTextureParams ProceduralTextureGenerator::generate(
        const PCGContext& ctx, HullClass hull,
        const std::string& faction) {

    DeterministicRNG rng(ctx.seed);

    GeneratedTextureParams result{};
    result.ship_id    = ctx.seed;
    result.faction    = faction;
    result.hull_class = hull;

    // 1. Palette.
    result.palette = derivepalette(rng, faction);

    // 2. Material.
    result.material = deriveMaterial(rng, hull, faction);

    // 3. Markings.
    result.markings = generateMarkings(rng, hull, result.palette);

    // 4. Engine glow.
    result.engine_glow = generateEngineGlow(rng, faction);

    // 5. Window lights.
    result.window_lights = generateWindowLights(rng, hull);

    // 6. Panel tiling.
    result.panel_tile_count = computePanelTiling(hull);

    // 7. Validate.
    result.valid = !result.markings.empty()
                && !result.window_lights.empty()
                && result.material.metalness >= 0.0f
                && result.material.metalness <= 1.0f;

    return result;
}

} // namespace pcg
} // namespace atlas
