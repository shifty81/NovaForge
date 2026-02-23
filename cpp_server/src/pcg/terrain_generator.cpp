#include "pcg/terrain_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Deterministic 2D noise ──────────────────────────────────────────

float TerrainGenerator::noise2D(uint64_t seed, int x, int y) {
    // Hash-based noise: deterministic, platform-independent.
    uint64_t h = hashCombine(seed, static_cast<uint64_t>(x * 73856093));
    h = hashCombine(h, static_cast<uint64_t>(y * 19349663));
    // Map to [0, 1].
    return static_cast<float>(h & 0xFFFFFF) / static_cast<float>(0xFFFFFF);
}

// ── Biome classification ────────────────────────────────────────────

BiomeType TerrainGenerator::classifyBiome(float height, float moisture,
                                           PlanetType type) {
    // Planet type overrides for extreme environments.
    switch (type) {
        case PlanetType::Lava:
            return (height > 0.7f) ? BiomeType::Volcanic : BiomeType::Craters;
        case PlanetType::Ice:
            return (height > 0.6f) ? BiomeType::Mountains : BiomeType::Tundra;
        case PlanetType::Desert:
            return (height > 0.6f) ? BiomeType::Plateaus : BiomeType::Dunes;
        case PlanetType::Ocean:
            return (height > 0.5f) ? BiomeType::Plains : BiomeType::Valleys;
        case PlanetType::Gas:
            // Gas giants have no walkable surface — placeholder.
            return BiomeType::Plains;
        default:
            break;
    }

    // Generic classification for Rocky / Forest / Barren.
    if (height > 0.75f) return BiomeType::Mountains;
    if (height > 0.55f) {
        return (moisture > 0.5f) ? BiomeType::Plateaus : BiomeType::Craters;
    }
    if (height > 0.35f) {
        return (moisture > 0.5f) ? BiomeType::Plains : BiomeType::Dunes;
    }
    return (moisture > 0.4f) ? BiomeType::Valleys : BiomeType::Tundra;
}

std::string TerrainGenerator::biomeName(BiomeType biome) {
    switch (biome) {
        case BiomeType::Plains:    return "Plains";
        case BiomeType::Mountains: return "Mountains";
        case BiomeType::Valleys:   return "Valleys";
        case BiomeType::Plateaus:  return "Plateaus";
        case BiomeType::Craters:   return "Craters";
        case BiomeType::Dunes:     return "Dunes";
        case BiomeType::Tundra:    return "Tundra";
        case BiomeType::Volcanic:  return "Volcanic";
    }
    return "Unknown";
}

// ── Public API ──────────────────────────────────────────────────────

GeneratedTerrain TerrainGenerator::generate(uint64_t seed,
                                             PlanetType type,
                                             int gridSize) {
    DeterministicRNG rng(seed);

    GeneratedTerrain terrain{};
    terrain.planet_id  = seed;
    terrain.grid_size  = gridSize;
    terrain.valid      = true;

    for (int i = 0; i < 8; ++i) terrain.biome_counts[i] = 0;
    terrain.mineable_count     = 0;
    terrain.landing_zone_count = 0;

    // Derive sub-seeds for height and moisture noise layers.
    uint64_t heightSeed   = deriveSeed(seed, 0x48454947); // "HEIG"
    uint64_t moistureSeed = deriveSeed(seed, 0x4D4F4953); // "MOIS"
    uint64_t resourceSeed = deriveSeed(seed, 0x52455343); // "RESC"

    // Resource pool for mineable deposits.
    static const char* RESOURCE_POOL[] = {
        "iron", "silicon", "ice", "rare_earth", "titanium",
        "hydrogen", "helium", "water", "carbon", "uranium"
    };
    static constexpr int RESOURCE_POOL_SIZE = 10;

    // Generate regions (one per grid cell).
    for (int gy = 0; gy < gridSize; ++gy) {
        for (int gx = 0; gx < gridSize; ++gx) {
            TerrainRegion region{};
            region.grid_x = gx;
            region.grid_y = gy;

            // Each region has a 4×4 sub-grid of cells.
            static constexpr int CELLS_PER_REGION = 4;
            float heightSum = 0.0f;
            float maxH      = 0.0f;

            for (int cy = 0; cy < CELLS_PER_REGION; ++cy) {
                for (int cx = 0; cx < CELLS_PER_REGION; ++cx) {
                    TerrainCell cell{};
                    int wx = gx * CELLS_PER_REGION + cx;
                    int wy = gy * CELLS_PER_REGION + cy;

                    cell.height   = noise2D(heightSeed, wx, wy);
                    cell.moisture = noise2D(moistureSeed, wx, wy);
                    cell.biome    = classifyBiome(cell.height, cell.moisture, type);

                    // Mineable check.
                    float mineChance = noise2D(resourceSeed, wx, wy);
                    cell.mineable = (mineChance < 0.15f) && (type != PlanetType::Gas);
                    if (cell.mineable) {
                        DeterministicRNG resRng(hashCombine(resourceSeed,
                            static_cast<uint64_t>(wx * 1000 + wy)));
                        cell.resource_type = RESOURCE_POOL[resRng.range(0, RESOURCE_POOL_SIZE - 1)];
                        terrain.mineable_count++;
                    }

                    terrain.biome_counts[static_cast<int>(cell.biome)]++;

                    heightSum += cell.height;
                    if (cell.height > maxH) maxH = cell.height;

                    region.cells.push_back(cell);
                }
            }

            region.avg_height = heightSum / static_cast<float>(CELLS_PER_REGION * CELLS_PER_REGION);
            region.max_height = maxH;

            // Landing zone: avg height < 0.4 and max height < 0.5 (flat area).
            region.has_landing_zone = (region.avg_height < 0.4f) && (region.max_height < 0.5f);
            if (region.has_landing_zone) terrain.landing_zone_count++;

            terrain.regions.push_back(region);
        }
    }

    return terrain;
}

} // namespace pcg
} // namespace atlas
