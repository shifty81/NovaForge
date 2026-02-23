#ifndef EVE_PCG_PLANET_GENERATOR_H
#define EVE_PCG_PLANET_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

enum class PlanetType : uint32_t {
    Rocky, Gas, Ice, Lava, Ocean, Desert, Forest, Barren
};

struct PlanetResource {
    std::string resource_type;
    float abundance;
    float depth;
    bool requires_scan;
};

struct GeneratedPlanet {
    uint64_t planet_id;
    PlanetType type;
    float radius;
    float gravity;
    float temperature;
    bool has_atmosphere;
    bool terraformable;
    std::vector<PlanetResource> resources;
    int surface_poi_count;
    int underground_poi_count;
};

class PlanetGenerator {
public:
    GeneratedPlanet generate(uint64_t seed, PlanetType type) const;
    static float computeGravity(PlanetType type, float radius);
    static int computeResourceCount(uint64_t seed, PlanetType type);
    static bool isTerraformable(PlanetType type);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_PLANET_GENERATOR_H
