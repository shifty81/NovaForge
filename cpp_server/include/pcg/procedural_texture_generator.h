#ifndef NOVAFORGE_PCG_PROCEDURAL_TEXTURE_GENERATOR_H
#define NOVAFORGE_PCG_PROCEDURAL_TEXTURE_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Faction-specific colour palette for ship textures.
 */
struct FactionColorPalette {
    float primary_r,   primary_g,   primary_b;    ///< Main hull colour.
    float secondary_r, secondary_g, secondary_b;  ///< Accent / trim colour.
    float accent_r,    accent_g,    accent_b;      ///< Detail highlights.
    float emissive_r,  emissive_g,  emissive_b;    ///< Engine glow / windows.
};

/**
 * @brief PBR material properties for a generated hull surface.
 */
struct HullMaterialParams {
    float metalness;    ///< 0.0 (dielectric) – 1.0 (fully metallic).
    float roughness;    ///< 0.0 (mirror) – 1.0 (diffuse).
    float wear;         ///< Accumulated surface wear [0,1].
    float panel_depth;  ///< Normal-map panel groove depth [0,1].
};

/**
 * @brief Type of procedural hull marking / decal.
 */
enum class MarkingType : uint32_t {
    StripeHorizontal,  ///< Racing / faction stripe.
    StripeVertical,    ///< Vertical identification bar.
    RegistrationCode,  ///< Hull serial number area.
    FactionInsignia,   ///< Faction logo placeholder.
    WarningHazard,     ///< Hazard / warning paint.
};

/**
 * @brief A procedural marking placed on the hull.
 */
struct HullMarking {
    MarkingType type;
    float position_x;  ///< Normalised position along spine [0,1].
    float position_y;  ///< Normalised lateral offset [-1,1].
    float width;       ///< Normalised width [0,1].
    float height;      ///< Normalised height [0,1].
    float color_r, color_g, color_b;
};

/**
 * @brief Engine glow visual parameters.
 */
struct EngineGlowParams {
    float intensity;   ///< Glow brightness [0,1].
    float color_r, color_g, color_b;
    float core_radius; ///< Core bright area fraction [0,1].
    float halo_radius; ///< Outer halo extent fraction [0,1].
    float pulse_rate;  ///< Pulse frequency (Hz), 0 = steady.
};

/**
 * @brief Window / running-light placement on the hull.
 */
struct WindowLight {
    float position_x;  ///< Along spine [0,1].
    float position_y;  ///< Lateral [-1,1].
    float size;        ///< Normalised size.
    float color_r, color_g, color_b;
    int   zone_index;  ///< 0=Command, 1=MidHull, 2=Engineering.
};

/**
 * @brief Complete procedural texture specification for a ship.
 */
struct GeneratedTextureParams {
    uint64_t                    ship_id;
    std::string                 faction;
    HullClass                   hull_class;
    FactionColorPalette         palette;
    HullMaterialParams          material;
    EngineGlowParams            engine_glow;
    std::vector<HullMarking>    markings;
    std::vector<WindowLight>    window_lights;
    int                         panel_tile_count; ///< UV tiling count for panels.
    bool                        valid;
};

/**
 * @brief Deterministic procedural texture parameter generator (Phase 4).
 *
 * Produces all the data a renderer needs to procedurally texture a ship
 * hull without any hand-authored bitmaps.  Deterministic: same context
 * always yields the same output.
 *
 * Generation order:
 *   1. Select faction colour palette (with per-ship variation).
 *   2. Derive PBR material properties from faction + class.
 *   3. Place hull markings (stripes, insignia, registration).
 *   4. Generate engine glow parameters.
 *   5. Distribute window / running lights.
 *   6. Compute UV panel tiling from class size.
 */
class ProceduralTextureGenerator {
public:
    /** Generate texture parameters for a ship. */
    static GeneratedTextureParams generate(const PCGContext& ctx,
                                           HullClass hull,
                                           const std::string& faction);

    /** Get the base colour palette for a faction (no per-ship variation). */
    static FactionColorPalette basePalette(const std::string& faction);

    /** Human-readable marking type name. */
    static std::string markingTypeName(MarkingType type);

private:
    static FactionColorPalette derivepalette(DeterministicRNG& rng,
                                             const std::string& faction);
    static HullMaterialParams  deriveMaterial(DeterministicRNG& rng,
                                              HullClass hull,
                                              const std::string& faction);
    static std::vector<HullMarking> generateMarkings(DeterministicRNG& rng,
                                                      HullClass hull,
                                                      const FactionColorPalette& pal);
    static EngineGlowParams    generateEngineGlow(DeterministicRNG& rng,
                                                   const std::string& faction);
    static std::vector<WindowLight> generateWindowLights(DeterministicRNG& rng,
                                                          HullClass hull);
    static int                 computePanelTiling(HullClass hull);
};

} // namespace pcg
} // namespace atlas

#endif // NOVAFORGE_PCG_PROCEDURAL_TEXTURE_GENERATOR_H
