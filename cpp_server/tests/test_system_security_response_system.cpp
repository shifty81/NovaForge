// Tests for: System Security Response System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/system_security_response_system.h"

using namespace atlas;

// ==================== System Security Response System Tests ====================

static void testSecurityResponseCreate() {
    std::cout << "\n=== SecurityResponse: Create ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    assertTrue(sys.initialize("system_jita", "highsec"), "Init highsec");
    assertTrue(sys.getOffenceCount("system_jita") == 0, "0 offences");
    assertTrue(sys.getResponseLevel("system_jita") == 0.0f, "0 response level");
    assertTrue(sys.getState("system_jita") == "idle", "State idle");
    assertTrue(sys.getResponseShipsDispatched("system_jita") == 0, "0 ships dispatched");
    assertTrue(sys.getTotalResponses("system_jita") == 0, "0 total responses");
}

static void testSecurityResponseReportOffence() {
    std::cout << "\n=== SecurityResponse: ReportOffence ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    sys.initialize("system_jita", "highsec");

    assertTrue(sys.reportOffence("system_jita", "pirate_01", "assault", 3.0f), "Report offence");
    assertTrue(sys.getOffenceCount("system_jita") == 1, "1 offence");
    // severity 3 * 5 = 15, exceeds highsec alert_threshold of 10
    assertTrue(sys.getResponseLevel("system_jita") == 15.0f, "Response level 15");

    // Update tick to trigger state transition
    sys.update(0.0f);
    assertTrue(sys.getState("system_jita") == "alerted", "State alerted");
}

static void testSecurityResponseEscalation() {
    std::cout << "\n=== SecurityResponse: Escalation ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    sys.initialize("system_jita", "highsec");  // respond_threshold = 30

    // Two offences: 4.0 * 5 = 20 + 4.0 * 5 = 20 = 40 → above respond_threshold (30)
    sys.reportOffence("system_jita", "pirate_01", "assault", 4.0f);
    sys.reportOffence("system_jita", "pirate_01", "theft", 4.0f);
    sys.update(0.0f);
    assertTrue(sys.getState("system_jita") == "responding", "State responding");

    // Dispatch response
    assertTrue(sys.dispatchResponse("system_jita", 3), "Dispatch 3 ships");
    assertTrue(sys.getResponseShipsDispatched("system_jita") == 3, "3 ships dispatched");
    assertTrue(sys.getTotalResponses("system_jita") == 1, "1 response");
    assertTrue(sys.getState("system_jita") == "engaged", "State engaged");
}

static void testSecurityResponseDecay() {
    std::cout << "\n=== SecurityResponse: Decay ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    sys.initialize("system_jita", "highsec");  // decay_rate = 1.0/s

    sys.reportOffence("system_jita", "pirate_01", "smuggling", 2.0f);  // → level 10
    assertTrue(sys.getResponseLevel("system_jita") == 10.0f, "Level 10");

    // Decay over 10 seconds: 10 - 1.0 * 10 = 0
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getResponseLevel("system_jita"), 0.0f), "Level decayed to 0");
    assertTrue(sys.getState("system_jita") == "idle", "Back to idle after decay");
}

static void testSecurityResponseLowsec() {
    std::cout << "\n=== SecurityResponse: Lowsec ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_rancer");
    sys.initialize("system_rancer", "lowsec");  // alert=30, respond=60, decay=3

    // Offence severity 5 → 25 < lowsec alert_threshold of 30
    sys.reportOffence("system_rancer", "pirate_01", "assault", 5.0f);
    sys.update(0.0f);
    assertTrue(sys.getState("system_rancer") == "idle", "Still idle in lowsec");

    // Second offence → 25+25=50, exceeds 30 but not 60
    sys.reportOffence("system_rancer", "pirate_01", "assault", 5.0f);
    sys.update(0.0f);
    assertTrue(sys.getState("system_rancer") == "alerted", "Alerted in lowsec");
}

static void testSecurityResponseClear() {
    std::cout << "\n=== SecurityResponse: Clear ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    sys.initialize("system_jita", "highsec");

    sys.reportOffence("system_jita", "pirate_01", "assault", 8.0f);
    sys.reportOffence("system_jita", "pirate_02", "theft", 3.0f);
    assertTrue(sys.getOffenceCount("system_jita") == 2, "2 offences before clear");

    assertTrue(sys.clearOffences("system_jita"), "Clear succeeds");
    assertTrue(sys.getOffenceCount("system_jita") == 0, "0 offences after clear");
    assertTrue(sys.getResponseLevel("system_jita") == 0.0f, "Level 0 after clear");
    assertTrue(sys.getState("system_jita") == "idle", "Idle after clear");
}

static void testSecurityResponseDispatchRequiresResponding() {
    std::cout << "\n=== SecurityResponse: DispatchRequiresResponding ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    sys.initialize("system_jita", "highsec");

    // Can't dispatch when idle
    assertTrue(!sys.dispatchResponse("system_jita", 2), "Dispatch fails when idle");

    // Get to alerted (not responding)
    sys.reportOffence("system_jita", "pirate_01", "smuggling", 3.0f);
    sys.update(0.0f);
    assertTrue(sys.getState("system_jita") == "alerted", "State alerted");
    assertTrue(!sys.dispatchResponse("system_jita", 2), "Dispatch fails when only alerted");
}

static void testSecurityResponseMaxOffences() {
    std::cout << "\n=== SecurityResponse: MaxOffences ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    world.createEntity("system_jita");
    sys.initialize("system_jita", "highsec");

    // Fill to max (50 offences)
    for (int i = 0; i < 55; i++) {
        sys.reportOffence("system_jita", "pirate_" + std::to_string(i), "assault", 0.1f);
    }
    assertTrue(sys.getOffenceCount("system_jita") == 50, "Capped at 50 offences");
}

static void testSecurityResponseMissing() {
    std::cout << "\n=== SecurityResponse: Missing ===" << std::endl;
    ecs::World world;
    systems::SystemSecurityResponseSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "highsec"), "Init fails on missing");
    assertTrue(!sys.reportOffence("nonexistent", "p1", "assault", 5.0f), "Report fails on missing");
    assertTrue(!sys.dispatchResponse("nonexistent", 2), "Dispatch fails on missing");
    assertTrue(!sys.clearOffences("nonexistent"), "Clear fails on missing");
    assertTrue(sys.getOffenceCount("nonexistent") == 0, "0 offences on missing");
    assertTrue(sys.getResponseLevel("nonexistent") == 0.0f, "0 level on missing");
    assertTrue(sys.getState("nonexistent") == "unknown", "Unknown state on missing");
    assertTrue(sys.getResponseShipsDispatched("nonexistent") == 0, "0 ships on missing");
    assertTrue(sys.getTotalResponses("nonexistent") == 0, "0 responses on missing");
}

void run_system_security_response_system_tests() {
    testSecurityResponseCreate();
    testSecurityResponseReportOffence();
    testSecurityResponseEscalation();
    testSecurityResponseDecay();
    testSecurityResponseLowsec();
    testSecurityResponseClear();
    testSecurityResponseDispatchRequiresResponding();
    testSecurityResponseMaxOffences();
    testSecurityResponseMissing();
}
