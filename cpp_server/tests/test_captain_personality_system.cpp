// Tests for: CaptainPersonalitySystem Tests, Extended Captain Personality Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/captain_personality_system.h"

using namespace atlas;

// ==================== CaptainPersonalitySystem Tests ====================

static void testCaptainPersonalityAssign() {
    std::cout << "\n=== Captain Personality Assign ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Solari");
    float agg = sys.getPersonalityTrait("cap1", "aggression");
    float soc = sys.getPersonalityTrait("cap1", "sociability");
    float opt = sys.getPersonalityTrait("cap1", "optimism");
    float pro = sys.getPersonalityTrait("cap1", "professionalism");
    assertTrue(agg >= 0.0f && agg <= 1.0f, "Aggression in valid range");
    assertTrue(soc >= 0.0f && soc <= 1.0f, "Sociability in valid range");
    assertTrue(opt >= 0.0f && opt <= 1.0f, "Optimism in valid range");
    assertTrue(pro >= 0.0f && pro <= 1.0f, "Professionalism in valid range");
}

static void testCaptainPersonalityFactionTraits() {
    std::cout << "\n=== Captain Personality Faction Traits ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Keldari_Captain", "Keldari");
    float agg = sys.getPersonalityTrait("cap1", "aggression");
    assertTrue(agg > 0.5f, "Keldari captain has high aggression");
}

static void testCaptainPersonalitySetTrait() {
    std::cout << "\n=== Captain Personality Set Trait ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Solari");
    sys.setPersonalityTrait("cap1", "aggression", 0.9f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "aggression"), 0.9f),
               "Set trait reads back correctly");
}

static void testCaptainPersonalityGetFaction() {
    std::cout << "\n=== Captain Personality Get Faction ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Veyren");
    assertTrue(sys.getCaptainFaction("cap1") == "Veyren", "Faction returned correctly");
}

static void testCaptainPersonalityDeterministic() {
    std::cout << "\n=== Captain Personality Deterministic ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Aurelian");
    float agg1 = sys.getPersonalityTrait("cap1", "aggression");
    float soc1 = sys.getPersonalityTrait("cap1", "sociability");
    // Assign again - should get same result (deterministic)
    sys.assignPersonality("cap1", "TestCaptain", "Aurelian");
    float agg2 = sys.getPersonalityTrait("cap1", "aggression");
    float soc2 = sys.getPersonalityTrait("cap1", "sociability");
    assertTrue(approxEqual(agg1, agg2), "Aggression is deterministic");
    assertTrue(approxEqual(soc1, soc2), "Sociability is deterministic");
}


// ==================== Extended Captain Personality Tests ====================

static void testCaptainPersonalityNewTraitsAssigned() {
    std::cout << "\n=== Captain Personality: New Traits Assigned ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Keldari");

    float loyalty = sys.getPersonalityTrait("cap1", "loyalty");
    float paranoia = sys.getPersonalityTrait("cap1", "paranoia");
    float ambition = sys.getPersonalityTrait("cap1", "ambition");
    float adaptability = sys.getPersonalityTrait("cap1", "adaptability");

    assertTrue(loyalty >= 0.0f && loyalty <= 1.0f, "Loyalty in valid range");
    assertTrue(paranoia >= 0.0f && paranoia <= 1.0f, "Paranoia in valid range");
    assertTrue(ambition >= 0.0f && ambition <= 1.0f, "Ambition in valid range");
    assertTrue(adaptability >= 0.0f && adaptability <= 1.0f, "Adaptability in valid range");
}

static void testCaptainPersonalityKeldariHighParanoia() {
    std::cout << "\n=== Captain Personality: Keldari High Paranoia ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "KeldariCaptain", "Keldari");
    float paranoia = sys.getPersonalityTrait("cap1", "paranoia");
    assertTrue(paranoia > 0.5f, "Keldari captain has high paranoia");
}

static void testCaptainPersonalitySolariHighLoyalty() {
    std::cout << "\n=== Captain Personality: Solari High Loyalty ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "SolariCaptain", "Solari");
    float loyalty = sys.getPersonalityTrait("cap1", "loyalty");
    assertTrue(loyalty > 0.5f, "Solari captain has high loyalty");
}

static void testCaptainPersonalitySetNewTrait() {
    std::cout << "\n=== Captain Personality: Set New Trait ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Test", "Veyren");
    sys.setPersonalityTrait("cap1", "loyalty", 0.95f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "loyalty"), 0.95f),
               "Loyalty set correctly");
    sys.setPersonalityTrait("cap1", "paranoia", 0.1f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "paranoia"), 0.1f),
               "Paranoia set correctly");
}

static void testCaptainPersonalityNewTraitsDeterministic() {
    std::cout << "\n=== Captain Personality: New Traits Deterministic ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Test", "Aurelian");
    float loy1 = sys.getPersonalityTrait("cap1", "loyalty");
    float par1 = sys.getPersonalityTrait("cap1", "paranoia");
    sys.assignPersonality("cap1", "Test", "Aurelian");
    float loy2 = sys.getPersonalityTrait("cap1", "loyalty");
    float par2 = sys.getPersonalityTrait("cap1", "paranoia");
    assertTrue(approxEqual(loy1, loy2), "Loyalty is deterministic");
    assertTrue(approxEqual(par1, par2), "Paranoia is deterministic");
}


void run_captain_personality_system_tests() {
    testCaptainPersonalityAssign();
    testCaptainPersonalityFactionTraits();
    testCaptainPersonalitySetTrait();
    testCaptainPersonalityGetFaction();
    testCaptainPersonalityDeterministic();
    testCaptainPersonalityNewTraitsAssigned();
    testCaptainPersonalityKeldariHighParanoia();
    testCaptainPersonalitySolariHighLoyalty();
    testCaptainPersonalitySetNewTrait();
    testCaptainPersonalityNewTraitsDeterministic();
}
