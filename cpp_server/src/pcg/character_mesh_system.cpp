#include "pcg/character_mesh_system.h"

namespace atlas {
namespace pcg {

// ── Public API ─────────────────────────────────────────────────────

float CharacterMeshSystem::computeHeight(const CharacterSliders& sliders) {
    return 1.5f + sliders.height * (2.2f - 1.5f);
}

void CharacterMeshSystem::applyRacialTraits(GeneratedCharacter& character) {
    character.learning_rate = 1.0f;
    character.diplomacy_bonus = 0.0f;
    character.automation_bonus = 0.0f;
    character.resilience = 1.0f;

    switch (character.race) {
        case Race::TerranDescendant:
            character.learning_rate = 1.2f;
            character.diplomacy_bonus = 0.15f;
            break;
        case Race::SynthBorn:
            character.automation_bonus = 0.25f;
            character.resilience = 0.8f;
            break;
        case Race::PureAlien:
            character.resilience = 1.3f;
            break;
        case Race::HybridEvolutionary:
            character.learning_rate = 1.1f;
            character.resilience = 1.1f;
            break;
    }
}

GeneratedCharacter CharacterMeshSystem::generate(uint64_t seed, Race race, const CharacterSliders& sliders) const {
    DeterministicRNG rng(seed);

    GeneratedCharacter character{};
    character.character_id = seed;
    character.race = race;
    character.sliders = sliders;

    float variation = rng.rangeFloat(-0.02f, 0.02f);
    character.total_height = computeHeight(sliders) + variation;
    character.head_radius = 0.09f + sliders.head_shape * (0.13f - 0.09f);
    character.arm_span = character.total_height * (0.9f + sliders.limb_length * (1.1f - 0.9f));
    character.torso_height = character.total_height * (0.35f + sliders.torso_proportion * (0.45f - 0.35f));
    character.leg_height = character.total_height - character.torso_height - character.head_radius;

    applyRacialTraits(character);

    return character;
}

} // namespace pcg
} // namespace atlas
