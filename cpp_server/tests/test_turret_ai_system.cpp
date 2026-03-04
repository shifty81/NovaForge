// Tests for: Turret AI System tests
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/turret_ai_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Turret AI System tests ====================

static void testTurretAIWithinArc() {
    std::cout << "\n=== Turret AI Within Arc ===" << std::endl;
    using namespace atlas;

    // Turret facing forward (0 deg) with 90 deg arc
    assertTrue(systems::TurretAISystem::isWithinArc(0.0f, 0.0f, 90.0f),
               "Dead center is within arc");
    assertTrue(systems::TurretAISystem::isWithinArc(44.0f, 0.0f, 90.0f),
               "44 deg is within 90 deg arc");
    assertTrue(!systems::TurretAISystem::isWithinArc(46.0f, 0.0f, 90.0f),
               "46 deg is outside 90 deg arc");
    assertTrue(systems::TurretAISystem::isWithinArc(-44.0f, 0.0f, 90.0f),
               "-44 deg is within 90 deg arc");
    assertTrue(!systems::TurretAISystem::isWithinArc(-46.0f, 0.0f, 90.0f),
               "-46 deg is outside 90 deg arc");
}

static void testTurretAITrackingPenalty() {
    std::cout << "\n=== Turret AI Tracking Penalty ===" << std::endl;
    using namespace atlas;

    // Perfect tracking (stationary target)
    float p1 = systems::TurretAISystem::computeTrackingPenalty(0.0f, 1.0f);
    assertTrue(approxEqual(p1, 1.0f), "Stationary target = full damage");

    // Angular velocity equals tracking speed
    float p2 = systems::TurretAISystem::computeTrackingPenalty(1.0f, 1.0f);
    assertTrue(approxEqual(p2, 0.5f), "Equal angular/tracking = 50% damage");

    // Very fast target
    float p3 = systems::TurretAISystem::computeTrackingPenalty(10.0f, 1.0f);
    assertTrue(p3 < 0.02f, "Very fast target = near-zero damage");

    // Zero tracking speed
    float p4 = systems::TurretAISystem::computeTrackingPenalty(1.0f, 0.0f);
    assertTrue(approxEqual(p4, 0.0f), "Zero tracking speed = no hit");
}

static void testTurretAICooldown() {
    std::cout << "\n=== Turret AI Cooldown ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = true;
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 2.0f;  // 2 shots/sec
    turret->base_damage = 50.0f;
    turret->tracking_speed = 5.0f;
    turret->angular_velocity = 0.0f;  // stationary target

    // First update: should fire (cooldown starts at 0)
    sys.update(0.1f);
    assertTrue(turret->shots_fired == 1, "First shot fired");
    assertTrue(turret->cooldown_remaining > 0.0f, "Cooldown set after shot");

    // Cooldown should be 0.5 seconds (1/2 rate_of_fire)
    assertTrue(approxEqual(turret->cooldown_remaining, 0.5f),
               "Cooldown = 1/rate_of_fire");
}

static void testTurretAIDamageAccumulation() {
    std::cout << "\n=== Turret AI Damage Accumulation ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = true;
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 1.0f;   // 1 shot/sec
    turret->base_damage = 100.0f;
    turret->tracking_speed = 10.0f;
    turret->angular_velocity = 0.0f;  // stationary = full damage

    sys.update(0.1f);  // fires, sets 1s cooldown
    assertTrue(approxEqual(turret->total_damage_dealt, 100.0f),
               "Full damage on stationary target");

    // Wait for cooldown to expire
    sys.update(1.0f);  // cooldown drops to 0, fires again
    assertTrue(turret->shots_fired == 2, "Second shot after cooldown");
    assertTrue(approxEqual(turret->total_damage_dealt, 200.0f),
               "Accumulated damage after 2 shots");
}

static void testTurretAITrackingReducesDamage() {
    std::cout << "\n=== Turret AI Tracking Reduces Damage ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = true;
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 1.0f;
    turret->base_damage = 100.0f;
    turret->tracking_speed = 1.0f;
    turret->angular_velocity = 1.0f;  // matches tracking = 50% damage

    sys.update(0.1f);
    assertTrue(approxEqual(turret->total_damage_dealt, 50.0f),
               "50% damage when angular vel = tracking speed");
}

static void testTurretAINotEngaged() {
    std::cout << "\n=== Turret AI Not Engaged ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = false;  // not firing
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 1.0f;
    turret->base_damage = 100.0f;

    sys.update(1.0f);
    assertTrue(turret->shots_fired == 0, "No shots when not engaged");
    assertTrue(approxEqual(turret->total_damage_dealt, 0.0f),
               "No damage when not engaged");
}

static void testTurretAIComponentDefaults() {
    std::cout << "\n=== Turret AI Component Defaults ===" << std::endl;
    using namespace atlas;

    components::TurretAIState turret;
    assertTrue(turret.turret_index == 0, "Turret index default 0");
    assertTrue(approxEqual(turret.arc_degrees, 90.0f), "Arc default 90 deg");
    assertTrue(approxEqual(turret.direction_deg, 0.0f), "Direction default 0");
    assertTrue(approxEqual(turret.tracking_speed, 1.0f), "Tracking default 1.0");
    assertTrue(approxEqual(turret.base_damage, 10.0f), "Base damage default 10");
    assertTrue(turret.target_entity_id.empty(), "No target by default");
    assertTrue(!turret.engaged, "Not engaged by default");
    assertTrue(turret.shots_fired == 0, "No shots by default");
}


void run_turret_ai_system_tests() {
    testTurretAIWithinArc();
    testTurretAITrackingPenalty();
    testTurretAICooldown();
    testTurretAIDamageAccumulation();
    testTurretAITrackingReducesDamage();
    testTurretAINotEngaged();
    testTurretAIComponentDefaults();
}
