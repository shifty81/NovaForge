// Tests for: System Resources Component Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"

using namespace atlas;

// ==================== System Resources Component Tests ====================

static void testSystemResourcesTracking() {
    std::cout << "\n=== System Resources: Tracking ===" << std::endl;

    ecs::World world;
    auto* system = world.createEntity("system_jita");
    auto* res = addComp<components::SystemResources>(system);

    components::SystemResources::ResourceEntry ferrite;
    ferrite.mineral_type = "Ferrite";
    ferrite.total_quantity = 100000.0f;
    ferrite.remaining_quantity = 100000.0f;
    res->resources.push_back(ferrite);

    components::SystemResources::ResourceEntry galvite;
    galvite.mineral_type = "Galvite";
    galvite.total_quantity = 50000.0f;
    galvite.remaining_quantity = 30000.0f;
    res->resources.push_back(galvite);

    assertTrue(res->resources.size() == 2, "Two resource types tracked");
    assertTrue(approxEqual(res->totalRemaining(), 130000.0f), "Total remaining correct");

    // Simulate depletion
    res->resources[0].remaining_quantity -= 10000.0f;
    assertTrue(approxEqual(res->totalRemaining(), 120000.0f), "Total after depletion");
}


void run_ecs_components_tests() {
    testSystemResourcesTracking();
}
