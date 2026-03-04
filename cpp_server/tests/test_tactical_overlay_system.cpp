// Tests for: TacticalOverlaySystem Tests, Phase 10: Tactical Overlay Shared Filters Tests, Tactical Overlay Stage 4 Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/exploration_components.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/tactical_overlay_system.h"

using namespace atlas;

// ==================== TacticalOverlaySystem Tests ====================

static void testTacticalOverlayToggle() {
    std::cout << "\n=== Tactical Overlay Toggle ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    sys.toggleOverlay("player1");
    assertTrue(sys.isEnabled("player1"), "Overlay enabled after toggle");
}

static void testTacticalOverlayToggleTwice() {
    std::cout << "\n=== Tactical Overlay Toggle Twice ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    sys.toggleOverlay("player1");
    sys.toggleOverlay("player1");
    assertTrue(!sys.isEnabled("player1"), "Overlay disabled after double toggle");
}

static void testTacticalOverlaySetToolRange() {
    std::cout << "\n=== Tactical Overlay Set Tool Range ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    auto* overlay = addComp<components::TacticalOverlayState>(entity);
    sys.setToolRange("player1", 5000.0f, "weapon");
    assertTrue(approxEqual(overlay->tool_range, 5000.0f), "Tool range set to 5000");
}

static void testTacticalOverlayRingDistances() {
    std::cout << "\n=== Tactical Overlay Ring Distances ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    std::vector<float> custom = {10.0f, 25.0f, 50.0f};
    sys.setRingDistances("player1", custom);
    auto result = sys.getRingDistances("player1");
    assertTrue(result.size() == 3, "Ring distances has 3 entries");
    assertTrue(approxEqual(result[0], 10.0f), "First ring distance is 10");
    assertTrue(approxEqual(result[2], 50.0f), "Third ring distance is 50");
}

static void testTacticalOverlayDefaultRings() {
    std::cout << "\n=== Tactical Overlay Default Rings ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    auto rings = sys.getRingDistances("player1");
    assertTrue(rings.size() == 6, "Default ring distances has 6 entries");
    assertTrue(approxEqual(rings[0], 5.0f), "Default first ring is 5.0");
    assertTrue(approxEqual(rings[5], 100.0f), "Default last ring is 100.0");
}


// ==================== Phase 10: Tactical Overlay Shared Filters Tests ====================

static void testOverlaySharedFilters() {
    std::cout << "\n=== Overlay Shared Filters ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);

    auto defaults = sys.getFilterCategories("player1");
    assertTrue(defaults.size() == 4, "Default has 4 filter categories");

    std::vector<std::string> custom = {"hostile", "structure"};
    sys.setFilterCategories("player1", custom);
    auto updated = sys.getFilterCategories("player1");
    assertTrue(updated.size() == 2, "Updated to 2 filter categories");
    assertTrue(updated[0] == "hostile", "First filter is hostile");
    assertTrue(updated[1] == "structure", "Second filter is structure");
}

static void testOverlayPassiveDisplayOnly() {
    std::cout << "\n=== Overlay Passive Display Only ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);

    assertTrue(sys.isPassiveDisplayOnly("player1"), "Overlay is passive by default");
}

static void testOverlayEntityPriority() {
    std::cout << "\n=== Overlay Entity Priority ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);

    assertTrue(approxEqual(sys.getEntityDisplayPriority("player1"), 1.0f),
               "Default entity priority is 1.0");

    sys.setEntityDisplayPriority("player1", 5.0f);
    assertTrue(approxEqual(sys.getEntityDisplayPriority("player1"), 5.0f),
               "Entity priority updated to 5.0");
}

static void testOverlayEntityPriorityHostileHighAsteroidLow() {
    std::cout << "\n=== Overlay Hostile High / Asteroid Low Priority ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* hostile = world.createEntity("hostile1");
    addComp<components::TacticalOverlayState>(hostile);
    sys.setEntityDisplayPriority("hostile1", 10.0f);

    auto* asteroid = world.createEntity("asteroid1");
    addComp<components::TacticalOverlayState>(asteroid);
    sys.setEntityDisplayPriority("asteroid1", 0.5f);

    assertTrue(sys.getEntityDisplayPriority("hostile1") > sys.getEntityDisplayPriority("asteroid1"),
               "Hostile higher priority than asteroid");
}


// ==================== Tactical Overlay Stage 4 Tests ====================

static void testOverlayAnchorRing() {
    std::cout << "\n=== Tactical Overlay: Anchor Ring ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_anchor");

    toSys.setAnchorRing("overlay_anchor", "fc_ship", 25.0f);
    assertTrue(approxEqual(toSys.getAnchorRingRadius("overlay_anchor"), 25.0f),
               "Anchor ring radius set to 25");
}

static void testOverlayAnchorRingDisabled() {
    std::cout << "\n=== Tactical Overlay: Anchor Ring Disabled ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_noanchor");
    addComp<components::TacticalOverlayState>(player);

    assertTrue(toSys.getAnchorRingRadius("overlay_noanchor") == 0.0f,
               "Anchor ring disabled by default");
}

static void testOverlayWingBands() {
    std::cout << "\n=== Tactical Overlay: Wing Bands ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_wings");

    std::vector<float> offsets = {10.0f, 20.0f, 30.0f};
    toSys.setWingBands("overlay_wings", true, offsets);
    assertTrue(toSys.areWingBandsEnabled("overlay_wings"), "Wing bands enabled");

    auto result = toSys.getWingBandOffsets("overlay_wings");
    assertTrue(result.size() == 3, "Three wing band offsets");
    assertTrue(approxEqual(result[0], 10.0f), "First offset correct");
    assertTrue(approxEqual(result[2], 30.0f), "Third offset correct");
}

static void testOverlayWingBandsDisabledByDefault() {
    std::cout << "\n=== Tactical Overlay: Wing Bands Disabled Default ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_nowings");
    addComp<components::TacticalOverlayState>(player);

    assertTrue(!toSys.areWingBandsEnabled("overlay_nowings"), "Wing bands disabled by default");
    assertTrue(toSys.getWingBandOffsets("overlay_nowings").empty(), "No offsets by default");
}

static void testOverlayFleetExtensionsMissing() {
    std::cout << "\n=== Tactical Overlay: Fleet Extensions Missing Entity ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    assertTrue(toSys.getAnchorRingRadius("nobody") == 0.0f, "Missing entity anchor ring is 0");
    assertTrue(!toSys.areWingBandsEnabled("nobody"), "Missing entity wing bands disabled");
    assertTrue(toSys.getWingBandOffsets("nobody").empty(), "Missing entity no offsets");
}


void run_tactical_overlay_system_tests() {
    testTacticalOverlayToggle();
    testTacticalOverlayToggleTwice();
    testTacticalOverlaySetToolRange();
    testTacticalOverlayRingDistances();
    testTacticalOverlayDefaultRings();
    testOverlaySharedFilters();
    testOverlayPassiveDisplayOnly();
    testOverlayEntityPriority();
    testOverlayEntityPriorityHostileHighAsteroidLow();
    testOverlayAnchorRing();
    testOverlayAnchorRingDisabled();
    testOverlayWingBands();
    testOverlayWingBandsDisabledByDefault();
    testOverlayFleetExtensionsMissing();
}
