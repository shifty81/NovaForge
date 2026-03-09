// Tests for: PvPToggleSystem Tests
#include "test_log.h"
#include "components/game_components.h"
#include "systems/pvp_toggle_system.h"

using namespace atlas;

// ==================== PvPToggleSystem Tests ====================

static void testPvPCreateAndDefaults() {
    std::cout << "\n=== PvP Create And Defaults ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");

    bool created = sys.createPvPState("player1");
    assertTrue(created, "createPvPState returns true");

    assertTrue(!sys.isPvPEnabled("player1"), "Default pvp_enabled is false");
    assertTrue(sys.getSafetyLevel("player1") == "HighSec", "Default safety level is HighSec");
    assertTrue(approxEqual(sys.getSecurityStatus("player1"), 5.0f), "Default security_status is 5.0");
    assertTrue(sys.getKillCount("player1") == 0, "Default kill_count is 0");
    assertTrue(approxEqual(sys.getAggressionTimer("player1"), 0.0f), "Default aggression_timer is 0.0");
    assertTrue(approxEqual(sys.getBounty("player1"), 0.0f), "Default bounty is 0.0");
}

static void testPvPSafetyLevels() {
    std::cout << "\n=== PvP Safety Levels ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");

    // HighSec: force-disables PvP
    sys.setSafetyLevel("player1", "HighSec");
    assertTrue(sys.getSafetyLevel("player1") == "HighSec", "Safety level set to HighSec");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP disabled in HighSec");

    // LowSec: PvP not auto-set, stays as-is
    sys.setSafetyLevel("player1", "LowSec");
    assertTrue(sys.getSafetyLevel("player1") == "LowSec", "Safety level set to LowSec");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP not auto-enabled in LowSec");

    // NullSec: auto-enables PvP
    sys.setSafetyLevel("player1", "NullSec");
    assertTrue(sys.getSafetyLevel("player1") == "NullSec", "Safety level set to NullSec");
    assertTrue(sys.isPvPEnabled("player1"), "PvP auto-enabled in NullSec");

    // Wormhole: auto-enables PvP
    sys.setSafetyLevel("player1", "Wormhole");
    assertTrue(sys.getSafetyLevel("player1") == "Wormhole", "Safety level set to Wormhole");
    assertTrue(sys.isPvPEnabled("player1"), "PvP auto-enabled in Wormhole");

    // Invalid safety level rejected
    bool result = sys.setSafetyLevel("player1", "SuperSafe");
    assertTrue(!result, "Invalid safety level rejected");
    assertTrue(sys.getSafetyLevel("player1") == "Wormhole", "Safety level unchanged after invalid set");
}

static void testPvPEnableDisable() {
    std::cout << "\n=== PvP Enable Disable ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");

    // Cannot enable PvP in HighSec
    sys.setSafetyLevel("player1", "HighSec");
    bool enabled = sys.enablePvP("player1");
    assertTrue(!enabled, "enablePvP fails in HighSec");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP remains disabled in HighSec");

    // Can enable PvP in LowSec
    sys.setSafetyLevel("player1", "LowSec");
    enabled = sys.enablePvP("player1");
    assertTrue(enabled, "enablePvP succeeds in LowSec");
    assertTrue(sys.isPvPEnabled("player1"), "PvP enabled in LowSec");

    // Can disable PvP when no aggression timer
    bool disabled = sys.disablePvP("player1");
    assertTrue(disabled, "disablePvP succeeds with no aggression timer");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP disabled after disablePvP");

    // Cannot disable PvP with active aggression timer
    world.createEntity("player2");
    sys.createPvPState("player2");
    sys.setSafetyLevel("player1", "LowSec");
    sys.setSafetyLevel("player2", "LowSec");
    sys.enablePvP("player1");
    sys.enablePvP("player2");
    sys.recordEngagement("player1", "player2");
    disabled = sys.disablePvP("player1");
    assertTrue(!disabled, "disablePvP fails with active aggression timer");
    assertTrue(sys.isPvPEnabled("player1"), "PvP remains enabled with aggression timer");

    // Cannot disable PvP in NullSec
    world.createEntity("player3");
    sys.createPvPState("player3");
    sys.setSafetyLevel("player3", "NullSec");
    disabled = sys.disablePvP("player3");
    assertTrue(!disabled, "disablePvP fails in NullSec");

    // Cannot disable PvP in Wormhole
    world.createEntity("player4");
    sys.createPvPState("player4");
    sys.setSafetyLevel("player4", "Wormhole");
    disabled = sys.disablePvP("player4");
    assertTrue(!disabled, "disablePvP fails in Wormhole");
}

