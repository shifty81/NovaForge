#include "pcg/damage_state_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Damage level classification ─────────────────────────────────────

DamageLevel DamageStateGenerator::classifyDamage(float damageNorm) {
    if (damageNorm <= 0.0f)  return DamageLevel::Pristine;
    if (damageNorm <= 0.20f) return DamageLevel::Light;
    if (damageNorm <= 0.50f) return DamageLevel::Moderate;
    if (damageNorm <= 0.80f) return DamageLevel::Heavy;
    return DamageLevel::Critical;
}

// ── Hull breach count ───────────────────────────────────────────────

int DamageStateGenerator::computeBreachCount(DeterministicRNG& rng,
                                              DamageLevel level,
                                              HullClass hull) {
    // Bigger ships can sustain more breaches before being critical.
    int hullScale = 1;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       hullScale = 1; break;
        case HullClass::Destroyer:     hullScale = 1; break;
        case HullClass::Cruiser:       hullScale = 2; break;
        case HullClass::Battlecruiser: hullScale = 2; break;
        case HullClass::Battleship:    hullScale = 3; break;
        case HullClass::Capital:       hullScale = 4; break;
        default:                       hullScale = 2; break;
    }

    switch (level) {
        case DamageLevel::Pristine: return 0;
        case DamageLevel::Light:    return 0;
        case DamageLevel::Moderate: return rng.range(1, 1 * hullScale);
        case DamageLevel::Heavy:    return rng.range(1, 3 * hullScale);
        case DamageLevel::Critical: return rng.range(2, 5 * hullScale);
    }
    return 0;
}

// ── Missing module count ────────────────────────────────────────────

int DamageStateGenerator::computeMissingModules(DeterministicRNG& rng,
                                                 DamageLevel level,
                                                 HullClass hull) {
    int maxModules = 2;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       maxModules = 2; break;
        case HullClass::Destroyer:     maxModules = 3; break;
        case HullClass::Cruiser:       maxModules = 4; break;
        case HullClass::Battlecruiser: maxModules = 5; break;
        case HullClass::Battleship:    maxModules = 6; break;
        case HullClass::Capital:       maxModules = 8; break;
        default:                       maxModules = 4; break;
    }

    switch (level) {
        case DamageLevel::Pristine: return 0;
        case DamageLevel::Light:    return 0;
        case DamageLevel::Moderate: return rng.range(0, 1);
        case DamageLevel::Heavy:    return rng.range(1, maxModules / 2);
        case DamageLevel::Critical: return rng.range(maxModules / 2, maxModules);
    }
    return 0;
}

// ── Decal generation ────────────────────────────────────────────────

std::vector<DamageDecal> DamageStateGenerator::generateDecals(
        DeterministicRNG& rng, DamageLevel level, HullClass hull) {

    if (level == DamageLevel::Pristine) return {};

    // Decal count driven by damage level and ship class.
    int baseCount = 0;
    switch (level) {
        case DamageLevel::Pristine: baseCount = 0; break;
        case DamageLevel::Light:    baseCount = 2; break;
        case DamageLevel::Moderate: baseCount = 5; break;
        case DamageLevel::Heavy:    baseCount = 10; break;
        case DamageLevel::Critical: baseCount = 16; break;
    }

    // Bigger ships have more surface area → more decals.
    int hullMultiplier = 1;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       hullMultiplier = 1; break;
        case HullClass::Destroyer:     hullMultiplier = 1; break;
        case HullClass::Cruiser:       hullMultiplier = 2; break;
        case HullClass::Battlecruiser: hullMultiplier = 2; break;
        case HullClass::Battleship:    hullMultiplier = 3; break;
        case HullClass::Capital:       hullMultiplier = 4; break;
        default:                       hullMultiplier = 2; break;
    }

    int totalDecals = baseCount * hullMultiplier;
    std::vector<DamageDecal> decals;
    decals.reserve(static_cast<size_t>(totalDecals));

    // Available decal types weighted by damage severity.
    for (int i = 0; i < totalDecals; ++i) {
        DamageDecal d{};

        // Select decal type.
        float roll = rng.nextFloat();
        if (level == DamageLevel::Light) {
            d.type = (roll < 0.6f) ? DecalType::ScorchMark : DecalType::ArmorCrack;
        } else if (level == DamageLevel::Moderate) {
            if      (roll < 0.30f) d.type = DecalType::ScorchMark;
            else if (roll < 0.55f) d.type = DecalType::ArmorCrack;
            else if (roll < 0.75f) d.type = DecalType::MissingPlate;
            else                   d.type = DecalType::HullBreach;
        } else {
            // Heavy / Critical — full range.
            if      (roll < 0.20f) d.type = DecalType::ScorchMark;
            else if (roll < 0.35f) d.type = DecalType::ArmorCrack;
            else if (roll < 0.50f) d.type = DecalType::MissingPlate;
            else if (roll < 0.70f) d.type = DecalType::HullBreach;
            else if (roll < 0.85f) d.type = DecalType::StructuralBend;
            else                   d.type = DecalType::ElectricalScar;
        }

        // Position along spine and laterally.
        d.position_x = rng.nextFloat();              // 0..1
        d.position_y = rng.rangeFloat(-1.0f, 1.0f);  // left–right

        // Zone assignment based on position.
        if      (d.position_x < 0.30f) d.zone_index = 0; // Command
        else if (d.position_x < 0.75f) d.zone_index = 1; // MidHull
        else                           d.zone_index = 2; // Engineering

        // Size scales with ship class.
        float baseSize = 0.5f;
        switch (baseHullClass(hull)) {
            case HullClass::Frigate:       baseSize = 0.3f; break;
            case HullClass::Destroyer:     baseSize = 0.4f; break;
            case HullClass::Cruiser:       baseSize = 0.6f; break;
            case HullClass::Battlecruiser: baseSize = 0.8f; break;
            case HullClass::Battleship:    baseSize = 1.2f; break;
            case HullClass::Capital:       baseSize = 2.0f; break;
            default:                       baseSize = 0.6f; break;
        }
        d.size = baseSize * rng.rangeFloat(0.5f, 1.5f);

        // Severity intensity.
        float levelBias = 0.0f;
        switch (level) {
            case DamageLevel::Pristine: levelBias = 0.0f; break;
            case DamageLevel::Light:    levelBias = 0.2f; break;
            case DamageLevel::Moderate: levelBias = 0.4f; break;
            case DamageLevel::Heavy:    levelBias = 0.6f; break;
            case DamageLevel::Critical: levelBias = 0.8f; break;
        }
        d.severity = std::min(1.0f, levelBias + rng.rangeFloat(0.0f, 0.2f));

        decals.push_back(d);
    }

    return decals;
}

