#ifndef NOVAFORGE_PCG_SHIELD_EFFECT_GENERATOR_H
#define NOVAFORGE_PCG_SHIELD_EFFECT_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Visual pattern for the shield surface.
 */
enum class ShieldPattern : uint32_t {
    Hexagonal,   ///< Classic hexagonal grid (Veyren / tech).
    Smooth,      ///< Smooth energy dome (Aurelian / organic).
    Lattice,     ///< Lattice / cross-hatch (Keldari / industrial).
    Ornate,      ///< Ornate filigree pattern (Solari / decorative).
    Ripple,      ///< Concentric ripple rings.
};

/**
 * @brief An impact ripple on the shield surface.
 */
struct ShieldImpactRipple {
    float origin_x;    ///< Normalised hit position along spine [0,1].
    float origin_y;    ///< Normalised lateral hit position [-1,1].
    float intensity;   ///< Peak brightness [0,1].
    float radius;      ///< Max ripple radius (normalised) [0,1].
    float decay_rate;  ///< How fast the ripple fades (units/sec).
    float speed;       ///< Ripple expansion speed (normalised/sec).
};

/**
 * @brief Complete procedural shield visual specification.
 */
struct GeneratedShieldEffect {
    uint64_t      ship_id;
    ShieldPattern pattern;
    float         base_color_r, base_color_g, base_color_b;
    float         base_opacity;       ///< Idle shield visibility [0,1].
    float         hit_flash_color_r, hit_flash_color_g, hit_flash_color_b;
    float         shimmer_speed;      ///< Idle animation speed (Hz).
    float         shimmer_amplitude;  ///< Idle brightness variation [0,1].
    float         pattern_scale;      ///< Tile scale for the pattern.
    float         fresnel_power;      ///< Edge glow exponent.
    float         shield_radius;      ///< Normalised radius multiplier.
    std::vector<ShieldImpactRipple> sample_impacts; ///< Pre-generated samples.
    bool          valid;
};

/**
 * @brief Deterministic shield effect generator.
 *
 * Produces all parameters a renderer needs to display a procedural
 * shield bubble.  Faction controls colour and pattern; class controls
 * scale and density.
 *
 * Generation order:
 *   1. Select pattern from faction design language.
 *   2. Derive base colour (faction palette).
 *   3. Compute shimmer / animation parameters.
 *   4. Compute Fresnel and pattern scale from class.
 *   5. Generate sample impact ripples for testing.
 */
class ShieldEffectGenerator {
public:
    /** Generate shield parameters for a ship. */
    static GeneratedShieldEffect generate(const PCGContext& ctx,
                                          HullClass hull,
                                          const std::string& faction);

    /** Human-readable pattern name. */
    static std::string patternName(ShieldPattern pattern);

private:
    static ShieldPattern selectPattern(DeterministicRNG& rng,
                                       const std::string& faction);
    static void deriveBaseColor(DeterministicRNG& rng,
                                const std::string& faction,
                                float& r, float& g, float& b);
    static void deriveHitFlashColor(DeterministicRNG& rng,
                                    const std::string& faction,
                                    float& r, float& g, float& b);
    static float computePatternScale(HullClass hull);
    static float computeShieldRadius(HullClass hull);
    static std::vector<ShieldImpactRipple> generateSampleImpacts(
            DeterministicRNG& rng, int count);
};

} // namespace pcg
} // namespace atlas

#endif // NOVAFORGE_PCG_SHIELD_EFFECT_GENERATOR_H
