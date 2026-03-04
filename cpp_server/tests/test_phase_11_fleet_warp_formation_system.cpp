// Tests for: Phase 11: Fleet Warp Formation System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_warp_formation_system.h"

using namespace atlas;

// ==================== Phase 11: Fleet Warp Formation System Tests ====================

static void testFleetWarpFormationBeginEnd() {
    std::cout << "\n=== Fleet Warp Formation Begin/End ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.beginFleetWarp("ship1");
    assertTrue(sys.isInFleetWarp("ship1"), "Ship is in fleet warp");
    sys.endFleetWarp("ship1");
    assertTrue(!sys.isInFleetWarp("ship1"), "Ship no longer in fleet warp");
}

static void testFleetWarpFormationShipClassSelection() {
    std::cout << "\n=== Fleet Warp Formation Ship Class Selection ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* frig = world.createEntity("frig1");
    addComp<components::FleetWarpState>(frig);
    sys.selectFormationByShipClass("frig1", "Frigate");
    auto* ws = frig->getComponent<components::FleetWarpState>();
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::TightEchelon,
               "Frigates use TightEchelon");

    auto* cruiser = world.createEntity("cruiser1");
    addComp<components::FleetWarpState>(cruiser);
    sys.selectFormationByShipClass("cruiser1", "Cruiser");
    ws = cruiser->getComponent<components::FleetWarpState>();
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::LooseDiamond,
               "Cruisers use LooseDiamond");

    auto* cap = world.createEntity("cap1");
    addComp<components::FleetWarpState>(cap);
    sys.selectFormationByShipClass("cap1", "Capital");
    ws = cap->getComponent<components::FleetWarpState>();
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::WideCapital,
               "Capitals use WideCapital");
}

static void testFleetWarpFormationBreathing() {
    std::cout << "\n=== Fleet Warp Formation Breathing ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.beginFleetWarp("ship1");
    float offset0 = sys.getBreathingOffset("ship1");
    assertTrue(approxEqual(offset0, 0.0f, 0.01f), "Breathing starts at 0");
    sys.update(5.0f); // advance time
    float offset1 = sys.getBreathingOffset("ship1");
    // After 5s with default breathing_frequency=0.03, phase should have advanced
    assertTrue(!approxEqual(offset0, offset1, 0.001f), "Breathing changes over time");
}

static void testFleetWarpFormationDistortion() {
    std::cout << "\n=== Fleet Warp Formation Distortion ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* frig = world.createEntity("frig1");
    addComp<components::FleetWarpState>(frig);
    sys.selectFormationByShipClass("frig1", "Frigate");
    float frig_bend = sys.getDistortionBend("frig1");

    auto* cap = world.createEntity("cap1");
    addComp<components::FleetWarpState>(cap);
    sys.selectFormationByShipClass("cap1", "Capital");
    float cap_bend = sys.getDistortionBend("cap1");

    assertTrue(cap_bend > frig_bend, "Capitals bend space more than frigates");
}

static void testFleetWarpFormationLeaderAtOrigin() {
    std::cout << "\n=== Fleet Warp Formation Leader At Origin ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.selectFormationByShipClass("ship1", "Cruiser");
    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 0, ox, oy, oz);
    assertTrue(approxEqual(ox, 0.0f) && approxEqual(oy, 0.0f) && approxEqual(oz, 0.0f),
               "Leader (slot 0) at origin");
}

static void testFleetWarpFormationSlotOffsets() {
    std::cout << "\n=== Fleet Warp Formation Slot Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.selectFormationByShipClass("ship1", "Cruiser");
    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 1, ox, oy, oz);
    assertTrue(ox != 0.0f || oz != 0.0f, "Non-leader slots have non-zero offsets");
}


void run_phase_11_fleet_warp_formation_system_tests() {
    testFleetWarpFormationBeginEnd();
    testFleetWarpFormationShipClassSelection();
    testFleetWarpFormationBreathing();
    testFleetWarpFormationDistortion();
    testFleetWarpFormationLeaderAtOrigin();
    testFleetWarpFormationSlotOffsets();
}