static void testPvPEngagement() {
    std::cout << "\n=== PvP Engagement ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);

    world.createEntity("attacker");
    world.createEntity("defender");
    sys.createPvPState("attacker");
    sys.createPvPState("defender");
    sys.setSafetyLevel("attacker", "LowSec");
    sys.setSafetyLevel("defender", "LowSec");
    sys.enablePvP("attacker");
    sys.enablePvP("defender");

    bool canFight = sys.canEngage("attacker", "defender");
    assertTrue(canFight, "Both PvP-enabled players can engage");

    bool engaged = sys.recordEngagement("attacker", "defender");
    assertTrue(engaged, "recordEngagement succeeds");
    assertTrue(approxEqual(sys.getAggressionTimer("attacker"), 300.0f), "Attacker aggression timer set to 300");
    assertTrue(approxEqual(sys.getAggressionTimer("defender"), 300.0f), "Defender aggression timer set to 300");

    // canEngage fails when defender has PvP disabled
    world.createEntity("pacifist");
    sys.createPvPState("pacifist");
    sys.setSafetyLevel("pacifist", "HighSec");
    bool canAttack = sys.canEngage("attacker", "pacifist");
    assertTrue(!canAttack, "Cannot engage player with PvP disabled");
}

static void testPvPKillTracking() {
    std::cout << "\n=== PvP Kill Tracking ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("killer");
    sys.createPvPState("killer");

    // Record first kill
    sys.recordKill("killer");
    assertTrue(sys.getKillCount("killer") == 1, "Kill count is 1 after first kill");
    assertTrue(approxEqual(sys.getBounty("killer"), 1000.0f), "Bounty is 1000 after first kill");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), 4.5f), "Security status is 4.5 after first kill");

    // Record second kill
    sys.recordKill("killer");
    assertTrue(sys.getKillCount("killer") == 2, "Kill count is 2 after second kill");
    assertTrue(approxEqual(sys.getBounty("killer"), 2000.0f), "Bounty is 2000 after second kill");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), 4.0f), "Security status is 4.0 after second kill");

    // Security status clamps at -10
    for (int i = 0; i < 30; ++i) {
        sys.recordKill("killer");
    }
    assertTrue(sys.getSecurityStatus("killer") >= -10.0f, "Security status clamped at -10.0");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), -10.0f), "Security status exactly -10.0");
    assertTrue(sys.getKillCount("killer") == 32, "Kill count is 32 after all kills");
    assertTrue(approxEqual(sys.getBounty("killer"), 32000.0f), "Bounty accumulated correctly");
}

static void testPvPAggressionDecay() {
    std::cout << "\n=== PvP Aggression Decay ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);

    world.createEntity("attacker");
    world.createEntity("defender");
    sys.createPvPState("attacker");
    sys.createPvPState("defender");
    sys.setSafetyLevel("attacker", "LowSec");
    sys.setSafetyLevel("defender", "LowSec");
    sys.enablePvP("attacker");
    sys.enablePvP("defender");
    sys.recordEngagement("attacker", "defender");

    assertTrue(approxEqual(sys.getAggressionTimer("attacker"), 300.0f), "Initial aggression timer is 300");

    sys.update(100.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("attacker"), 200.0f), "Timer decayed by 100 to 200");

    sys.update(150.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("attacker"), 50.0f), "Timer decayed by 150 to 50");

    sys.update(100.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("attacker"), 0.0f), "Timer clamped at 0");
}

static void testPvPMissingEntity() {
    std::cout << "\n=== PvP Missing Entity ===" << std::endl;

    ecs::World world;
    systems::PvPToggleSystem sys(&world);

    assertTrue(!sys.enablePvP("ghost"), "enablePvP fails for nonexistent entity");
    assertTrue(!sys.disablePvP("ghost"), "disablePvP fails for nonexistent entity");
    assertTrue(!sys.setSafetyLevel("ghost", "LowSec"), "setSafetyLevel fails for nonexistent entity");
    assertTrue(!sys.isPvPEnabled("ghost"), "isPvPEnabled returns false for nonexistent entity");
    assertTrue(sys.getSafetyLevel("ghost") == "", "getSafetyLevel returns empty for nonexistent entity");
    assertTrue(sys.getKillCount("ghost") == 0, "getKillCount returns 0 for nonexistent entity");
    assertTrue(approxEqual(sys.getSecurityStatus("ghost"), 0.0f), "getSecurityStatus returns 0 for nonexistent entity");
    assertTrue(approxEqual(sys.getAggressionTimer("ghost"), 0.0f), "getAggressionTimer returns 0 for nonexistent entity");
    assertTrue(approxEqual(sys.getBounty("ghost"), 0.0f), "getBounty returns 0 for nonexistent entity");
    assertTrue(!sys.recordKill("ghost"), "recordKill fails for nonexistent entity");
    assertTrue(!sys.canEngage("ghost", "ghost2"), "canEngage fails for nonexistent entities");
    assertTrue(!sys.recordEngagement("ghost", "ghost2"), "recordEngagement fails for nonexistent entities");
}

void run_pvp_toggle_system_tests() {
    testPvPCreateAndDefaults();
    testPvPSafetyLevels();
    testPvPEnableDisable();
    testPvPEngagement();
    testPvPKillTracking();
    testPvPAggressionDecay();
    testPvPMissingEntity();
}
