// Tests for: FleetWarpFormationSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/fleet_warp_formation_system.h"

#include <cmath>

using namespace atlas;

// ==================== FleetWarpFormationSystem Tests ====================

static void testFleetWarpBeginEnd() {
    std::cout << "\n=== FleetWarpFormation: BeginEnd ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    assertTrue(!sys.isInFleetWarp("ship1"), "Not in fleet warp initially");

    sys.beginFleetWarp("ship1");
    assertTrue(sys.isInFleetWarp("ship1"), "In fleet warp after begin");

    sys.endFleetWarp("ship1");
    assertTrue(!sys.isInFleetWarp("ship1"), "Not in fleet warp after end");
}

static void testFleetWarpBeginCreatesComponent() {
    std::cout << "\n=== FleetWarpFormation: BeginCreatesComponent ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    // No component yet
    sys.beginFleetWarp("ship1");
    assertTrue(sys.isInFleetWarp("ship1"), "Component created and warp started");
}

static void testFleetWarpFrigateFormation() {
    std::cout << "\n=== FleetWarpFormation: FrigateFormation ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    sys.selectFormationByShipClass("ship1", "Frigate");
    auto* e = world.getEntity("ship1");
    auto* ws = e->getComponent<components::FleetWarpState>();

    assertTrue(ws != nullptr, "Component created by selectFormation");
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::TightEchelon,
               "Frigate uses TightEchelon");
    assertTrue(approxEqual(ws->breathing_frequency, 0.05f), "Frigate breathing freq 0.05");
    assertTrue(approxEqual(ws->distortion_bend, 0.2f), "Frigate distortion 0.2");
}

static void testFleetWarpDestroyerFormation() {
    std::cout << "\n=== FleetWarpFormation: DestroyerFormation ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    sys.selectFormationByShipClass("ship1", "Destroyer");
    auto* e = world.getEntity("ship1");
    auto* ws = e->getComponent<components::FleetWarpState>();

    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::TightEchelon,
               "Destroyer uses TightEchelon");
}

static void testFleetWarpCruiserFormation() {
    std::cout << "\n=== FleetWarpFormation: CruiserFormation ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    sys.selectFormationByShipClass("ship1", "Cruiser");
    auto* e = world.getEntity("ship1");
    auto* ws = e->getComponent<components::FleetWarpState>();

    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::LooseDiamond,
               "Cruiser uses LooseDiamond");
    assertTrue(approxEqual(ws->breathing_frequency, 0.03f), "Cruiser breathing freq 0.03");
    assertTrue(approxEqual(ws->distortion_bend, 0.5f), "Cruiser distortion 0.5");
}

static void testFleetWarpBattlecruiserFormation() {
    std::cout << "\n=== FleetWarpFormation: BattlecruiserFormation ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    sys.selectFormationByShipClass("ship1", "Battlecruiser");
    auto* e = world.getEntity("ship1");
    auto* ws = e->getComponent<components::FleetWarpState>();

    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::LooseDiamond,
               "Battlecruiser uses LooseDiamond");
}

static void testFleetWarpCapitalFormation() {
    std::cout << "\n=== FleetWarpFormation: CapitalFormation ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    sys.selectFormationByShipClass("ship1", "Titan");
    auto* e = world.getEntity("ship1");
    auto* ws = e->getComponent<components::FleetWarpState>();

    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::WideCapital,
               "Titan uses WideCapital");
    assertTrue(approxEqual(ws->breathing_frequency, 0.02f), "Capital breathing freq 0.02");
    assertTrue(approxEqual(ws->distortion_bend, 1.0f), "Capital distortion 1.0");
    assertTrue(approxEqual(ws->wake_ripple, 0.8f), "Capital wake ripple 0.8");
}

static void testFleetWarpBreathingOscillation() {
    std::cout << "\n=== FleetWarpFormation: BreathingOscillation ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("ship1");

    sys.selectFormationByShipClass("ship1", "Frigate");
    sys.beginFleetWarp("ship1");

    // Initial breathing offset should be 0 (sin(0) = 0)
    assertTrue(approxEqual(sys.getBreathingOffset("ship1"), 0.0f), "Breathing offset starts at 0");

    // After update, breathing phase advances
    sys.update(1.0f);
    float offset = sys.getBreathingOffset("ship1");
    assertTrue(offset != 0.0f, "Breathing offset changes after update");
}

static void testFleetWarpNoUpdateWhenNotWarping() {
    std::cout << "\n=== FleetWarpFormation: NoUpdateWhenNotWarping ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* ws = addComp<components::FleetWarpState>(e);
    ws->in_fleet_warp = false;
    ws->breathing_phase = 0.0f;

    sys.update(10.0f);
    assertTrue(approxEqual(ws->breathing_phase, 0.0f), "Phase unchanged when not warping");
}

