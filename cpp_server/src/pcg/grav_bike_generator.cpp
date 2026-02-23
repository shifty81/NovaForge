#include "pcg/grav_bike_generator.h"

namespace atlas {
namespace pcg {

// ── Public API ─────────────────────────────────────────────────────

float GravBikeGenerator::computeMaxSpeed(uint64_t seed) {
    DeterministicRNG rng(seed);
    return rng.rangeFloat(30.0f, 80.0f);
}

GeneratedGravBike GravBikeGenerator::generate(uint64_t seed, const std::string& faction) const {
    DeterministicRNG rng(seed);

    GeneratedGravBike bike{};
    bike.bike_id = seed;

    bike.config.max_speed = rng.rangeFloat(30.0f, 80.0f);
    bike.config.acceleration = rng.rangeFloat(5.0f, 15.0f);
    bike.config.fuel_capacity = rng.rangeFloat(50.0f, 200.0f);
    bike.config.cargo_capacity = rng.rangeFloat(50.0f, 300.0f);
    bike.config.has_weapon_mount = rng.chance(0.4f);
    bike.config.has_scanner = rng.chance(0.6f);
    bike.config.faction_style = faction;

    bike.hull_strength = rng.rangeFloat(100.0f, 500.0f);
    bike.shield_strength = rng.rangeFloat(50.0f, 300.0f);
    bike.turn_rate = rng.rangeFloat(60.0f, 180.0f);

    // Faction modifiers
    if (faction == "Solari") {
        bike.config.max_speed *= 1.1f;
        bike.hull_strength *= 0.9f;
    } else if (faction == "Veyren") {
        bike.config.acceleration *= 1.15f;
        bike.shield_strength *= 1.1f;
    } else if (faction == "Aurelian") {
        bike.turn_rate *= 1.2f;
        bike.config.cargo_capacity *= 0.8f;
    } else if (faction == "Keldari") {
        bike.hull_strength *= 1.2f;
        bike.config.max_speed *= 0.9f;
    }

    return bike;
}

} // namespace pcg
} // namespace atlas
