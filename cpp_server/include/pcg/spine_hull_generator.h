#ifndef EVE_PCG_SPINE_HULL_GENERATOR_H
#define EVE_PCG_SPINE_HULL_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Spine archetypes that define a ship's silhouette purpose.
 *
 * The spine is the backbone of the hull grammar — it determines the
 * overall shape before any detail is added.
 */
enum class SpineType : uint32_t {
    Needle,   ///< Long, thin — interceptors, fast-attack.
    Wedge,    ///< Front-heavy taper — assault ships.
    Hammer,   ///< Bulky front, narrow rear — brawlers.
    Slab,     ///< Wide, flat — haulers, carriers.
    Ring,     ///< Toroidal / radial — stations, special hulls.
};

/**
 * @brief Functional zones always appear in this fixed order
 *        along the spine: Command → MidHull → Engineering.
 */
enum class FunctionalZone : uint32_t {
    Command,      ///< Bridge, sensors, forward weapons.
    MidHull,      ///< Crew, cargo, weapon bays.
    Engineering,  ///< Engines, reactors, fuel.
};

/**
 * @brief Width profile sampled at the three canonical spine positions.
 */
struct SpineProfile {
    float length;       ///< Total spine length (metres).
    float width_fwd;    ///< Width at the command (forward) end.
    float width_mid;    ///< Width at mid-hull.
    float width_aft;    ///< Width at the engineering (aft) end.
};

/**
 * @brief One functional zone placed along the spine.
 */
struct HullZone {
    FunctionalZone zone;
    float length_fraction;  ///< Fraction of total spine length [0,1].
    int   greeble_count;    ///< Cosmetic details (antennas, plates …).
};

/**
 * @brief Complete output of the spine-based hull grammar.
 */
struct GeneratedSpineHull {
    uint64_t      hull_id;
    SpineType     spine;
    HullClass     hull_class;
    SpineProfile  profile;
    std::vector<HullZone> zones;        ///< Always 3: Cmd, Mid, Eng.
    bool          bilateral_symmetry;   ///< Always true for ships.
    float         aspect_ratio;         ///< length / width_mid.
    int           engine_cluster_count; ///< Engines at the aft end.
    int           total_greeble_count;  ///< Sum across all zones.
    std::string   faction_style;        ///< Faction shape language tag.
    bool          valid;                ///< true if all constraints pass.
};

/**
 * @brief Spine-based hull grammar generator (Phase 12).
 *
 * Replaces blob-assembly with a silhouette-first approach:
 *   1. Select spine archetype from hull class.
 *   2. Derive width profile & aspect ratio.
 *   3. Lay out functional zones (Command → MidHull → Engineering).
 *   4. Generate engine cluster at the aft end.
 *   5. Cosmetic greeble pass (last — never affects stats).
 *   6. Apply faction shape language.
 *   7. Enforce bilateral symmetry and aspect-ratio clamping.
 *
 * Deterministic: same PCGContext always yields the same hull.
 */
class SpineHullGenerator {
public:
    /** Generate a spine hull from context (hull class auto-selected). */
    static GeneratedSpineHull generate(const PCGContext& ctx);

    /** Generate with an explicit hull class. */
    static GeneratedSpineHull generate(const PCGContext& ctx, HullClass hull);

    /** Generate with explicit hull class and faction style. */
    static GeneratedSpineHull generate(const PCGContext& ctx,
                                       HullClass hull,
                                       const std::string& faction);

    /** Human-readable spine type name. */
    static std::string spineTypeName(SpineType spine);

private:
    static SpineType   selectSpine(DeterministicRNG& rng, HullClass hull);
    static SpineProfile deriveProfile(DeterministicRNG& rng,
                                      SpineType spine, HullClass hull);
    static std::vector<HullZone> layoutZones(DeterministicRNG& rng,
                                             HullClass hull);
    static int          generateEngineCluster(DeterministicRNG& rng,
                                              HullClass hull);
    static void         applyFactionStyle(DeterministicRNG& rng,
                                          GeneratedSpineHull& hull,
                                          const std::string& faction);
    static void         clampAspectRatio(GeneratedSpineHull& hull);
    static bool         validate(const GeneratedSpineHull& hull);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SPINE_HULL_GENERATOR_H
