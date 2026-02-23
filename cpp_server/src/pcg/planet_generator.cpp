#include "pcg/planet_generator.h"

namespace atlas {
namespace pcg {

static const char* RESOURCE_POOL[] = {
    "iron", "silicon", "ice", "rare_earth", "titanium",
    "hydrogen", "helium", "water", "carbon", "uranium"
};
static constexpr int RESOURCE_POOL_SIZE = 10;

// ── Public API ─────────────────────────────────────────────────────

float PlanetGenerator::computeGravity(PlanetType type, float radius) {
    switch (type) {
        case PlanetType::Rocky:   return radius / 6371.0f;
        case PlanetType::Gas:     return 2.5f;
        case PlanetType::Ice:     return 0.3f * radius / 6371.0f;
        case PlanetType::Lava:    return radius / 6371.0f * 1.2f;
        case PlanetType::Ocean:   return radius / 6371.0f * 0.9f;
        case PlanetType::Desert:  return radius / 6371.0f * 0.95f;
        case PlanetType::Forest:  return radius / 6371.0f;
        case PlanetType::Barren:  return radius / 6371.0f * 0.8f;
    }
    return 1.0f;
}

int PlanetGenerator::computeResourceCount(uint64_t seed, PlanetType type) {
    DeterministicRNG rng(seed);
    switch (type) {
        case PlanetType::Rocky:   return rng.range(3, 6);
        case PlanetType::Gas:     return rng.range(1, 3);
        case PlanetType::Ice:     return rng.range(2, 4);
        case PlanetType::Lava:    return rng.range(2, 5);
        case PlanetType::Ocean:   return rng.range(2, 4);
        case PlanetType::Desert:  return rng.range(2, 5);
        case PlanetType::Forest:  return rng.range(3, 5);
        case PlanetType::Barren:  return rng.range(1, 3);
    }
    return 2;
}

bool PlanetGenerator::isTerraformable(PlanetType type) {
    switch (type) {
        case PlanetType::Rocky:
        case PlanetType::Desert:
        case PlanetType::Barren:
            return true;
        default:
            return false;
    }
}

GeneratedPlanet PlanetGenerator::generate(uint64_t seed, PlanetType type) const {
    DeterministicRNG rng(seed);

    GeneratedPlanet planet{};
    planet.planet_id = seed;
    planet.type = type;

    // Radius by type
    switch (type) {
        case PlanetType::Rocky:   planet.radius = rng.rangeFloat(2000.0f, 8000.0f); break;
        case PlanetType::Gas:     planet.radius = rng.rangeFloat(20000.0f, 70000.0f); break;
        case PlanetType::Ice:     planet.radius = rng.rangeFloat(3000.0f, 12000.0f); break;
        case PlanetType::Lava:    planet.radius = rng.rangeFloat(2000.0f, 6000.0f); break;
        case PlanetType::Ocean:   planet.radius = rng.rangeFloat(4000.0f, 10000.0f); break;
        case PlanetType::Desert:  planet.radius = rng.rangeFloat(3000.0f, 9000.0f); break;
        case PlanetType::Forest:  planet.radius = rng.rangeFloat(4000.0f, 8000.0f); break;
        case PlanetType::Barren:  planet.radius = rng.rangeFloat(1500.0f, 7000.0f); break;
    }

    planet.gravity = computeGravity(type, planet.radius);
    planet.temperature = rng.rangeFloat(50.0f, 800.0f);
    planet.has_atmosphere = (type != PlanetType::Barren && type != PlanetType::Lava) ? rng.chance(0.7f) : false;
    planet.terraformable = isTerraformable(type);

    // Resources
    int resCount = computeResourceCount(seed ^ 0xABCD, type);
    for (int i = 0; i < resCount; ++i) {
        PlanetResource res{};
        res.resource_type = RESOURCE_POOL[rng.range(0, RESOURCE_POOL_SIZE - 1)];
        res.abundance = rng.rangeFloat(0.1f, 1.0f);
        res.depth = rng.rangeFloat(0.0f, 1.0f);
        res.requires_scan = rng.chance(0.4f);
        planet.resources.push_back(res);
    }

    // POIs
    switch (type) {
        case PlanetType::Rocky:   planet.surface_poi_count = rng.range(3, 8); break;
        case PlanetType::Gas:     planet.surface_poi_count = 0; break;
        case PlanetType::Ice:     planet.surface_poi_count = rng.range(2, 5); break;
        case PlanetType::Lava:    planet.surface_poi_count = rng.range(1, 4); break;
        case PlanetType::Ocean:   planet.surface_poi_count = rng.range(2, 6); break;
        case PlanetType::Desert:  planet.surface_poi_count = rng.range(3, 7); break;
        case PlanetType::Forest:  planet.surface_poi_count = rng.range(4, 8); break;
        case PlanetType::Barren:  planet.surface_poi_count = rng.range(2, 4); break;
    }

    switch (type) {
        case PlanetType::Rocky:
        case PlanetType::Desert:
            planet.underground_poi_count = rng.range(1, 5);
            break;
        case PlanetType::Gas:
        case PlanetType::Ocean:
            planet.underground_poi_count = 0;
            break;
        default:
            planet.underground_poi_count = rng.range(0, 3);
            break;
    }

    return planet;
}

} // namespace pcg
} // namespace atlas
