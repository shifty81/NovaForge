// Tests for: Phase 9: Interruptible Chatter Tests, Phase 9: Timing Rules Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_chatter_system.h"

using namespace atlas;

// ==================== Phase 9: Interruptible Chatter Tests ====================

static void testChatterInterruptHighPriority() {
    std::cout << "\n=== Chatter Interrupt High Priority ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Captain speaks initially");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter->is_speaking, "Captain is_speaking after getNextChatterLine");
    assertTrue(chatter->speaking_priority == 1.0f, "Speaking priority is 1.0 (normal)");
    bool interrupted = sys.interruptChatter("cap1", 5.0f);
    assertTrue(interrupted, "Interrupt succeeds with higher priority");
    assertTrue(!chatter->is_speaking, "Captain no longer speaking after interrupt");
    assertTrue(chatter->was_interrupted, "was_interrupted flag set");
}

static void testChatterInterruptLowPriorityFails() {
    std::cout << "\n=== Chatter Interrupt Low Priority Fails ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Combat");
    sys.getNextChatterLine("cap1");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter->is_speaking, "Captain is speaking");
    bool interrupted = sys.interruptChatter("cap1", 0.5f);
    assertTrue(!interrupted, "Interrupt fails with lower priority");
    assertTrue(chatter->is_speaking, "Captain still speaking");
}

static void testChatterInterruptNotSpeaking() {
    std::cout << "\n=== Chatter Interrupt Not Speaking ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::FleetChatterState>(entity);
    bool interrupted = sys.interruptChatter("cap1", 10.0f);
    assertTrue(!interrupted, "Cannot interrupt non-speaking captain");
}


// ==================== Phase 9: Timing Rules Tests ====================

static void testChatterTimingNoOverlap() {
    std::cout << "\n=== Chatter Timing No Overlap ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* e1 = world.createEntity("cap1");
    auto* e2 = world.createEntity("cap2");
    addComp<components::CaptainPersonality>(e1);
    addComp<components::FleetChatterState>(e1);
    addComp<components::CaptainPersonality>(e2);
    addComp<components::FleetChatterState>(e2);
    sys.setActivity("cap1", "Idle");
    sys.setActivity("cap2", "Idle");

    std::string line1 = sys.getNextChatterLine("cap1");
    assertTrue(!line1.empty(), "First captain speaks");
    assertTrue(sys.isAnyoneSpeaking(), "Someone is speaking");

    std::string line2 = sys.getNextChatterLine("cap2");
    assertTrue(line2.empty(), "Second captain blocked (overlap prevention)");
}

static void testChatterTimingCooldownRange() {
    std::cout << "\n=== Chatter Timing Cooldown Range ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->sociability = 0.1f;  // very low → would try to double cooldown
    pers->optimism = 0.0f;     // low → would push to 45+
    auto* chatter = addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    sys.getNextChatterLine("cap1");
    // With clamp, cooldown should be at most 45.0
    assertTrue(chatter->chatter_cooldown >= 20.0f, "Cooldown at least 20s");
    assertTrue(chatter->chatter_cooldown <= 45.0f, "Cooldown at most 45s");
}

static void testChatterSpeakingClearedAfterCooldown() {
    std::cout << "\n=== Chatter Speaking Cleared After Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    sys.getNextChatterLine("cap1");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter->is_speaking, "Captain is speaking");
    sys.update(60.0f);  // expire cooldown
    assertTrue(!chatter->is_speaking, "is_speaking cleared after cooldown");
    assertTrue(!sys.isAnyoneSpeaking(), "No one speaking after cooldown");
}


void run_fleet_chatter_tests() {
    testChatterInterruptHighPriority();
    testChatterInterruptLowPriorityFails();
    testChatterInterruptNotSpeaking();
    testChatterTimingNoOverlap();
    testChatterTimingCooldownRange();
    testChatterSpeakingClearedAfterCooldown();
}
