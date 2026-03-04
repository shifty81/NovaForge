// Tests for: Warp State Phase Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/ai_system.h"
#include "network/protocol_handler.h"
#include "systems/movement_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Warp State Phase Tests ====================

static void testWarpStatePhaseAlign() {
    std::cout << "\n=== Warp State Phase Align ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp initiated");
    assertTrue(warpState->phase == components::WarpState::WarpPhase::Align, "Initial phase is Align");
}

static void testWarpStatePhaseCruise() {
    std::cout << "\n=== Warp State Phase Cruise ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Warp duration ~2.5s at 200km with no Ship component (align_time=2.5 + ~0 travel)
    // Progress rate = 1/2.5 = 0.4/s; Cruise phase at progress 0.2-0.85
    // Need 0.5s+ elapsed for progress >= 0.2 (entry to cruise)
    for (int i = 0; i < 8; i++) {
        moveSys.update(0.1f);  // 0.8 seconds total, progress ~0.32
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::Cruise, "Phase is Cruise at 30% progress");
    assertTrue(warpState->warp_time > 0.0f, "Warp time is tracking");
}

static void testWarpStatePhaseExit() {
    std::cout << "\n=== Warp State Phase Exit ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Warp duration ~2.5s; Exit phase at progress > 0.85 = 2.125s elapsed
    // 22 updates of 0.1s = 2.2s, progress ~0.88 (in exit phase)
    for (int i = 0; i < 22; i++) {
        moveSys.update(0.1f);  // 2.2 seconds total
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::Exit, "Phase is Exit near completion");
}

static void testWarpStateResetOnArrival() {
    std::cout << "\n=== Warp State Reset On Arrival ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Complete the warp (10+ seconds at 0.1 progress/s)
    for (int i = 0; i < 110; i++) {
        moveSys.update(0.1f);  // 11.0 seconds total
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::None, "Phase reset to None after arrival");
    assertTrue(approxEqual(warpState->warp_time, 0.0f), "Warp time reset");
    assertTrue(approxEqual(pos->x, 200000.0f), "Ship arrived at destination X");
}

static void testWarpStateIntensity() {
    std::cout << "\n=== Warp State Intensity ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    warpState->mass_norm = 0.5f;  // medium-mass ship

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Advance a few ticks
    moveSys.update(1.0f);

    assertTrue(warpState->intensity > 0.0f, "Intensity increases during warp");
    assertTrue(warpState->intensity <= 1.0f, "Intensity clamped to max 1.0");
}

// ── Warp Disruption Tests ──────────────────────────────────────────

static void testWarpDisruptionPreventsWarp() {
    std::cout << "\n=== Warp Disruption Prevents Warp ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 1;  // default warp core strength

    // Apply disruption equal to warp strength
    warpState->warp_disrupt_strength = 1;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(!warped, "Warp rejected when disrupted (strength 1 >= warp core 1)");
}

static void testWarpDisruptionInsufficientStrength() {
    std::cout << "\n=== Warp Disruption Insufficient Strength ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 3;  // high warp core strength (e.g., warp core stabilizer fitted)

    // Apply disruption less than warp strength
    warpState->warp_disrupt_strength = 2;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp succeeds when disruption (2) < warp core strength (3)");
}

static void testIsWarpDisruptedQuery() {
    std::cout << "\n=== Is Warp Disrupted Query ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 1;

    warpState->warp_disrupt_strength = 0;
    assertTrue(!moveSys.isWarpDisrupted("ship1"), "Not disrupted with 0 disrupt strength");

    warpState->warp_disrupt_strength = 1;
    assertTrue(moveSys.isWarpDisrupted("ship1"), "Disrupted when disrupt strength >= warp core");

    warpState->warp_disrupt_strength = 3;
    assertTrue(moveSys.isWarpDisrupted("ship1"), "Disrupted when disrupt strength exceeds warp core");
}

// ── Ship-Class Warp Speed Tests ────────────────────────────────────

static void testWarpSpeedFromShipComponent() {
    std::cout << "\n=== Warp Speed From Ship Component ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_speed_au = 5.0f;   // frigate
    shipComp->align_time = 2.5f;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp initiated with frigate speed");
    assertTrue(warpState->warp_speed == 5.0f, "WarpState.warp_speed set from Ship component");
}

