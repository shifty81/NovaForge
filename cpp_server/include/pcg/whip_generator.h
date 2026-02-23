#ifndef ATLAS_PCG_WHIP_GENERATOR_H
#define ATLAS_PCG_WHIP_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Whip style (determines visual and mechanical properties).
 */
enum class WhipStyle : uint32_t {
    Energy,       ///< Plasma/energy tendril — high EM damage.
    Kinetic,      ///< Physical cable/chain — high kinetic damage.
    Thermal,      ///< Superheated filament — high thermal damage.
    Gravimetric,  ///< Gravity-tether — pulls targets closer.
};

/**
 * @brief Profile describing a procedurally generated whip weapon.
 */
struct WhipProfile {
    WhipStyle   style;
    float       reach;           ///< Maximum reach in metres.
    float       arc_degrees;     ///< Sweep arc (degrees, up to 360).
    float       base_damage;     ///< Per-hit damage.
    float       swing_time;      ///< Seconds per swing cycle.
    float       tracking_speed;  ///< Tracking speed (rad/s).
    int         tendril_count;   ///< Number of tendrils / lashes.
    std::string faction_style;   ///< Visual faction hints.
};

/**
 * @brief Fully generated whip weapon with fitting costs.
 */
struct GeneratedWhip {
    uint64_t     whip_id;
    WhipProfile  profile;
    float        power_draw;    ///< MW consumed.
    float        cpu_usage;     ///< tf consumed.
    float        capacitor_per_cycle;  ///< GJ per swing.
    float        tendril_length;       ///< Visual tendril length (metres).
    float        base_width;           ///< Base mount width.
    float        mass;                 ///< Additional mass (kg).
    bool         valid;                ///< true if constraints passed.
};

/**
 * @brief Deterministic character whip weapon generator.
 *
 * Whips are character-held melee weapons used during on-foot gameplay
 * (station interiors, ship interiors, planetary exploration).  They are
 * NOT ship weapons — spaceships use turrets, missiles, and energy beams.
 *
 * Given a seed, whip style, and faction, produces a fully-specified
 * GeneratedWhip.  The same inputs always yield the same whip.
 *
 * Generation follows the hierarchy:
 *   1. Select whip style
 *   2. Derive reach & damage from style
 *   3. Compute swing time & tracking
 *   4. Apply faction modifiers
 *   5. Validate fitting constraints
 */
class WhipGenerator {
public:
    /** Generate a whip from seed, style, and faction. */
    GeneratedWhip generate(uint64_t seed, WhipStyle style,
                           const std::string& faction) const;

    /** Generate with a random style chosen by seed. */
    GeneratedWhip generateRandom(uint64_t seed,
                                 const std::string& faction) const;

    /** Compute base reach for a given style. */
    static float computeBaseReach(WhipStyle style);

    /** Compute tendril count for a given seed and style. */
    static int computeTendrilCount(uint64_t seed, WhipStyle style);

    /** Human-readable whip style name. */
    static const char* styleName(WhipStyle style);
};

} // namespace pcg
} // namespace atlas

#endif // ATLAS_PCG_WHIP_GENERATOR_H
