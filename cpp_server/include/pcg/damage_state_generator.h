#ifndef EVE_PCG_DAMAGE_STATE_GENERATOR_H
#define EVE_PCG_DAMAGE_STATE_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Overall damage severity classification.
 */
enum class DamageLevel : uint32_t {
    Pristine,   ///< No visible damage.
    Light,      ///< Cosmetic scratches and scorch marks.
    Moderate,   ///< Missing plates, minor hull breaches.
    Heavy,      ///< Large breaches, missing modules, structural cracks.
    Critical,   ///< Catastrophic — ship barely holding together.
};

/**
 * @brief Type of visual damage decal.
 */
enum class DecalType : uint32_t {
    ScorchMark,       ///< Blackened blast marks.
    ArmorCrack,       ///< Cracked armor plating.
    HullBreach,       ///< Hole in the hull.
    MissingPlate,     ///< Armor plate torn off.
    StructuralBend,   ///< Warped frame / bent girder.
    ElectricalScar,   ///< Burn lines from short circuits.
};

/**
 * @brief A single procedural damage decal on the hull.
 */
struct DamageDecal {
    DecalType  type;
    float      position_x;   ///< Normalised position along spine [0,1].
    float      position_y;   ///< Normalised lateral offset [-1,1].
    float      size;         ///< Decal radius (metres).
    float      severity;     ///< Intensity 0.0 – 1.0.
    int        zone_index;   ///< Which functional zone (0=Cmd, 1=Mid, 2=Eng).
};

/**
 * @brief Complete procedural damage state for a ship.
 */
struct GeneratedDamageState {
    uint64_t                 ship_id;
    DamageLevel              level;
    std::vector<DamageDecal> decals;
    int                      hull_breach_count;
    int                      missing_module_count;
    float                    structural_integrity; ///< 0.0 (destroyed) – 1.0 (pristine).
    float                    visual_wear;          ///< Accumulated cosmetic wear [0,1].
    bool                     valid;
};

/**
 * @brief Deterministic damage state generator (Phase 12).
 *
 * Given a seed and a normalised damage amount (0.0 – 1.0) produces
 * a set of damage decals and module-loss counts that can be applied
 * to any spine hull.  Deterministic: same seed + damage → same result.
 *
 * Generation order:
 *   1. Classify damage level from normalised amount.
 *   2. Determine hull breach and missing-module counts.
 *   3. Place scorch marks and cracks (scaled by damage).
 *   4. Apply zone-biased decal distribution.
 *   5. Compute structural integrity and visual wear.
 */
class DamageStateGenerator {
public:
    /**
     * @brief Generate damage state for a ship.
     * @param ctx         PCG context (seed + version).
     * @param damageNorm  Normalised damage 0.0 (pristine) – 1.0 (critical).
     * @param hullClass   Ship class (affects scale of damage).
     */
    static GeneratedDamageState generate(const PCGContext& ctx,
                                         float damageNorm,
                                         HullClass hullClass);

    /** Human-readable damage level name. */
    static std::string damageLevelName(DamageLevel level);

    /** Human-readable decal type name. */
    static std::string decalTypeName(DecalType type);

private:
    static DamageLevel classifyDamage(float damageNorm);
    static int         computeBreachCount(DeterministicRNG& rng,
                                          DamageLevel level, HullClass hull);
    static int         computeMissingModules(DeterministicRNG& rng,
                                             DamageLevel level, HullClass hull);
    static std::vector<DamageDecal> generateDecals(DeterministicRNG& rng,
                                                    DamageLevel level,
                                                    HullClass hull);
    static float       computeStructuralIntegrity(DamageLevel level,
                                                   int breaches,
                                                   int missingModules);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_DAMAGE_STATE_GENERATOR_H
