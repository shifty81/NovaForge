#ifndef EVE_PCG_GRAV_BIKE_GENERATOR_H
#define EVE_PCG_GRAV_BIKE_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>

namespace atlas {
namespace pcg {

struct GravBikeConfig {
    float max_speed;
    float acceleration;
    float fuel_capacity;
    float cargo_capacity;
    bool has_weapon_mount;
    bool has_scanner;
    std::string faction_style;
};

struct GeneratedGravBike {
    uint64_t bike_id;
    GravBikeConfig config;
    float hull_strength;
    float shield_strength;
    float turn_rate;
};

class GravBikeGenerator {
public:
    GeneratedGravBike generate(uint64_t seed, const std::string& faction) const;
    static float computeMaxSpeed(uint64_t seed);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_GRAV_BIKE_GENERATOR_H