static void testBattleshipSlowerWarp() {
    std::cout << "\n=== Battleship Slower Warp ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    // Frigate ship
    auto* frig = world.createEntity("frigate1");
    auto* fpos = addComp<components::Position>(frig);
    fpos->x = 0.0f; fpos->y = 0.0f; fpos->z = 0.0f;
    addComp<components::Velocity>(frig);
    auto* fws = addComp<components::WarpState>(frig);
    auto* fs = addComp<components::Ship>(frig);
    fs->ship_class = "Frigate";
    fs->warp_speed_au = 5.0f;
    fs->align_time = 2.5f;

    // Battleship
    auto* bs = world.createEntity("bs1");
    auto* bpos = addComp<components::Position>(bs);
    bpos->x = 0.0f; bpos->y = 0.0f; bpos->z = 0.0f;
    addComp<components::Velocity>(bs);
    auto* bws = addComp<components::WarpState>(bs);
    auto* bsc = addComp<components::Ship>(bs);
    bsc->ship_class = "Battleship";
    bsc->warp_speed_au = 2.0f;
    bsc->align_time = 10.0f;

    moveSys.commandWarp("frigate1", 200000.0f, 0.0f, 0.0f);
    moveSys.commandWarp("bs1", 200000.0f, 0.0f, 0.0f);

    // Advance enough for frigate to complete warp (align_time=2.5s + ~0 travel ≈ 2.5s)
    for (int i = 0; i < 30; i++) {
        moveSys.update(0.1f);  // 3.0 seconds total
    }

    // Frigate should have arrived (warp_duration ≈ 2.5s, 3.0s > 2.5s)
    assertTrue(fws->phase == components::WarpState::WarpPhase::None,
               "Frigate arrived (faster warp)");

    // Battleship should still be in warp (warp_duration ≈ 10.0s, 3.0s < 10.0s)
    assertTrue(bws->phase != components::WarpState::WarpPhase::None,
               "Battleship still warping (slower warp)");
}

static void testWarpNoDisruptionWithoutWarpState() {
    std::cout << "\n=== Warp No Disruption Without WarpState ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    // No WarpState component - disruption check should return false

    assertTrue(!moveSys.isWarpDisrupted("ship1"), "No disruption without WarpState component");
}

// ── AI Dynamic Orbit Tests ──────────────────────────────────────────

static void testAIDynamicOrbitFrigate() {
    std::cout << "\n=== AI Dynamic Orbit Frigate ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Frigate");
    assertTrue(dist == 5000.0f, "Frigate orbit distance is 5000m");

    float dist2 = systems::AISystem::orbitDistanceForClass("Destroyer");
    assertTrue(dist2 == 5000.0f, "Destroyer orbit distance is 5000m");
}

static void testAIDynamicOrbitCruiser() {
    std::cout << "\n=== AI Dynamic Orbit Cruiser ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Cruiser");
    assertTrue(dist == 15000.0f, "Cruiser orbit distance is 15000m");

    float dist2 = systems::AISystem::orbitDistanceForClass("Battlecruiser");
    assertTrue(dist2 == 15000.0f, "Battlecruiser orbit distance is 15000m");
}

static void testAIDynamicOrbitBattleship() {
    std::cout << "\n=== AI Dynamic Orbit Battleship ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Battleship");
    assertTrue(dist == 30000.0f, "Battleship orbit distance is 30000m");
}

static void testAIDynamicOrbitCapital() {
    std::cout << "\n=== AI Dynamic Orbit Capital ===" << std::endl;

    float distCarrier = systems::AISystem::orbitDistanceForClass("Carrier");
    assertTrue(distCarrier == 50000.0f, "Carrier orbit distance is 50000m");

    float distTitan = systems::AISystem::orbitDistanceForClass("Titan");
    assertTrue(distTitan == 50000.0f, "Titan orbit distance is 50000m");
}

static void testAIDynamicOrbitUnknown() {
    std::cout << "\n=== AI Dynamic Orbit Unknown ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("SomeUnknownClass");
    assertTrue(dist == 10000.0f, "Unknown class orbit distance is 10000m default");
}

static void testAIEngagementRangeFromWeapon() {
    std::cout << "\n=== AI Engagement Range From Weapon ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("npc_test");
    auto* weapon = addComp<components::Weapon>(npc);
    weapon->optimal_range = 5000.0f;
    weapon->falloff_range = 2500.0f;

    float range = systems::AISystem::engagementRangeFromWeapon(npc);
    assertTrue(range == 7500.0f, "Engagement range is optimal + falloff");
}

