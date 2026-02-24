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

std::string CharacterMeshSystem::bodyTypeName(BodyType bt) {
    switch (bt) {
        case BodyType::Organic:    return "Organic";
        case BodyType::Augmented:  return "Augmented";
        case BodyType::Cybernetic: return "Cybernetic";
        case BodyType::FullSynth:  return "FullSynth";
        case BodyType::MechFrame:  return "MechFrame";
    }
    return "Unknown";
}

// ── Cybernetic limb generation ─────────────────────────────────────

static const char* LIMB_SLOTS[] = {
    "left_arm", "right_arm", "left_leg", "right_leg",
    "torso_core", "spine",
};
static constexpr int LIMB_SLOT_COUNT = 6;

void CharacterMeshSystem::applyCybernetics(DeterministicRNG& rng,
                                            GeneratedCharacter& character,
                                            BodyType body) {
    character.cyber_percentage        = 0.0f;
    character.strength_multiplier     = 1.0f;
    character.speed_multiplier        = 1.0f;
    character.integrated_weapon_count = 0;

    int replacedCount = 0;

    switch (body) {
        case BodyType::Organic:
            // No cybernetics.
            return;

        case BodyType::Augmented:
            // 1-2 limbs replaced.
            replacedCount = rng.range(1, 2);
            character.cyber_percentage = rng.rangeFloat(0.10f, 0.35f);
            break;

        case BodyType::Cybernetic:
            // 3-4 limbs replaced, heavier cyber.
            replacedCount = rng.range(3, 4);
            character.cyber_percentage = rng.rangeFloat(0.50f, 0.80f);
            break;

        case BodyType::FullSynth:
            // All limbs are synthetic.
            replacedCount = LIMB_SLOT_COUNT;
            character.cyber_percentage = 1.0f;
            break;

        case BodyType::MechFrame:
            // All limbs + industrial frame.
            replacedCount = LIMB_SLOT_COUNT;
            character.cyber_percentage = 1.0f;
            // MechFrames are larger and slower but much stronger.
            character.total_height *= rng.rangeFloat(1.2f, 1.6f);
            character.strength_multiplier = rng.rangeFloat(2.0f, 3.5f);
            character.speed_multiplier    = rng.rangeFloat(0.5f, 0.8f);
            break;
    }

    if (replacedCount > LIMB_SLOT_COUNT) replacedCount = LIMB_SLOT_COUNT;

    // Randomly pick which limbs to replace (shuffle first N from pool).
    int indices[LIMB_SLOT_COUNT];
    for (int i = 0; i < LIMB_SLOT_COUNT; ++i) indices[i] = i;
    for (int i = LIMB_SLOT_COUNT - 1; i > 0; --i) {
        int j = rng.range(0, i);
        int tmp = indices[i]; indices[i] = indices[j]; indices[j] = tmp;
    }

    for (int i = 0; i < replacedCount; ++i) {
        CyberLimb limb;
        limb.slot      = LIMB_SLOTS[indices[i]];
        limb.strength  = rng.rangeFloat(1.1f, 2.0f);
        limb.speed     = rng.rangeFloat(0.9f, 1.5f);
        limb.is_weapon = rng.chance(0.20f); // 20% chance of weapon limb
        if (limb.is_weapon) character.integrated_weapon_count++;
        character.cyberLimbs.push_back(limb);
    }

    // Aggregate multipliers from limbs.
    float strSum = 0.0f, spdSum = 0.0f;
    for (const auto& l : character.cyberLimbs) {
        strSum += l.strength;
        spdSum += l.speed;
    }
    if (!character.cyberLimbs.empty()) {
        float n = static_cast<float>(character.cyberLimbs.size());
        character.strength_multiplier *= (strSum / n);
        character.speed_multiplier    *= (spdSum / n);
    }
}

// ── Original generate (backward compatible — always Organic) ──────

GeneratedCharacter CharacterMeshSystem::generate(uint64_t seed, Race race, const CharacterSliders& sliders) const {
    return generate(seed, race, BodyType::Organic, sliders);
}

// ── Full generate with body type ───────────────────────────────────

GeneratedCharacter CharacterMeshSystem::generate(uint64_t seed, Race race,
                                                  BodyType body,
                                                  const CharacterSliders& sliders) const {
    DeterministicRNG rng(seed);

    GeneratedCharacter character{};
    character.character_id = seed;
    character.race = race;
    character.bodyType = body;
    character.sliders = sliders;

    float variation = rng.rangeFloat(-0.02f, 0.02f);
    character.total_height = computeHeight(sliders) + variation;
    character.head_radius = 0.09f + sliders.head_shape * (0.13f - 0.09f);
    character.arm_span = character.total_height * (0.9f + sliders.limb_length * (1.1f - 0.9f));
    character.torso_height = character.total_height * (0.35f + sliders.torso_proportion * (0.45f - 0.35f));
    character.leg_height = character.total_height - character.torso_height - character.head_radius;

    applyRacialTraits(character);
    applyCybernetics(rng, character, body);

    return character;
}

} // namespace pcg
} // namespace atlas
