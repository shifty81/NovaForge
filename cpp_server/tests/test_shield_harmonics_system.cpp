// Tests for: ShieldHarmonicsSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/shield_harmonics_system.h"

using namespace atlas;

// ==================== ShieldHarmonicsSystem Tests ====================

static void testShieldHarmonicsCreate() {
    std::cout << "\n=== ShieldHarmonics: Create ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 50.0f), "Init at frequency 50 succeeds");
    assertTrue(approxEqual(sys.getFrequency("ship1"), 50.0f), "Frequency 50");
    assertTrue(sys.getProfileCount("ship1") == 0, "Zero profiles initially");
    assertTrue(sys.getTotalRetunings("ship1") == 0, "Zero retunings initially");
    assertTrue(approxEqual(sys.getResonanceStrength("ship1"), 1.0f), "Full resonance at init");
}

static void testShieldHarmonicsInvalidInit() {
    std::cout << "\n=== ShieldHarmonics: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    assertTrue(!sys.initialize("missing", 50.0f), "Missing entity fails");
    world.createEntity("ship1");
    assertTrue(!sys.initialize("ship1", -10.0f), "Negative frequency fails");
    assertTrue(!sys.initialize("ship1", 101.0f), "Frequency > 100 fails");
}

static void testShieldHarmonicsAddProfile() {
    std::cout << "\n=== ShieldHarmonics: AddProfile ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(sys.addProfile("ship1", "em", 0.2f), "Add EM profile");
    assertTrue(sys.addProfile("ship1", "thermal", 0.3f), "Add thermal profile");
    assertTrue(sys.addProfile("ship1", "kinetic", 0.4f), "Add kinetic profile");
    assertTrue(sys.addProfile("ship1", "explosive", 0.1f), "Add explosive profile");
    assertTrue(sys.getProfileCount("ship1") == 4, "4 profiles");

    // Duplicate rejected
    assertTrue(!sys.addProfile("ship1", "em", 0.5f), "Duplicate EM rejected");

    // Max profiles reached
    assertTrue(!sys.addProfile("ship1", "radiation", 0.1f), "5th profile rejected (max 4)");
}

static void testShieldHarmonicsInvalidProfile() {
    std::cout << "\n=== ShieldHarmonics: InvalidProfile ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(!sys.addProfile("ship1", "", 0.2f), "Empty damage type rejected");
    assertTrue(!sys.addProfile("ship1", "em", -0.1f), "Negative resistance rejected");
    assertTrue(!sys.addProfile("ship1", "em", 1.1f), "Resistance > 1.0 rejected");
    assertTrue(!sys.addProfile("nonexistent", "em", 0.2f), "Missing entity rejected");
}

static void testShieldHarmonicsRemoveProfile() {
    std::cout << "\n=== ShieldHarmonics: RemoveProfile ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    sys.addProfile("ship1", "em", 0.2f);
    sys.addProfile("ship1", "thermal", 0.3f);

    assertTrue(sys.removeProfile("ship1", "em"), "Remove EM succeeds");
    assertTrue(sys.getProfileCount("ship1") == 1, "1 profile remaining");
    assertTrue(!sys.removeProfile("ship1", "em"), "Double remove fails");
    assertTrue(!sys.removeProfile("ship1", "kinetic"), "Remove nonexistent fails");
}

static void testShieldHarmonicsTuneFrequency() {
    std::cout << "\n=== ShieldHarmonics: TuneFrequency ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(sys.tuneFrequency("ship1", 80.0f), "Tune to 80 succeeds");
    assertTrue(sys.getTotalRetunings("ship1") == 1, "1 retuning");

    assertTrue(sys.tuneFrequency("ship1", 20.0f), "Tune to 20 succeeds");
    assertTrue(sys.getTotalRetunings("ship1") == 2, "2 retunings");

    // Invalid tuning
    assertTrue(!sys.tuneFrequency("ship1", -5.0f), "Negative frequency rejected");
    assertTrue(!sys.tuneFrequency("ship1", 105.0f), "Frequency > 100 rejected");
    assertTrue(!sys.tuneFrequency("nonexistent", 50.0f), "Missing entity rejected");
}

