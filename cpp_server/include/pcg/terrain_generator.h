#ifndef EVE_PCG_TERRAIN_GENERATOR_H
#define EVE_PCG_TERRAIN_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "planet_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Surface biome types generated from height + moisture.
 */
enum class BiomeType : uint32_t {
    Plains,
    Mountains,
    Valleys,
    Plateaus,
    Craters,
    Dunes,
    Tundra,
    Volcanic,
};

/**
 * @brief A single terrain cell on the planet surface grid.
 */
struct TerrainCell {
    float       height;         ///< Normalised height [0, 1].
    float       moisture;       ///< Normalised moisture [0, 1].
    BiomeType   biome;
    bool        mineable;       ///< Contains an extractable deposit.
    std::string resource_type;  ///< Non-empty when mineable is true.
};

/**
 * @brief A coarse region of the terrain grid.
 */
struct TerrainRegion {
    int   grid_x;
    int   grid_y;
    std::vector<TerrainCell> cells;
    float avg_height;
    float max_height;
    bool  has_landing_zone;     ///< Flat enough for ship landing.
};

/**
 * @brief Complete terrain output for one planet.
 */
struct GeneratedTerrain {
    uint64_t  planet_id;
    int       grid_size;            ///< Side length of the terrain grid.
    std::vector<TerrainRegion> regions;
    int       biome_counts[8];      ///< Per-BiomeType cell counts.
    int       mineable_count;       ///< Total mineable cells.
    int       landing_zone_count;   ///< Regions suitable for landing.
    bool      valid;
};

/**
 * @brief Noise-based planet terrain generator (Phase 14).
 *
 * Generates a deterministic heightmap grid for a planet surface:
 *   1. Compute height via layered deterministic noise.
 *   2. Compute moisture from a separate noise pass.
 *   3. Classify biomes from height × moisture × planet type.
 *   4. Place mineable deposits based on planet resources.
 *   5. Identify flat regions as landing zones.
 *
 * All output is a pure function of (seed, planet type, grid size).
 */
class TerrainGenerator {
public:
    /** Generate terrain for a planet. */
    static GeneratedTerrain generate(uint64_t seed,
                                     PlanetType type,
                                     int gridSize = 8);

    /** Classify a biome from height and moisture. */
    static BiomeType classifyBiome(float height, float moisture,
                                   PlanetType type);

    /** Human-readable biome name. */
    static std::string biomeName(BiomeType biome);

private:
    /** Deterministic 2D noise value in [0,1]. */
    static float noise2D(uint64_t seed, int x, int y);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_TERRAIN_GENERATOR_H
