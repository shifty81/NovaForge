// Tests for: Damage Resistance Profile System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/damage_resistance_profile_system.h"

using namespace atlas;

// ==================== Damage Resistance Profile System Tests ====================

static void testDamageResistanceProfileCreate() {
    std::cout << "\n=== DamageResistanceProfile: Create ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getHardenerCount("ship1") == 0, "0 hardeners");
    assertTrue(sys.getActiveHardenerCount("ship1") == 0, "0 active hardeners");
    assertTrue(sys.getTotalDamageMitigated("ship1") == 0.0f, "0 mitigated");
    assertTrue(sys.getEffectiveResistance("ship1", "em") == 0.0f, "0 EM res");
    assertTrue(sys.getEffectiveResistance("ship1", "thermal") == 0.0f, "0 thermal res");
    assertTrue(sys.getEffectiveResistance("ship1", "kinetic") == 0.0f, "0 kinetic res");
    assertTrue(sys.getEffectiveResistance("ship1", "explosive") == 0.0f, "0 explosive res");
}

static void testDamageResistanceProfileSetBase() {
    std::cout << "\n=== DamageResistanceProfile: SetBase ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.setBaseResistance("ship1", "em", 0.2f), "Set EM base");
    assertTrue(sys.setBaseResistance("ship1", "thermal", 0.4f), "Set thermal base");
    assertTrue(sys.setBaseResistance("ship1", "kinetic", 0.6f), "Set kinetic base");
    assertTrue(sys.setBaseResistance("ship1", "explosive", 0.1f), "Set explosive base");

    // Verify effective resistance matches base when no hardeners active
    float em = sys.getEffectiveResistance("ship1", "em");
    assertTrue(em > 0.19f && em < 0.21f, "EM res ~0.2");
    float th = sys.getEffectiveResistance("ship1", "thermal");
    assertTrue(th > 0.39f && th < 0.41f, "Thermal res ~0.4");

    // Invalid type rejected
    assertTrue(!sys.setBaseResistance("ship1", "plasma", 0.5f), "Invalid type rejected");

    // Clamped to 0.85 max
    assertTrue(sys.setBaseResistance("ship1", "em", 0.95f), "Over-cap accepted (clamped)");
    float em2 = sys.getEffectiveResistance("ship1", "em");
    assertTrue(em2 <= 0.85f, "EM clamped to 0.85");
}

static void testDamageResistanceProfileAddHardener() {
    std::cout << "\n=== DamageResistanceProfile: AddHardener ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.addHardener("ship1", "h1", "em", 0.3f), "Add EM hardener");
    assertTrue(sys.addHardener("ship1", "h2", "thermal", 0.25f), "Add thermal hardener");
    assertTrue(sys.getHardenerCount("ship1") == 2, "2 hardeners");

    // Duplicate rejected
    assertTrue(!sys.addHardener("ship1", "h1", "kinetic", 0.2f), "Dup ID rejected");

    // Invalid type rejected
    assertTrue(!sys.addHardener("ship1", "h3", "plasma", 0.2f), "Invalid type rejected");
}

static void testDamageResistanceProfileActivateDeactivate() {
    std::cout << "\n=== DamageResistanceProfile: ActivateDeactivate ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setBaseResistance("ship1", "em", 0.2f);
    sys.addHardener("ship1", "h1", "em", 0.3f);

    // Inactive hardener doesn't affect resistance
    float before = sys.getEffectiveResistance("ship1", "em");
    assertTrue(before > 0.19f && before < 0.21f, "Inactive: EM still ~0.2");

    assertTrue(sys.activateHardener("ship1", "h1"), "Activate h1");
    assertTrue(sys.getActiveHardenerCount("ship1") == 1, "1 active");

    // Active hardener boosts resistance
    float after = sys.getEffectiveResistance("ship1", "em");
    assertTrue(after > before, "Active: EM increased");

    // Can't activate twice
    assertTrue(!sys.activateHardener("ship1", "h1"), "Already active rejected");

    assertTrue(sys.deactivateHardener("ship1", "h1"), "Deactivate h1");
    assertTrue(sys.getActiveHardenerCount("ship1") == 0, "0 active after deactivate");

    // Can't deactivate twice
    assertTrue(!sys.deactivateHardener("ship1", "h1"), "Already inactive rejected");
}

