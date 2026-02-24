#ifndef EVE_PCG_GALAXY_GENERATOR_H
#define EVE_PCG_GALAXY_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Security zones ──────────────────────────────────────────────────
enum class SecurityZone : uint32_t {
    HighSec = 0,   ///< Empire-controlled, CONCORD-protected.
    LowSec  = 1,   ///< Faction sovereignty, limited policing.
    NullSec = 2,   ///< Lawless, player-controlled.
};

// ── Galaxy node (star system vertex) ────────────────────────────────
struct GalaxyNode {
    uint64_t              system_id;
    float                 x, y, z;          ///< Position in light-years.
    SecurityZone          security;
    float                 security_level;    ///< 0.0 (null) – 1.0 (high).
    std::vector<uint64_t> neighbor_ids;
    uint32_t              constellation_id;
    uint32_t              region_id;
};

// ── Galaxy route (edge between systems) ─────────────────────────────
struct GalaxyRoute {
    uint64_t from_id;
    uint64_t to_id;
    float    distance;       ///< Light-years.
    bool     is_chokepoint;  ///< Removing this route would disconnect regions.
};

// ── Full generated galaxy graph ─────────────────────────────────────
struct GeneratedGalaxy {
    uint64_t                  seed;
    std::vector<GalaxyNode>   nodes;
    std::vector<GalaxyRoute>  routes;
    int                       total_systems;
    int                       highsec_count;
    int                       lowsec_count;
    int                       nullsec_count;
    bool                      valid;
};

/**
 * @brief Deterministic galaxy graph generator.
 *
 * Generates a connected graph of star systems arranged in a disc with
 * security-zone concentric rings.  Core systems are high-sec, the
 * middle ring is low-sec, and the outer edge is null-sec.  Systems are
 * grouped into constellations and regions.  Deterministic: the same
 * PCGContext always yields the same galaxy.
 *
 * Generation follows:
 *   1. Spread system positions in a disc using deterministic RNG
 *   2. Build connections between nearby systems (distance threshold)
 *   3. Assign security zones based on radial distance from core
 *   4. Group systems into constellations and regions
 *   5. Identify chokepoint routes
 */
class GalaxyGenerator {
public:
    /**
     * @brief Generate a galaxy graph.
     * @param ctx          PCG context (seed drives all randomness).
     * @param systemCount  Desired number of systems (clamped to [10, 500]).
     */
    static GeneratedGalaxy generate(const PCGContext& ctx, int systemCount);

    /** Human-readable security zone name. */
    static std::string securityZoneName(SecurityZone zone);

private:
    static void generatePositions(DeterministicRNG& rng,
                                  GeneratedGalaxy& galaxy,
                                  int systemCount,
                                  uint64_t baseSeed);
    static void buildConnections(DeterministicRNG& rng,
                                 GeneratedGalaxy& galaxy);
    static void assignSecurity(GeneratedGalaxy& galaxy);
    static void identifyChokepoints(GeneratedGalaxy& galaxy);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_GALAXY_GENERATOR_H
