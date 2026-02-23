#include "pcg/turret_generator.h"

#include <algorithm>

namespace atlas {
namespace pcg {

// ── Public API ─────────────────────────────────────────────────────

float TurretGenerator::computeOptimalRange(TurretSize size, TurretType type) {
    float base = 5000.0f;
    switch (size) {
        case TurretSize::Small:   base = 5000.0f;  break;
        case TurretSize::Medium:  base = 15000.0f; break;
        case TurretSize::Large:   base = 35000.0f; break;
        case TurretSize::Capital: base = 60000.0f; break;
    }
    switch (type) {
        case TurretType::Energy:  base *= 1.2f; break;
        case TurretType::Missile: base *= 1.5f; break;
        case TurretType::Mining:  base *= 0.4f; break;
        case TurretType::Whip:    base *= 0.3f; break;  // Close-range weapon
        default: break;
    }
    return base;
}

int TurretGenerator::computeBarrelCount(uint64_t seed, TurretSize size) {
    DeterministicRNG rng(seed);
    switch (size) {
        case TurretSize::Small:   return rng.range(1, 2);
        case TurretSize::Medium:  return rng.range(1, 3);
        case TurretSize::Large:   return rng.range(2, 4);
        case TurretSize::Capital: return rng.range(1, 2);
    }
    return 1;
}

GeneratedTurret TurretGenerator::generate(uint64_t seed, TurretSize size, TurretType type, const std::string& faction) const {
    DeterministicRNG rng(seed);

    GeneratedTurret turret{};
    turret.turret_id = seed;

    turret.profile.size = size;
    turret.profile.type = type;
    turret.profile.faction_style = faction;
    turret.profile.barrel_count = computeBarrelCount(seed, size);

    // Base stats by size
    float sizeMultiplier = 1.0f;
    switch (size) {
        case TurretSize::Small:   sizeMultiplier = 1.0f; break;
        case TurretSize::Medium:  sizeMultiplier = 2.5f; break;
        case TurretSize::Large:   sizeMultiplier = 5.0f; break;
        case TurretSize::Capital: sizeMultiplier = 10.0f; break;
    }

    turret.profile.base_damage = rng.rangeFloat(50.0f, 100.0f) * sizeMultiplier;
    turret.profile.tracking_speed = rng.rangeFloat(20.0f, 60.0f) / sizeMultiplier;
    turret.profile.rate_of_fire = rng.rangeFloat(0.5f, 2.0f) / sizeMultiplier;
    turret.profile.arc_degrees = rng.rangeFloat(90.0f, 360.0f);

    turret.optimal_range = computeOptimalRange(size, type);
    turret.falloff_range = turret.optimal_range * rng.rangeFloat(0.3f, 0.6f);
    turret.power_draw = rng.rangeFloat(10.0f, 30.0f) * sizeMultiplier;
    turret.cpu_usage = rng.rangeFloat(5.0f, 20.0f) * sizeMultiplier;

    turret.barrel_length = rng.rangeFloat(1.0f, 5.0f) * sizeMultiplier;
    turret.base_width = rng.rangeFloat(0.5f, 2.0f) * sizeMultiplier;
    turret.base_height = rng.rangeFloat(0.3f, 1.5f) * sizeMultiplier;

    // Faction modifiers
    if (faction == "Solari") {
        turret.profile.base_damage *= 1.1f;
        turret.profile.tracking_speed *= 0.95f;
    } else if (faction == "Veyren") {
        turret.profile.tracking_speed *= 1.05f;
        turret.profile.rate_of_fire *= 1.05f;
    } else if (faction == "Aurelian") {
        turret.profile.tracking_speed *= 1.15f;
        turret.profile.base_damage *= 0.9f;
    } else if (faction == "Keldari") {
        turret.optimal_range *= 1.2f;
        turret.profile.rate_of_fire *= 0.9f;
    }

    // Whip-type adjustments: close-range, wide arc, fast tracking
    if (type == TurretType::Whip) {
        turret.profile.arc_degrees = std::min(turret.profile.arc_degrees * 1.5f, 360.0f);
        turret.profile.tracking_speed *= 1.3f;
        turret.optimal_range *= 0.3f;
        turret.falloff_range *= 0.5f;
    }

    return turret;
}

} // namespace pcg
} // namespace atlas