static void testAIEngagementRangeNoWeapon() {
    std::cout << "\n=== AI Engagement Range No Weapon ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("npc_no_weapon");

    float range = systems::AISystem::engagementRangeFromWeapon(npc);
    assertTrue(range == 0.0f, "No weapon returns 0 engagement range");
}

static void testAITargetSelectionClosest() {
    std::cout << "\n=== AI Target Selection Closest ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create NPC
    auto* npc = world.createEntity("npc_selector");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->target_selection = components::AI::TargetSelection::Closest;
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(npc);

    // Create two players at different distances
    auto* far_player = world.createEntity("player_far");
    auto* far_pos = addComp<components::Position>(far_player);
    far_pos->x = 10000.0f;
    addComp<components::Player>(far_player);

    auto* near_player = world.createEntity("player_near");
    auto* near_pos = addComp<components::Position>(near_player);
    near_pos->x = 3000.0f;
    addComp<components::Player>(near_player);

    // Run AI - should pick nearest
    aiSys.update(1.0f);
    assertTrue(ai->target_entity_id == "player_near",
               "Closest selection picks nearest player");
}

static void testAITargetSelectionLowestHP() {
    std::cout << "\n=== AI Target Selection Lowest HP ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc_hp_select");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->target_selection = components::AI::TargetSelection::LowestHP;
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);

    // Player at 5000m with full HP
    auto* full_hp = world.createEntity("player_full");
    auto* fpos = addComp<components::Position>(full_hp);
    fpos->x = 5000.0f;
    addComp<components::Player>(full_hp);
    auto* fh = addComp<components::Health>(full_hp);
    fh->shield_hp = 100.0f; fh->shield_max = 100.0f;
    fh->armor_hp = 100.0f; fh->armor_max = 100.0f;
    fh->hull_hp = 100.0f; fh->hull_max = 100.0f;

    // Player at 3000m with low HP
    auto* low_hp = world.createEntity("player_low");
    auto* lpos = addComp<components::Position>(low_hp);
    lpos->x = 3000.0f;
    addComp<components::Player>(low_hp);
    auto* lh = addComp<components::Health>(low_hp);
    lh->shield_hp = 0.0f; lh->shield_max = 100.0f;
    lh->armor_hp = 10.0f; lh->armor_max = 100.0f;
    lh->hull_hp = 50.0f; lh->hull_max = 100.0f;

    aiSys.update(1.0f);
    assertTrue(ai->target_entity_id == "player_low",
               "LowestHP selection picks damaged player");
}

static void testAIDynamicOrbitApplied() {
    std::cout << "\n=== AI Dynamic Orbit Applied ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc_orbit_test");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->use_dynamic_orbit = true;
    ai->orbit_distance = 1000.0f;  // default, should be overridden
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);
    auto* ship = addComp<components::Ship>(npc);
    ship->ship_class = "Battleship";

    // Create a player in range
    auto* player = world.createEntity("player_orb");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 40000.0f;
    addComp<components::Player>(player);

    aiSys.update(1.0f);
    assertTrue(ai->orbit_distance == 30000.0f,
               "Dynamic orbit updates to battleship distance (30000m)");
}

// ── Protocol Message Tests ──────────────────────────────────────────

static void testProtocolDockMessages() {
    std::cout << "\n=== Protocol Dock Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Dock success
    std::string dockSuccess = proto.createDockSuccess("station_01");
    assertTrue(dockSuccess.find("dock_success") != std::string::npos,
               "Dock success contains message type");
    assertTrue(dockSuccess.find("station_01") != std::string::npos,
               "Dock success contains station ID");

    // Dock failed
    std::string dockFailed = proto.createDockFailed("out of range");
    assertTrue(dockFailed.find("dock_failed") != std::string::npos,
               "Dock failed contains message type");
    assertTrue(dockFailed.find("out of range") != std::string::npos,
               "Dock failed contains reason");
}

static void testProtocolUndockMessage() {
    std::cout << "\n=== Protocol Undock Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string undock = proto.createUndockSuccess();
    assertTrue(undock.find("undock_success") != std::string::npos,
               "Undock success contains message type");
}