// ── Structural integrity ────────────────────────────────────────────

float DamageStateGenerator::computeStructuralIntegrity(
        DamageLevel level, int breaches, int missingModules) {
    float base = 1.0f;
    switch (level) {
        case DamageLevel::Pristine: base = 1.0f; break;
        case DamageLevel::Light:    base = 0.90f; break;
        case DamageLevel::Moderate: base = 0.70f; break;
        case DamageLevel::Heavy:    base = 0.40f; break;
        case DamageLevel::Critical: base = 0.15f; break;
    }

    // Each breach and missing module further reduces integrity.
    float penalty = static_cast<float>(breaches) * 0.03f
                  + static_cast<float>(missingModules) * 0.02f;
    return std::max(0.0f, base - penalty);
}

// ── Public API ──────────────────────────────────────────────────────

std::string DamageStateGenerator::damageLevelName(DamageLevel level) {
    switch (level) {
        case DamageLevel::Pristine: return "Pristine";
        case DamageLevel::Light:    return "Light";
        case DamageLevel::Moderate: return "Moderate";
        case DamageLevel::Heavy:    return "Heavy";
        case DamageLevel::Critical: return "Critical";
    }
    return "Unknown";
}

std::string DamageStateGenerator::decalTypeName(DecalType type) {
    switch (type) {
        case DecalType::ScorchMark:     return "ScorchMark";
        case DecalType::ArmorCrack:     return "ArmorCrack";
        case DecalType::HullBreach:     return "HullBreach";
        case DecalType::MissingPlate:   return "MissingPlate";
        case DecalType::StructuralBend: return "StructuralBend";
        case DecalType::ElectricalScar: return "ElectricalScar";
    }
    return "Unknown";
}

GeneratedDamageState DamageStateGenerator::generate(const PCGContext& ctx,
                                                     float damageNorm,
                                                     HullClass hullClass) {
    DeterministicRNG rng(ctx.seed);

    // Clamp input.
    damageNorm = std::max(0.0f, std::min(1.0f, damageNorm));

    GeneratedDamageState state{};
    state.ship_id = ctx.seed;

    // 1. Classify damage level.
    state.level = classifyDamage(damageNorm);

    // 2. Hull breaches.
    state.hull_breach_count = computeBreachCount(rng, state.level, hullClass);

    // 3. Missing modules.
    state.missing_module_count = computeMissingModules(rng, state.level, hullClass);

    // 4. Generate decals.
    state.decals = generateDecals(rng, state.level, hullClass);

    // 5. Structural integrity.
    state.structural_integrity = computeStructuralIntegrity(
        state.level, state.hull_breach_count, state.missing_module_count);

    // 6. Visual wear: a composite 0–1 value for shader use.
    state.visual_wear = damageNorm;

    // 7. Validate.
    state.valid = (state.structural_integrity >= 0.0f)
               && (state.structural_integrity <= 1.0f);

    return state;
}

} // namespace pcg
} // namespace atlas