static void testShieldHarmonicsResonance() {
    std::cout << "\n=== ShieldHarmonics: Resonance ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    sys.addProfile("ship1", "em", 0.2f);

    // At init, frequency == optimal → resonance = 1.0
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getResonanceStrength("ship1"), 1.0f), "Full resonance when aligned");

    // Tune away — update enough to move frequency
    sys.tuneFrequency("ship1", 100.0f);
    // Before update, frequency is still 50
    sys.update(10.0f); // 10s at 5/s speed = 50 units, should reach 100
    assertTrue(approxEqual(sys.getResonanceStrength("ship1"), 1.0f), "Resonance ~1.0 at target");
}

static void testShieldHarmonicsEffectiveResistance() {
    std::cout << "\n=== ShieldHarmonics: EffectiveResistance ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);
    sys.addProfile("ship1", "em", 0.2f);
    sys.addProfile("ship1", "thermal", 0.3f);

    // At resonance 1.0, bonus = max_bonus (0.3)
    sys.update(0.1f);
    float emRes = sys.getEffectiveResistance("ship1", "em");
    assertTrue(emRes > 0.49f && emRes < 0.51f, "EM effective ~0.5 (0.2 + 0.3 bonus)");

    float thermRes = sys.getEffectiveResistance("ship1", "thermal");
    assertTrue(thermRes > 0.59f && thermRes < 0.61f, "Thermal effective ~0.6 (0.3 + 0.3 bonus)");

    // Unknown type returns 0
    assertTrue(approxEqual(sys.getEffectiveResistance("ship1", "kinetic"), 0.0f),
               "Unknown type returns 0");
}

static void testShieldHarmonicsSetTuningSpeed() {
    std::cout << "\n=== ShieldHarmonics: SetTuningSpeed ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(sys.setTuningSpeed("ship1", 10.0f), "Set speed to 10");
    assertTrue(approxEqual(sys.getTuningSpeed("ship1"), 10.0f), "Speed is 10");

    assertTrue(!sys.setTuningSpeed("ship1", 0.0f), "Zero speed rejected");
    assertTrue(!sys.setTuningSpeed("ship1", -5.0f), "Negative speed rejected");
    assertTrue(!sys.setTuningSpeed("nonexistent", 10.0f), "Missing entity rejected");
}

static void testShieldHarmonicsSetMaxBonus() {
    std::cout << "\n=== ShieldHarmonics: SetMaxBonus ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    assertTrue(sys.setMaxBonus("ship1", 0.5f), "Set max bonus to 0.5");
    assertTrue(approxEqual(sys.getMaxBonus("ship1"), 0.5f), "Max bonus is 0.5");

    assertTrue(!sys.setMaxBonus("ship1", -0.1f), "Negative bonus rejected");
    assertTrue(!sys.setMaxBonus("ship1", 1.1f), "Bonus > 1.0 rejected");
    assertTrue(!sys.setMaxBonus("nonexistent", 0.5f), "Missing entity rejected");
}

static void testShieldHarmonicsUpdate() {
    std::cout << "\n=== ShieldHarmonics: Update ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 50.0f);

    sys.update(1.0f);
    // Just verifies update doesn't crash
    assertTrue(true, "Update tick OK");
}

static void testShieldHarmonicsMissing() {
    std::cout << "\n=== ShieldHarmonics: Missing ===" << std::endl;
    ecs::World world;
    systems::ShieldHarmonicsSystem sys(&world);
    assertTrue(approxEqual(sys.getFrequency("x"), 0.0f), "Default frequency on missing");
    assertTrue(approxEqual(sys.getResonanceStrength("x"), 0.0f), "Default resonance on missing");
    assertTrue(sys.getProfileCount("x") == 0, "Default profiles on missing");
    assertTrue(sys.getTotalRetunings("x") == 0, "Default retunings on missing");
    assertTrue(approxEqual(sys.getTuningSpeed("x"), 0.0f), "Default speed on missing");
    assertTrue(approxEqual(sys.getMaxBonus("x"), 0.0f), "Default bonus on missing");
    assertTrue(approxEqual(sys.getEffectiveResistance("x", "em"), 0.0f), "Default resistance on missing");
}

void run_shield_harmonics_system_tests() {
    testShieldHarmonicsCreate();
    testShieldHarmonicsInvalidInit();
    testShieldHarmonicsAddProfile();
    testShieldHarmonicsInvalidProfile();
    testShieldHarmonicsRemoveProfile();
    testShieldHarmonicsTuneFrequency();
    testShieldHarmonicsResonance();
    testShieldHarmonicsEffectiveResistance();
    testShieldHarmonicsSetTuningSpeed();
    testShieldHarmonicsSetMaxBonus();
    testShieldHarmonicsUpdate();
    testShieldHarmonicsMissing();
}