static void testProtocolRepairMessage() {
    std::cout << "\n=== Protocol Repair Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string repair = proto.createRepairResult(5000.0f, 100.0f, 100.0f, 100.0f);
    assertTrue(repair.find("repair_result") != std::string::npos,
               "Repair result contains message type");
    assertTrue(repair.find("5000") != std::string::npos,
               "Repair result contains cost");
}

static void testProtocolDamageEventMessage() {
    std::cout << "\n=== Protocol Damage Event Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string dmg = proto.createDamageEvent("target_01", 150.0f, "kinetic",
                                               "shield", true, false, false);
    assertTrue(dmg.find("damage_event") != std::string::npos,
               "Damage event contains message type");
    assertTrue(dmg.find("target_01") != std::string::npos,
               "Damage event contains target ID");
    assertTrue(dmg.find("shield") != std::string::npos,
               "Damage event contains layer_hit");
    assertTrue(dmg.find("\"shield_depleted\":true") != std::string::npos,
               "Damage event has shield_depleted flag");
}

static void testProtocolDockRequestParse() {
    std::cout << "\n=== Protocol Dock Request Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"dock_request\",\"data\":{\"station_id\":\"s1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Dock request parses successfully");
    assertTrue(type == atlas::network::MessageType::DOCK_REQUEST, "Parsed type is DOCK_REQUEST");
}

static void testProtocolWarpMessages() {
    std::cout << "\n=== Protocol Warp Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse warp_request message
    std::string msg = "{\"message_type\":\"warp_request\",\"data\":{\"dest_x\":1000,\"dest_y\":0,\"dest_z\":5000}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Warp request parses successfully");
    assertTrue(type == atlas::network::MessageType::WARP_REQUEST, "Parsed type is WARP_REQUEST");

    // Warp result creation
    std::string result = proto.createWarpResult(true);
    assertTrue(result.find("warp_result") != std::string::npos, "Warp result contains message type");
    assertTrue(result.find("true") != std::string::npos, "Warp result contains success");

    // Warp result with failure reason
    std::string fail = proto.createWarpResult(false, "Warp drive disrupted");
    assertTrue(fail.find("false") != std::string::npos, "Warp fail contains false");
    assertTrue(fail.find("Warp drive disrupted") != std::string::npos, "Warp fail contains reason");
}

static void testProtocolMovementMessages() {
    std::cout << "\n=== Protocol Movement Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse approach message
    std::string msg = "{\"message_type\":\"approach\",\"data\":{\"target_id\":\"npc_1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Approach message parses successfully");
    assertTrue(type == atlas::network::MessageType::APPROACH, "Parsed type is APPROACH");

    // Parse orbit message
    msg = "{\"message_type\":\"orbit\",\"data\":{\"target_id\":\"npc_1\",\"distance\":5000}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Orbit message parses successfully");
    assertTrue(type == atlas::network::MessageType::ORBIT, "Parsed type is ORBIT");

    // Parse stop message
    msg = "{\"message_type\":\"stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Stop message parses successfully");
    assertTrue(type == atlas::network::MessageType::STOP, "Parsed type is STOP");

    // Movement acknowledgement creation
    std::string ack = proto.createMovementAck("approach", true);
    assertTrue(ack.find("approach") != std::string::npos, "Movement ack contains command");
    assertTrue(ack.find("true") != std::string::npos, "Movement ack contains success");
}


void run_warp_system_tests() {
    testWarpStatePhaseAlign();
    testWarpStatePhaseCruise();
    testWarpStatePhaseExit();
    testWarpStateResetOnArrival();
    testWarpStateIntensity();
    testWarpDisruptionPreventsWarp();
    testWarpDisruptionInsufficientStrength();
    testIsWarpDisruptedQuery();
    testWarpSpeedFromShipComponent();
    testBattleshipSlowerWarp();
    testWarpNoDisruptionWithoutWarpState();
    testAIDynamicOrbitFrigate();
    testAIDynamicOrbitCruiser();
    testAIDynamicOrbitBattleship();
    testAIDynamicOrbitCapital();
    testAIDynamicOrbitUnknown();
    testAIEngagementRangeFromWeapon();
    testAIEngagementRangeNoWeapon();
    testAITargetSelectionClosest();
    testAITargetSelectionLowestHP();
    testAIDynamicOrbitApplied();
    testProtocolDockMessages();
    testProtocolUndockMessage();
    testProtocolRepairMessage();
    testProtocolDamageEventMessage();
    testProtocolDockRequestParse();
    testProtocolWarpMessages();
    testProtocolMovementMessages();
}
