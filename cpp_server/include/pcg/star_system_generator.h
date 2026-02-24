#ifndef EVE_PCG_STAR_SYSTEM_GENERATOR_H
#define EVE_PCG_STAR_SYSTEM_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Star spectral classes ───────────────────────────────────────────
enum class StarClass : uint32_t {
    O = 0,   ///< Blue supergiant, extremely hot and luminous.
    B = 1,   ///< Blue-white giant.
    A = 2,   ///< White main-sequence.
    F = 3,   ///< Yellow-white.
    G = 4,   ///< Yellow (Sol-like).
    K = 5,   ///< Orange dwarf.
    M = 6,   ///< Red dwarf, most common.
};

static constexpr int STAR_CLASS_COUNT = 7;

// ── Planet types (for star system generation) ───────────────────────
// Note: distinct from planet_generator.h PlanetType for orbit-level
// classification vs. detailed planet generation.
enum class SystemPlanetType : uint32_t {
    Barren    = 0,
    Temperate = 1,
    Oceanic   = 2,
    Gas       = 3,
    Ice       = 4,
    Lava      = 5,
    Storm     = 6,
};

// ── Orbit slot types ────────────────────────────────────────────────
enum class OrbitSlotType : uint32_t {
    Empty  = 0,
    Planet = 1,
    Belt   = 2,
    Moon   = 3,
};

// ── Star data ───────────────────────────────────────────────────────
struct StarData {
    StarClass starClass;
    float     luminosity;    ///< Solar luminosities.
    float     temperature;   ///< Kelvin.
    float     radius;        ///< Solar radii.
};

// ── Orbit slot ──────────────────────────────────────────────────────
struct OrbitSlot {
    int          orbitIndex;
    float        orbitRadius;  ///< AU from the star.
    OrbitSlotType type;
    SystemPlanetType   planetType;   ///< Only meaningful when type == Planet.
};

// ── System station ──────────────────────────────────────────────────
struct SystemStation {
    uint64_t    stationId;
    int         orbitIndex;
    std::string faction;
};

// ── Jump gate ───────────────────────────────────────────────────────
struct JumpGate {
    uint64_t gateId;
    uint64_t destinationSystemId;
    int      orbitIndex;
};

// ── Full generated star system ──────────────────────────────────────
struct GeneratedStarSystem {
    uint64_t                  systemId;
    uint64_t                  seed;
    StarData                  star;
    std::vector<OrbitSlot>    orbitSlots;
    std::vector<SystemStation> stations;
    std::vector<JumpGate>     gates;
    float                     securityLevel;   ///< 0.0 (null) – 1.0 (high).
    uint64_t                  constellationId;
    int                       totalPlanets;
    int                       totalBelts;
    bool                      valid;
};

/**
 * @brief Deterministic star-system generator.
 *
 * Generates a complete star system — star, orbital slots (planets,
 * asteroid belts, moons), stations, and jump gates — from a single
 * PCGContext seed.  The same context always yields the same system.
 *
 * Generation follows the hierarchy:
 *   1. Select star class (weighted by real stellar distribution)
 *   2. Derive star properties (luminosity, temperature, radius)
 *   3. Generate orbit slots with planet-type zoning
 *   4. Place stations (count depends on security level)
 *   5. Place jump gates (count depends on security level)
 */
class StarSystemGenerator {
public:
    /** Generate a star system from the given context and security level. */
    static GeneratedStarSystem generate(const PCGContext& ctx,
                                        float securityLevel);

    /** Generate with an explicit star class override. */
    static GeneratedStarSystem generate(const PCGContext& ctx,
                                        float securityLevel,
                                        StarClass starOverride);

    /** Human-readable star class name. */
    static std::string starClassName(StarClass sc);

private:
    static StarClass selectStarClass(DeterministicRNG& rng);
    static void      generateOrbits(DeterministicRNG& rng,
                                    GeneratedStarSystem& sys,
                                    StarClass sc);
    static void      placeStations(DeterministicRNG& rng,
                                   GeneratedStarSystem& sys);
    static void      placeGates(DeterministicRNG& rng,
                                GeneratedStarSystem& sys);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_STAR_SYSTEM_GENERATOR_H
