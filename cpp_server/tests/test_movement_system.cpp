// Tests for: Movement System & Collision Tests, Movement Command Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"

using namespace atlas;

// ==================== Movement System & Collision Tests ====================

static void testMovementBasicUpdate() {
    std::cout << "\n=== Movement Basic Update ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    auto* entity = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    assertTrue(approxEqual(pos->x, 100.0f), "Position updated by velocity * dt");
    assertTrue(approxEqual(pos->y, 0.0f), "Y unchanged when vy = 0");
}

static void testMovementSpeedLimit() {
    std::cout << "\n=== Movement Speed Limit ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    auto* entity = world.createEntity("ship2");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    pos->x = 0.0f;
    vel->vx = 500.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
    assertTrue(speed <= vel->max_speed + 0.01f, "Speed clamped to max_speed");
}

static void testMovementCollisionZonePush() {
    std::cout << "\n=== Movement Collision Zone Push ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    // Set up a collision zone at origin (like a sun)
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});  // 500km radius sun
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship3");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Place ship inside the sun's collision zone
    pos->x = 100000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = -100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // After update, ship should be pushed outside the collision zone
    float dist = std::sqrt(pos->x * pos->x + pos->y * pos->y + pos->z * pos->z);
    assertTrue(dist >= 500000.0f, "Ship pushed outside collision zone (sun)");
}

static void testMovementCollisionZoneVelocityKilled() {
    std::cout << "\n=== Movement Collision Zone Velocity Killed ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship4");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship inside zone moving toward center
    pos->x = 100000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = -200.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 300.0f;
    
    moveSys.update(1.0f);
    
    // Velocity toward the celestial should be killed
    assertTrue(vel->vx >= 0.0f, "Velocity toward celestial killed (bounce effect)");
}

static void testMovementOutsideCollisionZoneUnaffected() {
    std::cout << "\n=== Movement Outside Collision Zone Unaffected ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship5");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship well outside the collision zone
    pos->x = 1000000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 50.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // Position should be updated normally (not pushed)
    assertTrue(approxEqual(pos->x, 1000100.0f), "Ship outside zone moves normally in X");
    assertTrue(approxEqual(pos->y, 50.0f), "Ship outside zone moves normally in Y");
}

static void testMovementMultipleCollisionZones() {
    std::cout << "\n=== Movement Multiple Collision Zones ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    // Sun at origin, planet at 1M meters
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});        // Sun
    zones.push_back({1000000.0f, 0.0f, 0.0f, 6000.0f});     // Planet
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship6");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship inside planet's collision zone
    pos->x = 999000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // Ship should be pushed out of planet's collision zone
    float distToPlanet = std::sqrt((pos->x - 1000000.0f) * (pos->x - 1000000.0f) + 
                                    pos->y * pos->y + pos->z * pos->z);
    assertTrue(distToPlanet >= 6000.0f, "Ship pushed outside planet collision zone");
}


// ==================== Movement Command Tests ====================

static void testMovementOrbitCommand() {
    std::cout << "\n=== Movement Orbit Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(ship);
    vel->max_speed = 200.0f;

    auto* target = world.createEntity("target1");
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;
    addComp<components::Velocity>(target);

    moveSys.commandOrbit("ship1", "target1", 500.0f);
    moveSys.update(1.0f);

    // Ship should be moving (velocity non-zero)
    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
    assertTrue(speed > 0.0f, "Ship has velocity after orbit command");
}

static void testMovementApproachCommand() {
    std::cout << "\n=== Movement Approach Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(ship);
    vel->max_speed = 200.0f;

    auto* target = world.createEntity("target1");
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;
    addComp<components::Velocity>(target);

    moveSys.commandApproach("ship1", "target1");
    moveSys.update(1.0f);

    // Ship should be moving toward target (positive vx)
    assertTrue(vel->vx > 0.0f, "Ship moving toward target (positive X)");
    assertTrue(pos->x > 0.0f, "Ship position moved toward target");
}

static void testMovementStopCommand() {
    std::cout << "\n=== Movement Stop Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Position>(ship);
    auto* vel = addComp<components::Velocity>(ship);
    vel->vx = 100.0f;
    vel->vy = 50.0f;
    vel->max_speed = 200.0f;

    moveSys.commandStop("ship1");
    assertTrue(vel->vx == 0.0f && vel->vy == 0.0f && vel->vz == 0.0f,
               "Velocity zeroed after stop command");
}

static void testMovementWarpDistance() {
    std::cout << "\n=== Movement Warp Distance Check ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);

    // Try to warp too close (< 150km)
    bool warped = moveSys.commandWarp("ship1", 100.0f, 0.0f, 0.0f);
    assertTrue(!warped, "Warp rejected (destination too close)");

    // Warp to valid distance
    warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp accepted (>150km)");
}


void run_movement_system_tests() {
    testMovementBasicUpdate();
    testMovementSpeedLimit();
    testMovementCollisionZonePush();
    testMovementCollisionZoneVelocityKilled();
    testMovementOutsideCollisionZoneUnaffected();
    testMovementMultipleCollisionZones();
    testMovementOrbitCommand();
    testMovementApproachCommand();
    testMovementStopCommand();
    testMovementWarpDistance();
}