static void testDamageResistanceProfileApply() {
    std::cout << "\n=== DamageResistanceProfile: Apply ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setBaseResistance("ship1", "kinetic", 0.5f);

    float result = sys.applyResistance("ship1", "kinetic", 100.0f);
    assertTrue(result > 49.0f && result < 51.0f, "50% kinetic res → ~50 damage");

    float mitigated = sys.getTotalDamageMitigated("ship1");
    assertTrue(mitigated > 49.0f && mitigated < 51.0f, "~50 mitigated");

    // Zero resistance passes full damage
    float full = sys.applyResistance("ship1", "em", 100.0f);
    assertTrue(full > 99.0f && full < 101.0f, "0% EM res → ~100 damage");
}

static void testDamageResistanceProfileStackingPenalty() {
    std::cout << "\n=== DamageResistanceProfile: StackingPenalty ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Add 3 thermal hardeners and activate all
    sys.addHardener("ship1", "h1", "thermal", 0.3f);
    sys.addHardener("ship1", "h2", "thermal", 0.3f);
    sys.addHardener("ship1", "h3", "thermal", 0.3f);
    sys.activateHardener("ship1", "h1");
    sys.activateHardener("ship1", "h2");
    sys.activateHardener("ship1", "h3");

    float res = sys.getEffectiveResistance("ship1", "thermal");
    // With stacking penalty, 3×0.3 should be less than 0.9 (without stacking)
    assertTrue(res < 0.85f, "Stacking penalty: res < 85% cap");
    assertTrue(res > 0.3f, "Multiple hardeners boost beyond single");

    // Still capped at 85%
    assertTrue(res <= 0.85f, "Hard cap at 85%");
}

static void testDamageResistanceProfileRemoveHardener() {
    std::cout << "\n=== DamageResistanceProfile: RemoveHardener ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addHardener("ship1", "h1", "em", 0.3f);
    assertTrue(sys.removeHardener("ship1", "h1"), "Remove h1");
    assertTrue(sys.getHardenerCount("ship1") == 0, "0 hardeners after remove");

    // Can't remove again
    assertTrue(!sys.removeHardener("ship1", "h1"), "Can't remove twice");
    assertTrue(!sys.removeHardener("ship1", "nonexistent"), "Can't remove missing");
}

static void testDamageResistanceProfileMaxHardeners() {
    std::cout << "\n=== DamageResistanceProfile: MaxHardeners ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Fill to max (8)
    for (int i = 0; i < 8; i++) {
        assertTrue(sys.addHardener("ship1", "h_" + std::to_string(i), "em", 0.1f),
                   "Add h_" + std::to_string(i));
    }
    assertTrue(sys.getHardenerCount("ship1") == 8, "8 hardeners at max");
    assertTrue(!sys.addHardener("ship1", "h_8", "em", 0.1f), "9th rejected");
}

static void testDamageResistanceProfileUpdate() {
    std::cout << "\n=== DamageResistanceProfile: Update ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addHardener("ship1", "h1", "em", 0.3f);
    sys.activateHardener("ship1", "h1");

    // Tick should consume charge
    sys.update(1.0f);
    // No crash, system ticks correctly
    assertTrue(sys.getActiveHardenerCount("ship1") == 1, "Still 1 active after tick");
}

static void testDamageResistanceProfileMissing() {
    std::cout << "\n=== DamageResistanceProfile: Missing ===" << std::endl;
    ecs::World world;
    systems::DamageResistanceProfileSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.setBaseResistance("nonexistent", "em", 0.5f), "SetBase fails on missing");
    assertTrue(!sys.addHardener("nonexistent", "h1", "em", 0.3f), "AddHardener fails on missing");
    assertTrue(!sys.removeHardener("nonexistent", "h1"), "RemoveHardener fails on missing");
    assertTrue(!sys.activateHardener("nonexistent", "h1"), "Activate fails on missing");
    assertTrue(!sys.deactivateHardener("nonexistent", "h1"), "Deactivate fails on missing");
    assertTrue(sys.getEffectiveResistance("nonexistent", "em") == 0.0f, "0 res on missing");
    assertTrue(sys.getHardenerCount("nonexistent") == 0, "0 hardeners on missing");
    assertTrue(sys.getActiveHardenerCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalDamageMitigated("nonexistent") == 0.0f, "0 mitigated on missing");
}

void run_damage_resistance_profile_system_tests() {
    testDamageResistanceProfileCreate();
    testDamageResistanceProfileSetBase();
    testDamageResistanceProfileAddHardener();
    testDamageResistanceProfileActivateDeactivate();
    testDamageResistanceProfileApply();
    testDamageResistanceProfileStackingPenalty();
    testDamageResistanceProfileRemoveHardener();
    testDamageResistanceProfileMaxHardeners();
    testDamageResistanceProfileUpdate();
    testDamageResistanceProfileMissing();
}
