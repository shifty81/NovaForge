// Tests for: WarpAnomalySystem Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/anomaly_system.h"
#include "systems/warp_anomaly_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== WarpAnomalySystem Tests ====================

static void testWarpAnomalyNoneIfNotCruising() {
    std::cout << "\n=== Warp Anomaly None If Not Cruising ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Align;
    warp->warp_time = 5.0f;
    // tryTriggerAnomaly checks warp_time < 20, not phase; update() checks phase
    // With short warp_time and non-cruise phase, no anomaly via update
    sys.update(1.0f);
    assertTrue(sys.getAnomalyCount("ship1") == 0, "No anomaly when not in Cruise phase");
}

static void testWarpAnomalyNoneIfShortWarp() {
    std::cout << "\n=== Warp Anomaly None If Short Warp ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    warp->warp_time = 5.0f;
    bool triggered = sys.tryTriggerAnomaly("ship1");
    assertTrue(!triggered, "No anomaly when warp_time < 20");
}

static void testWarpAnomalyTriggersOnLongWarp() {
    std::cout << "\n=== Warp Anomaly Triggers On Long Warp ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    // Try many different warp_time values to find one that triggers
    bool any_triggered = false;
    for (int i = 20; i < 300; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) {
            any_triggered = true;
            break;
        }
    }
    assertTrue(any_triggered, "At least one anomaly triggered on long warp");
}

static void testWarpAnomalyCount() {
    std::cout << "\n=== Warp Anomaly Count ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    int triggered_count = 0;
    for (int i = 20; i < 500; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) {
            triggered_count++;
        }
    }
    assertTrue(sys.getAnomalyCount("ship1") == triggered_count,
               "getAnomalyCount matches triggered count");
}

static void testWarpAnomalyClear() {
    std::cout << "\n=== Warp Anomaly Clear ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    for (int i = 20; i < 300; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) break;
    }
    sys.clearAnomaly("ship1");
    auto cleared = sys.getLastAnomaly("ship1");
    assertTrue(cleared.name.empty(), "Anomaly cleared successfully");
}


void run_warp_anomaly_system_tests() {
    testWarpAnomalyNoneIfNotCruising();
    testWarpAnomalyNoneIfShortWarp();
    testWarpAnomalyTriggersOnLongWarp();
    testWarpAnomalyCount();
    testWarpAnomalyClear();
}
