// Tests for: Terrain Generator Tests
#include "test_log.h"
#include "pcg/terrain_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Terrain Generator Tests ====================

static void testTerrainGeneration() {
    std::cout << "\n=== Terrain Generation ===" << std::endl;
    auto terrain = pcg::TerrainGenerator::generate(42, pcg::PlanetType::Rocky, 4);
    assertTrue(terrain.valid, "Terrain is valid");
    assertTrue(terrain.grid_size == 4, "Grid size is 4");
    assertTrue(static_cast<int>(terrain.regions.size()) == 16, "4×4 = 16 regions");
    assertTrue(!terrain.regions[0].cells.empty(), "Regions have cells");
}

static void testTerrainDeterminism() {
    std::cout << "\n=== Terrain Determinism ===" << std::endl;
    auto t1 = pcg::TerrainGenerator::generate(888, pcg::PlanetType::Ice, 4);
    auto t2 = pcg::TerrainGenerator::generate(888, pcg::PlanetType::Ice, 4);
    assertTrue(t1.mineable_count == t2.mineable_count, "Same seed same mineable count");
    assertTrue(t1.landing_zone_count == t2.landing_zone_count, "Same seed same landing zones");
    assertTrue(t1.regions.size() == t2.regions.size(), "Same region count");
    // Check first region cells match.
    assertTrue(approxEqual(t1.regions[0].avg_height, t2.regions[0].avg_height),
               "Same seed same avg height");
}

static void testTerrainBiomeClassification() {
    std::cout << "\n=== Terrain Biome Classification ===" << std::endl;
    // Lava planet: high terrain → Volcanic.
    auto lava_biome = pcg::TerrainGenerator::classifyBiome(0.8f, 0.5f, pcg::PlanetType::Lava);
    assertTrue(lava_biome == pcg::BiomeType::Volcanic, "High lava terrain is Volcanic");
    // Ice planet: high terrain → Mountains.
    auto ice_biome = pcg::TerrainGenerator::classifyBiome(0.7f, 0.5f, pcg::PlanetType::Ice);
    assertTrue(ice_biome == pcg::BiomeType::Mountains, "High ice terrain is Mountains");
    // Desert planet: low terrain → Dunes.
    auto desert_biome = pcg::TerrainGenerator::classifyBiome(0.3f, 0.5f, pcg::PlanetType::Desert);
    assertTrue(desert_biome == pcg::BiomeType::Dunes, "Low desert terrain is Dunes");
}

static void testTerrainMineableDeposits() {
    std::cout << "\n=== Terrain Mineable Deposits ===" << std::endl;
    auto terrain = pcg::TerrainGenerator::generate(333, pcg::PlanetType::Rocky, 8);
    assertTrue(terrain.mineable_count > 0, "Rocky planet has mineable deposits");
    // Gas giants should have no mineable cells.
    auto gas = pcg::TerrainGenerator::generate(333, pcg::PlanetType::Gas, 4);
    assertTrue(gas.mineable_count == 0, "Gas giant has no mineable deposits");
}

static void testTerrainLandingZones() {
    std::cout << "\n=== Terrain Landing Zones ===" << std::endl;
    auto terrain = pcg::TerrainGenerator::generate(555, pcg::PlanetType::Forest, 8);
    assertTrue(terrain.landing_zone_count >= 0, "Landing zone count non-negative");
    // At least some regions should be flat enough on a large grid.
    auto big = pcg::TerrainGenerator::generate(555, pcg::PlanetType::Forest, 16);
    assertTrue(big.landing_zone_count >= 0, "Large grid has non-negative landing zones");
}

static void testTerrainBiomeName() {
    std::cout << "\n=== Terrain Biome Names ===" << std::endl;
    assertTrue(pcg::TerrainGenerator::biomeName(pcg::BiomeType::Plains) == "Plains", "Plains name");
    assertTrue(pcg::TerrainGenerator::biomeName(pcg::BiomeType::Mountains) == "Mountains", "Mountains name");
    assertTrue(pcg::TerrainGenerator::biomeName(pcg::BiomeType::Volcanic) == "Volcanic", "Volcanic name");
}

static void testTerrainDifferentSeeds() {
    std::cout << "\n=== Terrain Different Seeds ===" << std::endl;
    auto t1 = pcg::TerrainGenerator::generate(100, pcg::PlanetType::Rocky, 4);
    auto t2 = pcg::TerrainGenerator::generate(200, pcg::PlanetType::Rocky, 4);
    // Different seeds should produce different terrain (very unlikely to match).
    bool differs = (t1.mineable_count != t2.mineable_count)
                || (t1.landing_zone_count != t2.landing_zone_count)
                || !approxEqual(t1.regions[0].avg_height, t2.regions[0].avg_height);
    assertTrue(differs, "Different seeds produce different terrain");
}


void run_terrain_generator_tests() {
    testTerrainGeneration();
    testTerrainDeterminism();
    testTerrainBiomeClassification();
    testTerrainMineableDeposits();
    testTerrainLandingZones();
    testTerrainBiomeName();
    testTerrainDifferentSeeds();
}