static void testFleetWarpLeaderOffset() {
    std::cout << "\n=== FleetWarpFormation: LeaderOffset ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* ws = addComp<components::FleetWarpState>(e);
    ws->warp_formation = components::FleetWarpState::WarpFormationType::TightEchelon;

    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 0, ox, oy, oz);
    assertTrue(ox == 0.0f && oy == 0.0f && oz == 0.0f, "Leader has zero offset");
}

static void testFleetWarpEchelonOffsets() {
    std::cout << "\n=== FleetWarpFormation: EchelonOffsets ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* ws = addComp<components::FleetWarpState>(e);
    ws->warp_formation = components::FleetWarpState::WarpFormationType::TightEchelon;
    ws->breathing_phase = 0.0f; // sin(0) = 0, no breathing contribution

    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 1, ox, oy, oz);
    assertTrue(oz < 0.0f, "Echelon: slot 1 trails behind leader");
    assertTrue(oy == 0.0f, "Echelon: no vertical offset");

    sys.computeWarpOffset("ship1", 2, ox, oy, oz);
    assertTrue(oz < 0.0f, "Echelon: slot 2 trails behind leader");
}

static void testFleetWarpDiamondOffsets() {
    std::cout << "\n=== FleetWarpFormation: DiamondOffsets ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* ws = addComp<components::FleetWarpState>(e);
    ws->warp_formation = components::FleetWarpState::WarpFormationType::LooseDiamond;
    ws->breathing_phase = 0.0f;

    float ox1, oy1, oz1, ox2, oy2, oz2;
    sys.computeWarpOffset("ship1", 1, ox1, oy1, oz1);
    sys.computeWarpOffset("ship1", 2, ox2, oy2, oz2);

    // Slots 1 and 2 should be on opposite sides
    assertTrue(ox1 * ox2 < 0.0f, "Diamond: slots on opposite sides");
    assertTrue(oz1 < 0.0f, "Diamond: slot 1 trails");
}

static void testFleetWarpWideCapitalOffsets() {
    std::cout << "\n=== FleetWarpFormation: WideCapitalOffsets ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* ws = addComp<components::FleetWarpState>(e);
    ws->warp_formation = components::FleetWarpState::WarpFormationType::WideCapital;
    ws->breathing_phase = 0.0f;

    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 1, ox, oy, oz);
    // Wide capital has 1.5x spacing multiplier on ox
    assertTrue(std::fabs(ox) > 0.0f, "Capital: non-zero lateral offset");
    assertTrue(oz < 0.0f, "Capital: trails behind");
}

static void testFleetWarpDistortionBend() {
    std::cout << "\n=== FleetWarpFormation: DistortionBend ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    world.createEntity("frig");
    world.createEntity("titan");

    sys.selectFormationByShipClass("frig", "Frigate");
    sys.selectFormationByShipClass("titan", "Titan");

    float frig_bend = sys.getDistortionBend("frig");
    float titan_bend = sys.getDistortionBend("titan");

    assertTrue(frig_bend < titan_bend, "Larger ships have more distortion bend");
}

static void testFleetWarpMissingEntity() {
    std::cout << "\n=== FleetWarpFormation: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);

    assertTrue(!sys.isInFleetWarp("ghost"), "Not in warp for missing");
    assertTrue(approxEqual(sys.getBreathingOffset("ghost"), 0.0f), "Breathing 0 for missing");
    assertTrue(approxEqual(sys.getDistortionBend("ghost"), 0.0f), "Distortion 0 for missing");

    float ox, oy, oz;
    sys.computeWarpOffset("ghost", 1, ox, oy, oz);
    assertTrue(ox == 0.0f && oy == 0.0f && oz == 0.0f, "Offset zero for missing");
}

static void testFleetWarpSpacingConstant() {
    std::cout << "\n=== FleetWarpFormation: SpacingConstant ===" << std::endl;
    assertTrue(approxEqual(systems::FleetWarpFormationSystem::kWarpSpacing, 800.0f),
               "Default warp spacing is 800m");
}

void run_fleet_warp_formation_system_tests() {
    testFleetWarpBeginEnd();
    testFleetWarpBeginCreatesComponent();
    testFleetWarpFrigateFormation();
    testFleetWarpDestroyerFormation();
    testFleetWarpCruiserFormation();
    testFleetWarpBattlecruiserFormation();
    testFleetWarpCapitalFormation();
    testFleetWarpBreathingOscillation();
    testFleetWarpNoUpdateWhenNotWarping();
    testFleetWarpLeaderOffset();
    testFleetWarpEchelonOffsets();
    testFleetWarpDiamondOffsets();
    testFleetWarpWideCapitalOffsets();
    testFleetWarpDistortionBend();
    testFleetWarpMissingEntity();
    testFleetWarpSpacingConstant();
}
