#ifndef EVE_PCG_CHARACTER_MESH_SYSTEM_H
#define EVE_PCG_CHARACTER_MESH_SYSTEM_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>

namespace atlas {
namespace pcg {

enum class Race : uint32_t {
    TerranDescendant,
    SynthBorn,
    PureAlien,
    HybridEvolutionary
};

struct CharacterSliders {
    float height = 0.5f;
    float limb_length = 0.5f;
    float torso_proportion = 0.5f;
    float head_shape = 0.5f;
    float build = 0.5f;
    bool is_male = true;
};

struct GeneratedCharacter {
    uint64_t character_id;
    Race race;
    CharacterSliders sliders;
    float total_height;
    float arm_span;
    float torso_height;
    float leg_height;
    float head_radius;
    float learning_rate;
    float diplomacy_bonus;
    float automation_bonus;
    float resilience;
};

class CharacterMeshSystem {
public:
    GeneratedCharacter generate(uint64_t seed, Race race, const CharacterSliders& sliders) const;
    static void applyRacialTraits(GeneratedCharacter& character);
    static float computeHeight(const CharacterSliders& sliders);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_CHARACTER_MESH_SYSTEM_H
