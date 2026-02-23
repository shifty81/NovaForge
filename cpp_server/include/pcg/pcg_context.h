#ifndef EVE_PCG_CONTEXT_H
#define EVE_PCG_CONTEXT_H

#include <cstdint>

namespace atlas {
namespace pcg {

/**
 * @brief Domain identifiers for the PCG seed hierarchy.
 *
 * Each domain produces an independent seed sub-tree so that, for
 * example, changing asteroid generation rules never affects ship seeds.
 */
enum class PCGDomain : uint32_t {
    Universe    = 0,
    Galaxy      = 1,
    StarSystem  = 2,
    Sector      = 3,
    Ship        = 4,
    Asteroid    = 5,
    NPC         = 6,
    Fleet       = 7,
    Loot        = 8,
    Mission     = 9,
    CapitalShip = 10,
    Station     = 11,
    Salvage     = 12,
    Rover       = 13,
    Anomaly     = 14,
    Encounter   = 15,
    SpineHull    = 16,
    Terrain      = 17,
    DamageState  = 18,
    EconomyFleet = 19,
};

/**
 * @brief Immutable context passed to every procedural generator.
 *
 * Carries the derived seed and rules-version so that a generator
 * can produce bit-identical output on any machine, at any time.
 */
struct PCGContext {
    uint64_t seed;      ///< Derived seed for this object / scope.
    uint32_t version;   ///< Rules version (for migration safety).
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_CONTEXT_H
