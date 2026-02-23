#ifndef EVE_PCG_HABITAT_GENERATOR_H
#define EVE_PCG_HABITAT_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

enum class HabitatModuleType : uint32_t {
    LivingQuarters, FarmDeck, SolarArray, Hangar,
    MiningBay, Refinery, Storage, CommandCenter,
    MedBay, Workshop, Lab, Airlock
};

struct HabitatModule {
    HabitatModuleType type;
    int level;
    float power_draw;
    float power_generation;
    int capacity;
};

struct GeneratedHabitat {
    uint64_t habitat_id;
    int total_levels;
    int module_count;
    std::vector<HabitatModule> modules;
    float total_power_draw;
    float total_power_generation;
    bool is_self_sufficient;
};

class HabitatGenerator {
public:
    GeneratedHabitat generate(uint64_t seed, int target_levels) const;
    static float computePowerDraw(HabitatModuleType type);
    static float computePowerGeneration(HabitatModuleType type);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_HABITAT_GENERATOR_H
