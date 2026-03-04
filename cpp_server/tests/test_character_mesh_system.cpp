// Tests for: Character Mesh System Tests
#include "test_log.h"
#include "ecs/system.h"
#include "pcg/character_mesh_system.h"

using namespace atlas;

// ==================== Character Mesh System Tests ====================

static void testCharacterMeshGeneration() {
    std::cout << "\n=== Character Mesh Generation ===" << std::endl;
    pcg::CharacterMeshSystem meshSys;
    pcg::CharacterSliders sliders;
    sliders.height = 0.5f;
    auto character = meshSys.generate(12345, pcg::Race::TerranDescendant, sliders);
    assertTrue(character.total_height >= 1.5f && character.total_height <= 2.2f, "Height in valid range");
    assertTrue(character.arm_span > 0.0f, "Arm span positive");
    assertTrue(character.head_radius >= 0.09f && character.head_radius <= 0.13f, "Head radius in range");
}

static void testCharacterMeshDeterminism() {
    std::cout << "\n=== Character Mesh Determinism ===" << std::endl;
    pcg::CharacterMeshSystem meshSys;
    pcg::CharacterSliders sliders;
    auto c1 = meshSys.generate(99999, pcg::Race::SynthBorn, sliders);
    auto c2 = meshSys.generate(99999, pcg::Race::SynthBorn, sliders);
    assertTrue(approxEqual(c1.total_height, c2.total_height), "Same seed same height");
    assertTrue(approxEqual(c1.arm_span, c2.arm_span), "Same seed same arm span");
}

static void testCharacterRacialTraits() {
    std::cout << "\n=== Character Racial Traits ===" << std::endl;
    pcg::CharacterMeshSystem meshSys;
    pcg::CharacterSliders sliders;

    auto terran = meshSys.generate(1, pcg::Race::TerranDescendant, sliders);
    assertTrue(approxEqual(terran.learning_rate, 1.2f), "Terran learning rate 1.2");
    assertTrue(approxEqual(terran.diplomacy_bonus, 0.15f), "Terran diplomacy 0.15");

    auto synth = meshSys.generate(2, pcg::Race::SynthBorn, sliders);
    assertTrue(approxEqual(synth.automation_bonus, 0.25f), "SynthBorn automation 0.25");
    assertTrue(approxEqual(synth.resilience, 0.8f), "SynthBorn resilience 0.8");

    auto alien = meshSys.generate(3, pcg::Race::PureAlien, sliders);
    assertTrue(approxEqual(alien.resilience, 1.3f), "PureAlien resilience 1.3");

    auto hybrid = meshSys.generate(4, pcg::Race::HybridEvolutionary, sliders);
    assertTrue(approxEqual(hybrid.learning_rate, 1.1f), "Hybrid learning rate 1.1");
    assertTrue(approxEqual(hybrid.resilience, 1.1f), "Hybrid resilience 1.1");
}


void run_character_mesh_system_tests() {
    testCharacterMeshGeneration();
    testCharacterMeshDeterminism();
    testCharacterRacialTraits();
}
