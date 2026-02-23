#ifndef EVE_PCG_TURRET_GENERATOR_H
#define EVE_PCG_TURRET_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>

namespace atlas {
namespace pcg {

enum class TurretSize : uint32_t { Small, Medium, Large, Capital };
enum class TurretType : uint32_t { Projectile, Energy, Missile, Hybrid, Mining };

struct TurretProfile {
    TurretSize size;
    TurretType type;
    float arc_degrees;
    float tracking_speed;
    float base_damage;
    float rate_of_fire;
    int barrel_count;
    std::string faction_style;
};

struct GeneratedTurret {
    uint64_t turret_id;
    TurretProfile profile;
    float optimal_range;
    float falloff_range;
    float power_draw;
    float cpu_usage;
    float barrel_length;
    float base_width;
    float base_height;
};

class TurretGenerator {
public:
    GeneratedTurret generate(uint64_t seed, TurretSize size, TurretType type, const std::string& faction) const;
    static float computeOptimalRange(TurretSize size, TurretType type);
    static int computeBarrelCount(uint64_t seed, TurretSize size);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_TURRET_GENERATOR_H
