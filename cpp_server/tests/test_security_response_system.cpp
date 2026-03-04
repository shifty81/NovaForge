// Tests for: Security Response System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/security_response_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Security Response System Tests ====================

static void testSecurityResponseDefaults() {
    std::cout << "\n=== Security Response: Defaults ===" << std::endl;
    ecs::World world;

    auto* sys = world.createEntity("sec_sys");
    auto* resp = addComp<components::SecurityResponseState>(sys);

    assertTrue(!resp->responding, "Not responding by default");
    assertTrue(resp->response_timer == 0.0f, "Timer starts at 0");
    assertTrue(resp->response_strength == 0.0f, "Strength starts at 0");
}

static void testSecurityResponseTriggered() {
    std::cout << "\n=== Security Response: Triggered in High-Sec ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys = world.createEntity("sec_highsec");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.9f;
    state->threat_level = 0.5f;  // above threshold

    // Run enough ticks to pass the delay
    for (int i = 0; i < 100; i++) {
        secSys.update(1.0f);
    }

    assertTrue(resp->responding, "Security response active in high-sec with threat");
    assertTrue(secSys.isResponding("sec_highsec"), "Query confirms responding");
}

static void testSecurityResponseNoTriggerLowSec() {
    std::cout << "\n=== Security Response: No Response in Low-Sec ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys = world.createEntity("sec_lowsec");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.2f;  // below min level
    state->threat_level = 0.9f;    // high threat

    for (int i = 0; i < 100; i++) {
        secSys.update(1.0f);
    }

    assertTrue(!resp->responding, "No response in low-sec");
}

static void testSecurityResponseNoTriggerLowThreat() {
    std::cout << "\n=== Security Response: No Response on Low Threat ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys = world.createEntity("sec_calm");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.8f;
    state->threat_level = 0.1f;  // below threshold

    secSys.update(10.0f);
    assertTrue(!resp->responding, "No response on low threat");
}

static void testSecurityResponseDelayScaling() {
    std::cout << "\n=== Security Response: Faster in Higher Sec ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    // High-sec system — should respond faster
    auto* sys1 = world.createEntity("sec_fast");
    auto* resp1 = addComp<components::SecurityResponseState>(sys1);
    auto* st1   = addComp<components::SimStarSystemState>(sys1);
    st1->security_level = 1.0f;
    st1->threat_level = 0.5f;

    secSys.update(0.1f);  // start timer
    float timer_high = resp1->response_timer;

    ecs::World world2;
    systems::SecurityResponseSystem secSys2(&world2);
    auto* sys2 = world2.createEntity("sec_slow");
    auto* resp2 = addComp<components::SecurityResponseState>(sys2);
    auto* st2   = addComp<components::SimStarSystemState>(sys2);
    st2->security_level = 0.5f;
    st2->threat_level = 0.5f;

    secSys2.update(0.1f);
    float timer_mid = resp2->response_timer;

    assertTrue(timer_high < timer_mid, "High-sec response delay shorter than mid-sec");
}

static void testSecurityResponseDuration() {
    std::cout << "\n=== Security Response: Response Expires ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);
    secSys.response_duration = 10.0f;

    auto* sys = world.createEntity("sec_expire");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.9f;
    state->threat_level = 0.0f;  // no threat initially

    // Manually activate the response to test duration expiry
    resp->responding = true;
    resp->response_timer = 10.0f;
    assertTrue(resp->responding, "Response is active");

    // Tick past the response duration
    for (int i = 0; i < 15; i++) secSys.update(1.0f);
    assertTrue(!resp->responding, "Response expired after duration");
}

static void testSecurityResponseRespondingSystems() {
    std::cout << "\n=== Security Response: List Responding Systems ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys1 = world.createEntity("sec_a");
    auto* resp1 = addComp<components::SecurityResponseState>(sys1);
    addComp<components::SimStarSystemState>(sys1);
    resp1->responding = true;  // manually set for query test

    auto* sys2 = world.createEntity("sec_b");
    auto* resp2 = addComp<components::SecurityResponseState>(sys2);
    addComp<components::SimStarSystemState>(sys2);
    resp2->responding = false;

    auto list = secSys.getRespondingSystems();
    assertTrue(list.size() == 1, "One responding system");
    assertTrue(list[0] == "sec_a", "Correct responding system");
}


void run_security_response_system_tests() {
    testSecurityResponseDefaults();
    testSecurityResponseTriggered();
    testSecurityResponseNoTriggerLowSec();
    testSecurityResponseNoTriggerLowThreat();
    testSecurityResponseDelayScaling();
    testSecurityResponseDuration();
    testSecurityResponseRespondingSystems();
}
