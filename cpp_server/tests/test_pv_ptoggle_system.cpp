// Tests for: PvPToggleSystem Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/pvp_toggle_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== PvPToggleSystem Tests ====================

static void testPvPCreate() {
    std::cout << "\n=== PvPToggle: Create ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.createPvPState("player1"), "Create PvP state succeeds");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP disabled by default");
    assertTrue(sys.getSafetyLevel("player1") == "HighSec", "Default safety is HighSec");
    assertTrue(approxEqual(sys.getSecurityStatus("player1"), 5.0f), "Default security status 5.0");
}

static void testPvPEnable() {
    std::cout << "\n=== PvPToggle: Enable ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    assertTrue(!sys.enablePvP("player1"), "Cannot enable PvP in HighSec");
    sys.setSafetyLevel("player1", "LowSec");
    assertTrue(sys.enablePvP("player1"), "Enable PvP in LowSec succeeds");
    assertTrue(sys.isPvPEnabled("player1"), "PvP is now enabled");
}

static void testPvPDisable() {
    std::cout << "\n=== PvPToggle: Disable ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    sys.setSafetyLevel("player1", "LowSec");
    sys.enablePvP("player1");
    assertTrue(sys.disablePvP("player1"), "Disable PvP in LowSec succeeds");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP is now disabled");
}

static void testPvPSafetyLevel() {
    std::cout << "\n=== PvPToggle: SafetyLevel ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    assertTrue(sys.setSafetyLevel("player1", "LowSec"), "Set LowSec succeeds");
    assertTrue(sys.getSafetyLevel("player1") == "LowSec", "Safety is LowSec");
    assertTrue(sys.setSafetyLevel("player1", "NullSec"), "Set NullSec succeeds");
    assertTrue(sys.isPvPEnabled("player1"), "NullSec forces PvP enabled");
    assertTrue(!sys.disablePvP("player1"), "Cannot disable PvP in NullSec");
    assertTrue(sys.setSafetyLevel("player1", "Wormhole"), "Set Wormhole succeeds");
    assertTrue(sys.isPvPEnabled("player1"), "Wormhole forces PvP enabled");
    assertTrue(!sys.setSafetyLevel("player1", "InvalidZone"), "Invalid zone rejected");
}

static void testPvPCanEngage() {
    std::cout << "\n=== PvPToggle: CanEngage ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("attacker");
    world.createEntity("defender");
    sys.createPvPState("attacker");
    sys.createPvPState("defender");
    assertTrue(!sys.canEngage("attacker", "defender"), "Cannot engage in HighSec");
    sys.setSafetyLevel("attacker", "LowSec");
    sys.setSafetyLevel("defender", "LowSec");
    sys.enablePvP("attacker");
    assertTrue(!sys.canEngage("attacker", "defender"), "Cannot engage if defender unflagged");
    sys.enablePvP("defender");
    assertTrue(sys.canEngage("attacker", "defender"), "Both flagged in LowSec can engage");
}

static void testPvPNullSecEngage() {
    std::cout << "\n=== PvPToggle: NullSecEngage ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("attacker");
    world.createEntity("defender");
    sys.createPvPState("attacker");
    sys.createPvPState("defender");
    sys.setSafetyLevel("attacker", "NullSec");
    sys.setSafetyLevel("defender", "NullSec");
    assertTrue(sys.canEngage("attacker", "defender"), "NullSec: always engageable");
}

static void testPvPRecordEngagement() {
    std::cout << "\n=== PvPToggle: RecordEngagement ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("a");
    world.createEntity("d");
    sys.createPvPState("a");
    sys.createPvPState("d");
    assertTrue(sys.recordEngagement("a", "d"), "Record engagement succeeds");
    assertTrue(approxEqual(sys.getAggressionTimer("a"), 300.0f), "Attacker aggression timer set");
    assertTrue(approxEqual(sys.getAggressionTimer("d"), 300.0f), "Defender aggression timer set");
}

static void testPvPAggressionCooldown() {
    std::cout << "\n=== PvPToggle: AggressionCooldown ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    sys.setSafetyLevel("player1", "LowSec");
    sys.enablePvP("player1");
    world.createEntity("target");
    sys.createPvPState("target");
    sys.recordEngagement("player1", "target");
    assertTrue(!sys.disablePvP("player1"), "Cannot disable PvP with active aggression");
    sys.update(200.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("player1"), 100.0f), "Timer decreased by 200s");
    sys.update(100.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("player1"), 0.0f), "Timer reached 0");
    assertTrue(sys.disablePvP("player1"), "Can disable PvP after timer expires");
}

static void testPvPRecordKill() {
    std::cout << "\n=== PvPToggle: RecordKill ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("killer");
    sys.createPvPState("killer");
    assertTrue(sys.recordKill("killer"), "Record kill succeeds");
    assertTrue(sys.getKillCount("killer") == 1, "Kill count is 1");
    assertTrue(approxEqual(sys.getBounty("killer"), 1000.0f), "Bounty is 1000");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), 4.5f), "Security status decreased to 4.5");
    sys.recordKill("killer");
    sys.recordKill("killer");
    assertTrue(sys.getKillCount("killer") == 3, "Kill count is 3");
    assertTrue(approxEqual(sys.getBounty("killer"), 3000.0f), "Bounty is 3000");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), 3.5f), "Security status decreased to 3.5");
}

static void testPvPMissing() {
    std::cout << "\n=== PvPToggle: Missing ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    assertTrue(!sys.createPvPState("nonexistent"), "Create fails on missing entity");
    assertTrue(!sys.enablePvP("nonexistent"), "Enable fails on missing");
    assertTrue(!sys.disablePvP("nonexistent"), "Disable fails on missing");
    assertTrue(!sys.isPvPEnabled("nonexistent"), "isPvPEnabled false on missing");
    assertTrue(sys.getSafetyLevel("nonexistent") == "", "Empty safety on missing");
    assertTrue(sys.getKillCount("nonexistent") == 0, "0 kills on missing");
    assertTrue(approxEqual(sys.getSecurityStatus("nonexistent"), 0.0f), "0 security on missing");
    assertTrue(approxEqual(sys.getBounty("nonexistent"), 0.0f), "0 bounty on missing");
    assertTrue(approxEqual(sys.getAggressionTimer("nonexistent"), 0.0f), "0 timer on missing");
    assertTrue(!sys.recordKill("nonexistent"), "Record kill fails on missing");
}


void run_pv_ptoggle_system_tests() {
    testPvPCreate();
    testPvPEnable();
    testPvPDisable();
    testPvPSafetyLevel();
    testPvPCanEngage();
    testPvPNullSecEngage();
    testPvPRecordEngagement();
    testPvPAggressionCooldown();
    testPvPRecordKill();
    testPvPMissing();
}
