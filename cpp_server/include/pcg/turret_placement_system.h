#ifndef EVE_PCG_TURRET_PLACEMENT_SYSTEM_H
#define EVE_PCG_TURRET_PLACEMENT_SYSTEM_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include "turret_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief A single turret mount placed on a ship hull.
 */
struct TurretMount {
    uint32_t    socket_id;
    float       x_offset;       ///< Along spine (0 = centre, + = forward).
    float       y_offset;       ///< Lateral (0 = centreline).
    float       z_offset;       ///< Vertical (+ = dorsal).
    float       direction_deg;  ///< Nominal facing (degrees, 0 = forward).
    float       arc_deg;        ///< Total arc width (degrees).
    TurretSize  size;
};

/**
 * @brief Result of placing turrets on a ship hull.
 */
struct TurretPlacement {
    uint64_t                 ship_id;
    std::vector<TurretMount> mounts;
    float                    coverage_score; ///< [0, 1] — fraction of 360° covered.
    float                    max_overlap;    ///< Worst pairwise arc overlap [0, 1].
    bool                     valid;          ///< true if overlap < threshold.
};

/**
 * @brief Deterministic turret placement system (Phase 15).
 *
 * Places turret sockets on a ship hull based on class and faction:
 *   1. Determine mount count from hull class.
 *   2. Distribute mounts along the hull spine.
 *   3. Assign facing and arc per mount (role-dependent).
 *   4. Apply faction placement rules.
 *   5. Validate arc overlap (must be < 30%).
 *
 * Deterministic: same inputs always yield the same placement.
 */
class TurretPlacementSystem {
public:
    /** Place turrets for a generated ship. */
    static TurretPlacement place(const PCGContext& ctx,
                                 HullClass hull,
                                 int turretSlots,
                                 const std::string& faction = "");

    /** Compute the angular coverage score for a set of mounts. */
    static float computeCoverage(const std::vector<TurretMount>& mounts);

    /** Compute worst pairwise arc overlap [0, 1]. */
    static float computeMaxOverlap(const std::vector<TurretMount>& mounts);

private:
    static TurretSize sizeForHull(DeterministicRNG& rng, HullClass hull);
    static void       distributeMounts(DeterministicRNG& rng,
                                       TurretPlacement& placement,
                                       HullClass hull,
                                       int count,
                                       TurretSize size);
    static void       applyFactionRules(DeterministicRNG& rng,
                                        TurretPlacement& placement,
                                        const std::string& faction);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_TURRET_PLACEMENT_SYSTEM_H
