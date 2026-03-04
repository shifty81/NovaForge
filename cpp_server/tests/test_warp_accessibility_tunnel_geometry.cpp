// Tests for: WarpAccessibility tunnel_geometry Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_cinematic_system.h"

using namespace atlas;

// ==================== WarpAccessibility tunnel_geometry Tests ====================

static void testAccessibilityTunnelGeometryToggle() {
    std::cout << "\n=== Accessibility tunnel_geometry toggle ===" << std::endl;
    components::WarpAccessibility access;
    assertTrue(access.tunnel_geometry_enabled, "tunnel geometry on by default");
    access.tunnel_geometry_enabled = false;
    assertTrue(!access.tunnel_geometry_enabled, "tunnel geometry toggled off");
}

static void testCinematicSystemTunnelGeometryDisabled() {
    std::cout << "\n=== Cinematic system with tunnel geometry disabled ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_tunnel_geom");
    auto* ws = addComp<components::WarpState>(e);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->mass_norm = 0.5f;
    addComp<components::WarpTunnelConfig>(e);
    auto* access = addComp<components::WarpAccessibility>(e);
    access->tunnel_geometry_enabled = false;

    systems::WarpCinematicSystem sys(&world);
    sys.update(0.016f);

    auto* tc = e->getComponent<components::WarpTunnelConfig>();
    assertTrue(approxEqual(tc->tunnel_skin, 0.0f), "tunnel_skin zeroed when geometry disabled");
    assertTrue(tc->starfield_bloom > 0.0f, "star streaks still active");
}


void run_warp_accessibility_tunnel_geometry_tests() {
    testAccessibilityTunnelGeometryToggle();
    testCinematicSystemTunnelGeometryDisabled();
}
