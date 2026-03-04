// Tests for: FleetChatterSystem Tests, Contextual Chatter Tests
#include "test_log.h"
#include "components/crew_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/captain_personality_system.h"
#include "systems/fleet_chatter_system.h"

using namespace atlas;

// ==================== FleetChatterSystem Tests ====================

static void testFleetChatterSetActivity() {
    std::cout << "\n=== Fleet Chatter Set Activity ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    sys.setActivity("cap1", "Mining");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter != nullptr, "FleetChatterState component created");
    assertTrue(chatter->current_activity == "Mining", "Activity set to Mining");
}

static void testFleetChatterGetLine() {
    std::cout << "\n=== Fleet Chatter Get Line ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    addComp<components::FleetMorale>(entity);
    sys.setActivity("cap1", "Mining");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Chatter line is non-empty");
}

static void testFleetChatterCooldown() {
    std::cout << "\n=== Fleet Chatter Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    sys.getNextChatterLine("cap1");
    std::string line2 = sys.getNextChatterLine("cap1");
    assertTrue(line2.empty(), "Second line empty due to cooldown");
}

static void testFleetChatterLinesSpoken() {
    std::cout << "\n=== Fleet Chatter Lines Spoken ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Combat");
    sys.getNextChatterLine("cap1");
    assertTrue(sys.getTotalLinesSpoken("cap1") == 1, "Total lines spoken is 1");
}

static void testFleetChatterCooldownExpires() {
    std::cout << "\n=== Fleet Chatter Cooldown Expires ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Warp");
    sys.getNextChatterLine("cap1");
    assertTrue(sys.isOnCooldown("cap1"), "On cooldown after speaking");
    sys.update(60.0f);
    assertTrue(!sys.isOnCooldown("cap1"), "Cooldown expired after 60s");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Can speak again after cooldown expires");
}


// ==================== Contextual Chatter Tests ====================

static void testContextualChatterReturnsLine() {
    std::cout << "\n=== Contextual Chatter: Returns Line ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);
    systems::CaptainPersonalitySystem personalitySys(&world);

    auto* entity = world.createEntity("cap1");
    personalitySys.assignPersonality("cap1", "TestCaptain", "Keldari");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Combat");

    std::string line = chatterSys.getContextualLine("cap1");
    assertTrue(!line.empty(), "Contextual chatter returns a line");
}

static void testContextualChatterRespectsCooldown() {
    std::cout << "\n=== Contextual Chatter: Respects Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);
    systems::CaptainPersonalitySystem personalitySys(&world);

    auto* entity = world.createEntity("cap1");
    personalitySys.assignPersonality("cap1", "TestCaptain", "Solari");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Mining");

    chatterSys.getContextualLine("cap1");
    std::string second = chatterSys.getContextualLine("cap1");
    assertTrue(second.empty(), "Contextual chatter on cooldown returns empty");
}

static void testContextualChatterFallbackWithoutPersonality() {
    std::cout << "\n=== Contextual Chatter: Fallback Without Personality ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);

    auto* entity = world.createEntity("cap1");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Idle");

    std::string line = chatterSys.getContextualLine("cap1");
    assertTrue(!line.empty(), "Falls back to generic pool without personality");
}


void run_fleet_chatter_system_tests() {
    testFleetChatterSetActivity();
    testFleetChatterGetLine();
    testFleetChatterCooldown();
    testFleetChatterLinesSpoken();
    testFleetChatterCooldownExpires();
    testContextualChatterReturnsLine();
    testContextualChatterRespectsCooldown();
    testContextualChatterFallbackWithoutPersonality();
}
