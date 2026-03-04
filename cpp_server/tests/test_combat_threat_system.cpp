// Tests for: Combat Threat System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/combat_threat_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Combat Threat System Tests ====================

static void testCombatThreatDamage() {
    std::cout << "\n=== Combat Threat: Damage Increases Threat ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_1");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.0f;

    ctSys.recordCombatDamage("system_1", 500.0f);
    assertTrue(ctSys.getPendingDamage("system_1") == 500.0f, "Pending damage recorded");

    ctSys.update(1.0f);
    assertTrue(state->threat_level > 0.0f, "Threat increased after damage");
    float expected = 500.0f * ctSys.damage_threat_factor;
    assertTrue(approxEqual(state->threat_level, expected), "Threat matches expected value");
}

static void testCombatThreatDestruction() {
    std::cout << "\n=== Combat Threat: Destruction Spikes Threat ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_2");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.1f;

    ctSys.recordShipDestruction("system_2");
    ctSys.recordShipDestruction("system_2");
    assertTrue(ctSys.getPendingDestructions("system_2") == 2, "Two destructions pending");

    ctSys.update(1.0f);
    float expected = 0.1f + 2 * ctSys.destruction_threat_spike;
    assertTrue(approxEqual(state->threat_level, expected), "Threat spiked by 2 destructions");
}

static void testCombatThreatClamped() {
    std::cout << "\n=== Combat Threat: Threat Clamped at Max ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_3");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.95f;

    ctSys.recordShipDestruction("system_3");
    ctSys.recordShipDestruction("system_3");
    ctSys.recordShipDestruction("system_3");
    ctSys.update(1.0f);
    assertTrue(state->threat_level <= ctSys.max_threat, "Threat clamped at max");
}

static void testCombatThreatClearedAfterUpdate() {
    std::cout << "\n=== Combat Threat: Pending Cleared After Update ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_4");
    addComp<components::SimStarSystemState>(sys);

    ctSys.recordCombatDamage("system_4", 100.0f);
    ctSys.update(1.0f);
    assertTrue(ctSys.getPendingDamage("system_4") == 0.0f, "Pending damage cleared");
    assertTrue(ctSys.getPendingDestructions("system_4") == 0, "Pending destructions cleared");
}

static void testCombatThreatNoPendingForUnknown() {
    std::cout << "\n=== Combat Threat: No Pending for Unknown System ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    assertTrue(ctSys.getPendingDamage("unknown") == 0.0f, "No pending damage for unknown");
    assertTrue(ctSys.getPendingDestructions("unknown") == 0, "No pending destructions for unknown");
}


void run_combat_threat_system_tests() {
    testCombatThreatDamage();
    testCombatThreatDestruction();
    testCombatThreatClamped();
    testCombatThreatClearedAfterUpdate();
    testCombatThreatNoPendingForUnknown();
}
