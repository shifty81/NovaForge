#include "pcg/habitat_generator.h"
#include <algorithm>

namespace atlas {
namespace pcg {

// ── Public API ─────────────────────────────────────────────────────

float HabitatGenerator::computePowerDraw(HabitatModuleType type) {
    switch (type) {
        case HabitatModuleType::LivingQuarters: return 10.0f;
        case HabitatModuleType::FarmDeck:       return 15.0f;
        case HabitatModuleType::SolarArray:     return 0.0f;
        case HabitatModuleType::Hangar:         return 25.0f;
        case HabitatModuleType::MiningBay:      return 20.0f;
        case HabitatModuleType::Refinery:       return 30.0f;
        case HabitatModuleType::Storage:        return 5.0f;
        case HabitatModuleType::CommandCenter:  return 20.0f;
        case HabitatModuleType::MedBay:         return 12.0f;
        case HabitatModuleType::Workshop:       return 18.0f;
        case HabitatModuleType::Lab:            return 15.0f;
        case HabitatModuleType::Airlock:        return 3.0f;
    }
    return 0.0f;
}

float HabitatGenerator::computePowerGeneration(HabitatModuleType type) {
    if (type == HabitatModuleType::SolarArray) return 50.0f;
    return 0.0f;
}

GeneratedHabitat HabitatGenerator::generate(uint64_t seed, int target_levels) const {
    DeterministicRNG rng(seed);

    int levels = std::max(1, std::min(10, target_levels));

    GeneratedHabitat habitat{};
    habitat.habitat_id = seed;
    habitat.total_levels = levels;
    habitat.total_power_draw = 0.0f;
    habitat.total_power_generation = 0.0f;

    // Mid-level module pool
    static const HabitatModuleType MID_MODULES[] = {
        HabitatModuleType::LivingQuarters, HabitatModuleType::MiningBay,
        HabitatModuleType::Refinery, HabitatModuleType::Storage,
        HabitatModuleType::MedBay, HabitatModuleType::Workshop,
        HabitatModuleType::Lab, HabitatModuleType::Hangar,
        HabitatModuleType::Airlock
    };
    static constexpr int MID_POOL_SIZE = 9;

    for (int lvl = 0; lvl < levels; ++lvl) {
        int modulesOnLevel = rng.range(1, 3);

        for (int m = 0; m < modulesOnLevel; ++m) {
            HabitatModule mod{};
            mod.level = lvl;

            if (lvl == 0 && m == 0) {
                mod.type = HabitatModuleType::CommandCenter;
            } else if (lvl == 0) {
                mod.type = HabitatModuleType::LivingQuarters;
            } else if (lvl == levels - 1) {
                mod.type = (m == 0) ? HabitatModuleType::SolarArray : HabitatModuleType::FarmDeck;
            } else {
                mod.type = MID_MODULES[rng.range(0, MID_POOL_SIZE - 1)];
            }

            mod.power_draw = computePowerDraw(mod.type);
            mod.power_generation = computePowerGeneration(mod.type);
            mod.capacity = rng.range(2, 20);

            habitat.modules.push_back(mod);
            habitat.total_power_draw += mod.power_draw;
            habitat.total_power_generation += mod.power_generation;
        }
    }

    habitat.module_count = static_cast<int>(habitat.modules.size());
    habitat.is_self_sufficient = (habitat.total_power_generation >= habitat.total_power_draw);

    return habitat;
}

} // namespace pcg
} // namespace atlas
