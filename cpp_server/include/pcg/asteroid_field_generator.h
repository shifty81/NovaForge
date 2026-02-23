#ifndef EVE_PCG_ASTEROID_FIELD_GENERATOR_H
#define EVE_PCG_ASTEROID_FIELD_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Asteroid mineral types ──────────────────────────────────────────
enum class AsteroidType : uint32_t {
    Veldspar,      ///< Common, high-sec.
    Scordite,      ///< Common, high-sec.
    Pyroxeres,     ///< Uncommon, mid-sec.
    Plagioclase,   ///< Uncommon, mid-sec.
    Omber,         ///< Rare, low-sec.
    Kernite,       ///< Rare, low-sec.
    Jaspet,        ///< Very rare, null-sec.
    Mercoxit,      ///< Exceedingly rare, null-sec only.
};

// ── Single asteroid in a belt ───────────────────────────────────────
struct AsteroidNode {
    int           asteroidId;
    AsteroidType  type;
    float         radius;        ///< Metres.
    float         mineralYield;  ///< Units of ore available.
    float         posX, posY, posZ;
};

// ── Generated asteroid field / belt ─────────────────────────────────
struct AsteroidField {
    uint64_t                   fieldId;
    std::vector<AsteroidNode>  asteroids;
    float                      fieldRadius;   ///< Bounding radius of the entire field.
    int                        totalAsteroids;
    float                      totalYield;
};

/**
 * @brief Deterministic asteroid-belt generator.
 *
 * Places asteroids in a disc-shaped region with mineral types
 * determined by the security level parameter.  The same PCGContext
 * always produces the same field layout.
 */
class AsteroidFieldGenerator {
public:
    /**
     * @brief Generate a belt with a procedurally chosen asteroid count.
     * @param ctx       PCG context (seed + version).
     * @param secLevel  Security level 0.0 (null) – 1.0 (high-sec).
     */
    static AsteroidField generate(const PCGContext& ctx, float secLevel);

    /**
     * @brief Generate a belt with an explicit asteroid count.
     * @param ctx            PCG context.
     * @param asteroidCount  Number of asteroids to place.
     * @param secLevel       Security level 0.0 – 1.0.
     */
    static AsteroidField generate(const PCGContext& ctx,
                                  int asteroidCount,
                                  float secLevel);

    /** Compute total mineral yield for a field. */
    static float calculateTotalYield(const AsteroidField& field);

private:
    static AsteroidType selectType(DeterministicRNG& rng, float secLevel);
    static float        rollRadius(DeterministicRNG& rng, AsteroidType type);
    static float        rollYield(DeterministicRNG& rng, AsteroidType type);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_ASTEROID_FIELD_GENERATOR_H
