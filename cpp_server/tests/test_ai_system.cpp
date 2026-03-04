// Tests for: AI Retreat Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/ai_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== AI Retreat Tests ====================

static void testAIFleeOnLowHealth() {
    std::cout << "\n=== AI Flee On Low Health ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.25f;

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 0.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 0.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 50.0f;   // 50 out of 300 total = 16.7% < 25%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Fleeing, "NPC flees when health below threshold");
}

static void testAINoFleeAboveThreshold() {
    std::cout << "\n=== AI No Flee Above Threshold ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.25f;

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 50.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 50.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 100.0f;   // 200 out of 300 total = 66.7% > 25%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    // NPC needs a weapon to stay in attacking state (orbitBehavior checks for weapon)
    addComp<components::Weapon>(npc);

    aiSys.update(0.1f);

    assertTrue(ai->state != components::AI::State::Fleeing, "NPC does not flee when health above threshold");
}

static void testAIFleeThresholdCustom() {
    std::cout << "\n=== AI Flee Threshold Custom ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.50f;  // Custom high threshold

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 30.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 30.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 80.0f;   // 140 out of 300 = 46.7% < 50%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Fleeing, "NPC flees with custom high threshold");
}


void run_ai_system_tests() {
    testAIFleeOnLowHealth();
    testAINoFleeAboveThreshold();
    testAIFleeThresholdCustom();
}
