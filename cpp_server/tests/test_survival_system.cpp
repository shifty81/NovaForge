// Tests for: Survival System Tests
#include "test_log.h"
#include "components/crew_components.h"
#include "ecs/system.h"
#include "systems/survival_system.h"

using namespace atlas;

// ==================== Survival System Tests ====================

static void testSurvivalDefaults() {
    std::cout << "\n=== Survival Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv1");
    auto* needs = addComp<components::SurvivalNeeds>(e);
    assertTrue(approxEqual(needs->oxygen, 100.0f), "Full oxygen");
    assertTrue(approxEqual(needs->hunger, 0.0f), "No hunger");
    assertTrue(approxEqual(needs->fatigue, 0.0f), "No fatigue");
}

static void testSurvivalDrain() {
    std::cout << "\n=== Survival Drain ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv2");
    addComp<components::SurvivalNeeds>(e);

    systems::SurvivalSystem sys(&world);
    sys.update(10.0f);

    auto [oxy, hun, fat] = sys.getNeeds("surv2");
    assertTrue(oxy < 100.0f, "Oxygen drained");
    assertTrue(approxEqual(oxy, 95.0f), "Oxygen at 95 after 10s");
}

static void testSurvivalRefill() {
    std::cout << "\n=== Survival Refill ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv3");
    auto* needs = addComp<components::SurvivalNeeds>(e);
    needs->oxygen = 50.0f;

    systems::SurvivalSystem sys(&world);
    float newOxy = sys.refillOxygen("surv3", 30.0f);
    assertTrue(approxEqual(newOxy, 80.0f), "Oxygen refilled to 80");
}

static void testSurvivalHunger() {
    std::cout << "\n=== Survival Hunger ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv4");
    addComp<components::SurvivalNeeds>(e);

    systems::SurvivalSystem sys(&world);
    sys.update(20.0f);

    auto [oxy, hun, fat] = sys.getNeeds("surv4");
    assertTrue(hun > 0.0f, "Hunger increased");
    assertTrue(approxEqual(hun, 2.0f), "Hunger at 2.0 after 20s");
}


void run_survival_system_tests() {
    testSurvivalDefaults();
    testSurvivalDrain();
    testSurvivalRefill();
    testSurvivalHunger();
}
