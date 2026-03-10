// Tests for: AEGIS Spawn System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/aegis_spawn_system.h"

using namespace atlas;

// ==================== AEGIS Spawn System Tests ====================

static void testAegisSpawnCreate() {
    std::cout << "\n=== AegisSpawn: Create ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    world.createEntity("sys1");
    assertTrue(sys.initialize("sys1", "jita", 1.0f), "Init succeeds");
    assertTrue(sys.getSquadCount("sys1") == 0, "No squads initially");
    assertTrue(sys.getActiveSquadCount("sys1") == 0, "No active squads");
    assertTrue(sys.getTotalDispatched("sys1") == 0, "0 dispatched");
    assertTrue(sys.getTotalKills("sys1") == 0, "0 kills");
    assertTrue(approxEqual(sys.getSecurityLevel("sys1"), 1.0f), "Security 1.0");
}

static void testAegisSpawnReport() {
    std::cout << "\n=== AegisSpawn: ReportCriminal ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "jita", 1.0f);

    assertTrue(sys.reportCriminal("sys1", "pirate_01"), "Report criminal");
    assertTrue(sys.getSquadCount("sys1") == 1, "1 squad");
    assertTrue(sys.getTotalDispatched("sys1") == 1, "1 dispatched");

    // Squad is dispatching, not yet active
    assertTrue(sys.getActiveSquadCount("sys1") == 0, "0 active (dispatching)");
}

static void testAegisSpawnResponseTime() {
    std::cout << "\n=== AegisSpawn: ResponseTime ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);

    // High-sec: fast response
    world.createEntity("highsec");
    sys.initialize("highsec", "jita", 1.0f);
    float rt_high = sys.getResponseTime("highsec");
    assertTrue(approxEqual(rt_high, 6.0f), "High-sec response ~6s");

    // Mid-sec: slower
    world.createEntity("midsec");
    sys.initialize("midsec", "rens", 0.5f);
    float rt_mid = sys.getResponseTime("midsec");
    assertTrue(rt_mid > rt_high, "Mid-sec slower than high-sec");
    assertTrue(approxEqual(rt_mid, 21.0f), "Mid-sec response ~21s");

    // Low-sec: even slower
    world.createEntity("lowsec");
    sys.initialize("lowsec", "amamake", 0.1f);
    float rt_low = sys.getResponseTime("lowsec");
    assertTrue(rt_low > rt_mid, "Low-sec slower than mid-sec");
}

static void testAegisSpawnUpdate() {
    std::cout << "\n=== AegisSpawn: Update ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "jita", 1.0f);
    sys.reportCriminal("sys1", "pirate_01");

    // Response time is 6 seconds at sec 1.0, tick through it
    sys.update(3.0f);
    assertTrue(sys.getActiveSquadCount("sys1") == 0, "Not arrived yet at 3s");

    sys.update(4.0f); // dispatching done, transitions to warping
    // After warping transition on next tick, should be engaged
    sys.update(1.0f);
    assertTrue(sys.getActiveSquadCount("sys1") == 1, "Engaged after warp");
}

static void testAegisSpawnWithdraw() {
    std::cout << "\n=== AegisSpawn: Withdraw ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "jita", 1.0f);
    sys.reportCriminal("sys1", "pirate_01");

    // Fast-forward to engaged state
    sys.update(10.0f);
    sys.update(1.0f);

    assertTrue(sys.withdrawSquad("sys1", "aegis_0"), "Withdraw succeeds");
    // After next update, withdrawn squad is removed
    sys.update(1.0f);
    assertTrue(sys.getSquadCount("sys1") == 0, "Squad removed after withdraw");
}

static void testAegisSpawnMaxSquads() {
    std::cout << "\n=== AegisSpawn: MaxSquads ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "jita", 1.0f);

    for (int i = 0; i < 5; i++) {
        assertTrue(sys.reportCriminal("sys1", "criminal_" + std::to_string(i)),
                   "Report criminal " + std::to_string(i));
    }
    assertTrue(sys.getSquadCount("sys1") == 5, "5 squads (max)");
    assertTrue(!sys.reportCriminal("sys1", "criminal_overflow"), "Overflow rejected");
}

static void testAegisSpawnSetSecurity() {
    std::cout << "\n=== AegisSpawn: SetSecurity ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "jita", 1.0f);

    assertTrue(sys.setSecurityLevel("sys1", 0.3f), "Set security");
    assertTrue(approxEqual(sys.getSecurityLevel("sys1"), 0.3f), "Security updated");
    assertTrue(sys.getResponseTime("sys1") > 6.0f, "Longer response at lower sec");
}

static void testAegisSpawnMissing() {
    std::cout << "\n=== AegisSpawn: Missing ===" << std::endl;
    ecs::World world;
    systems::AegisSpawnSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.reportCriminal("nonexistent", "c1"), "Report fails on missing");
    assertTrue(!sys.withdrawSquad("nonexistent", "s1"), "Withdraw fails on missing");
    assertTrue(!sys.setSecurityLevel("nonexistent", 1.0f), "SetSec fails on missing");
    assertTrue(sys.getSquadCount("nonexistent") == 0, "0 squads on missing");
    assertTrue(sys.getActiveSquadCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalDispatched("nonexistent") == 0, "0 dispatched on missing");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(approxEqual(sys.getSecurityLevel("nonexistent"), 0.0f), "0.0 sec on missing");
    assertTrue(approxEqual(sys.getResponseTime("nonexistent"), 0.0f), "0.0 RT on missing");
}

void run_aegis_spawn_system_tests() {
    testAegisSpawnCreate();
    testAegisSpawnReport();
    testAegisSpawnResponseTime();
    testAegisSpawnUpdate();
    testAegisSpawnWithdraw();
    testAegisSpawnMaxSquads();
    testAegisSpawnSetSecurity();
    testAegisSpawnMissing();
}
