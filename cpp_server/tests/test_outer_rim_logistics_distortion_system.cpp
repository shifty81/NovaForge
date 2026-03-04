// Tests for: OuterRimLogisticsDistortion System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/outer_rim_logistics_distortion_system.h"

using namespace atlas;

// ===== OuterRimLogisticsDistortion System Tests =====

static void testOuterRimCreate() {
    std::cout << "\n=== OuterRim: Create ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    assertTrue(sys.initializeRegion("region1", "outer_rim_alpha"), "Init region succeeds");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 0.0f), "Initial threat is 0");
    assertTrue(sys.getRouteCount("region1") == 0, "No routes initially");
    assertTrue(sys.getDisruptedRouteCount("region1") == 0, "No disrupted routes");
    assertTrue(approxEqual(sys.getTotalPriceImpact("region1"), 0.0f), "No price impact");
}

static void testOuterRimAddRoute() {
    std::cout << "\n=== OuterRim: AddRoute ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    assertTrue(sys.addRoute("region1", "route_a"), "Add route succeeds");
    assertTrue(sys.getRouteCount("region1") == 1, "Route count is 1");
    assertTrue(approxEqual(sys.getRouteEfficiency("region1", "route_a"), 1.0f), "Default efficiency is 1.0");
    assertTrue(!sys.addRoute("region1", "route_a"), "Duplicate route fails");
}

static void testOuterRimGlobalThreat() {
    std::cout << "\n=== OuterRim: GlobalThreat ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    assertTrue(sys.setGlobalThreat("region1", 0.6f), "Set global threat");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 0.6f), "Threat is 0.6");
    assertTrue(sys.setGlobalThreat("region1", 2.0f), "Set threat past max");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 1.0f), "Threat clamped at 1.0");
    assertTrue(sys.setGlobalThreat("region1", -1.0f), "Set threat below min");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 0.0f), "Threat clamped at 0.0");
}

static void testOuterRimRouteThreat() {
    std::cout << "\n=== OuterRim: RouteThreat ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    assertTrue(sys.setRouteThreat("region1", "route_a", 0.4f), "Set route threat");
    assertTrue(!sys.setRouteThreat("region1", "route_missing", 0.5f), "Set threat on missing route fails");
}

static void testOuterRimUpdateDistortion() {
    std::cout << "\n=== OuterRim: UpdateDistortion ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f);
    // target_efficiency = 1.0 - (0.8 * 0.5) = 0.6, distortion is instant downward
    assertTrue(approxEqual(sys.getRouteEfficiency("region1", "route_a"), 0.6f), "Efficiency dropped to 0.6");
}

static void testOuterRimPriceImpact() {
    std::cout << "\n=== OuterRim: PriceImpact ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f);
    // price_impact = (1.0 - 0.6) * 2.0 = 0.8
    assertTrue(approxEqual(sys.getRoutePriceImpact("region1", "route_a"), 0.8f), "Price impact is 0.8");
    assertTrue(approxEqual(sys.getTotalPriceImpact("region1"), 0.8f), "Total price impact is 0.8");
}

static void testOuterRimRecovery() {
    std::cout << "\n=== OuterRim: Recovery ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f); // efficiency drops to 0.6
    sys.setGlobalThreat("region1", 0.0f);
    sys.update(1.0f); // recovery: 0.6 + 0.05*1.0 = 0.65
    float eff = sys.getRouteEfficiency("region1", "route_a");
    assertTrue(approxEqual(eff, 0.65f), "Efficiency recovering toward 1.0");
}

static void testOuterRimDisruptedCount() {
    std::cout << "\n=== OuterRim: DisruptedCount ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.addRoute("region1", "route_b");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f);
    // Both routes at 0.6 efficiency (< 0.8 threshold)
    assertTrue(sys.getDisruptedRouteCount("region1") == 2, "2 disrupted routes");
}

static void testOuterRimMaxRoutes() {
    std::cout << "\n=== OuterRim: MaxRoutes ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    for (int i = 0; i < 55; i++) {
        sys.addRoute("region1", "route_" + std::to_string(i));
    }
    assertTrue(sys.getRouteCount("region1") == 50, "Routes capped at max_routes");
}

static void testOuterRimMissing() {
    std::cout << "\n=== OuterRim: Missing ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    assertTrue(!sys.initializeRegion("nonexistent", "r1"), "Init fails on missing entity");
    assertTrue(!sys.addRoute("nonexistent", "route_a"), "Add route fails on missing");
    assertTrue(!sys.setGlobalThreat("nonexistent", 0.5f), "Set threat fails on missing");
    assertTrue(!sys.setRouteThreat("nonexistent", "route_a", 0.5f), "Set route threat fails on missing");
    assertTrue(approxEqual(sys.getRouteEfficiency("nonexistent", "route_a"), 0.0f), "0 efficiency on missing");
    assertTrue(approxEqual(sys.getGlobalThreat("nonexistent"), 0.0f), "0 threat on missing");
    assertTrue(sys.getRouteCount("nonexistent") == 0, "0 routes on missing");
    assertTrue(sys.getDisruptedRouteCount("nonexistent") == 0, "0 disrupted on missing");
    assertTrue(approxEqual(sys.getTotalPriceImpact("nonexistent"), 0.0f), "0 price impact on missing");
}


void run_outer_rim_logistics_distortion_system_tests() {
    testOuterRimCreate();
    testOuterRimAddRoute();
    testOuterRimGlobalThreat();
    testOuterRimRouteThreat();
    testOuterRimUpdateDistortion();
    testOuterRimPriceImpact();
    testOuterRimRecovery();
    testOuterRimDisruptedCount();
    testOuterRimMaxRoutes();
    testOuterRimMissing();
}
