// Tests for: LOD Priority Component Tests, Phase 5 Continued: LOD System Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/lod_system.h"

using namespace atlas;

// ==================== LOD Priority Component Tests ====================

static void testLODPriorityDefaults() {
    std::cout << "\n=== LOD Priority Defaults ===" << std::endl;
    components::LODPriority lod;
    assertTrue(approxEqual(lod.priority, 1.0f), "Default priority is 1.0");
    assertTrue(!lod.force_visible, "Default force_visible is false");
    assertTrue(approxEqual(lod.impostor_distance, 500.0f), "Default impostor_distance is 500.0");
}

static void testLODPriorityPlayerShip() {
    std::cout << "\n=== LOD Priority Player Ship ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("player_ship_lod");
    auto* lod = addComp<components::LODPriority>(ship);
    lod->priority = 2.0f;
    lod->force_visible = true;

    auto* retrieved = ship->getComponent<components::LODPriority>();
    assertTrue(retrieved != nullptr, "LODPriority component retrievable");
    assertTrue(approxEqual(retrieved->priority, 2.0f), "Priority set to 2.0 for player ship");
    assertTrue(retrieved->force_visible, "Player ship forced visible");
}


// ==================== Phase 5 Continued: LOD System Tests ====================

static void testLODSystemDefaults() {
    std::cout << "\n=== LOD System: Default Thresholds ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);

    assertTrue(approxEqual(lodSys.getNearThreshold(), 5000.0f), "Default near threshold is 5000");
    assertTrue(approxEqual(lodSys.getMidThreshold(), 20000.0f), "Default mid threshold is 20000");
    assertTrue(approxEqual(lodSys.getFarThreshold(), 80000.0f), "Default far threshold is 80000");
}

static void testLODSystemPriorityComputation() {
    std::cout << "\n=== LOD System: Priority Computation ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    // Entity within near threshold (1000m)
    auto* eNear = world.createEntity("lod_near");
    auto* pNear = addComp<components::Position>(eNear);
    pNear->x = 1000.0f; pNear->y = 0.0f; pNear->z = 0.0f;
    addComp<components::LODPriority>(eNear);

    // Entity at mid range (10000m)
    auto* eMid = world.createEntity("lod_mid");
    auto* pMid = addComp<components::Position>(eMid);
    pMid->x = 10000.0f; pMid->y = 0.0f; pMid->z = 0.0f;
    addComp<components::LODPriority>(eMid);

    // Entity at far range (50000m)
    auto* eFar = world.createEntity("lod_far");
    auto* pFar = addComp<components::Position>(eFar);
    pFar->x = 50000.0f; pFar->y = 0.0f; pFar->z = 0.0f;
    addComp<components::LODPriority>(eFar);

    // Entity beyond far (100000m)
    auto* eImp = world.createEntity("lod_impostor");
    auto* pImp = addComp<components::Position>(eImp);
    pImp->x = 100000.0f; pImp->y = 0.0f; pImp->z = 0.0f;
    addComp<components::LODPriority>(eImp);

    lodSys.update(1.0f);

    assertTrue(approxEqual(eNear->getComponent<components::LODPriority>()->priority, 2.0f),
               "Near entity gets full detail priority");
    assertTrue(approxEqual(eMid->getComponent<components::LODPriority>()->priority, 1.0f),
               "Mid entity gets reduced priority");
    assertTrue(approxEqual(eFar->getComponent<components::LODPriority>()->priority, 0.5f),
               "Far entity gets merged priority");
    assertTrue(approxEqual(eImp->getComponent<components::LODPriority>()->priority, 0.1f),
               "Impostor entity gets lowest priority");

    assertTrue(lodSys.getFullDetailCount() == 1, "1 entity at full detail");
    assertTrue(lodSys.getReducedCount() == 1, "1 entity at reduced");
    assertTrue(lodSys.getMergedCount() == 1, "1 entity at merged");
    assertTrue(lodSys.getImpostorCount() == 1, "1 entity at impostor");
}

static void testLODSystemForceVisible() {
    std::cout << "\n=== LOD System: Force Visible Override ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    // Very far entity but force_visible
    auto* e = world.createEntity("lod_forced");
    auto* p = addComp<components::Position>(e);
    p->x = 200000.0f; p->y = 0.0f; p->z = 0.0f;
    auto* lod = addComp<components::LODPriority>(e);
    lod->force_visible = true;

    lodSys.update(1.0f);

    assertTrue(approxEqual(lod->priority, 2.0f),
               "Force-visible entity keeps full detail even at extreme range");
    assertTrue(lodSys.getFullDetailCount() == 1, "Force-visible counted as full detail");
}

static void testLODSystemDistanceQuery() {
    std::cout << "\n=== LOD System: Distance Query ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    auto* e = world.createEntity("lod_dist");
    auto* p = addComp<components::Position>(e);
    p->x = 3000.0f; p->y = 4000.0f; p->z = 0.0f;

    float distSq = lodSys.distanceSqToEntity("lod_dist");
    assertTrue(approxEqual(distSq, 25000000.0f), "Distance squared correct (3-4-5 triangle)");

    float noEntity = lodSys.distanceSqToEntity("nonexistent");
    assertTrue(noEntity < 0.0f, "Non-existent entity returns negative");
}


void run_lod_system_tests() {
    testLODPriorityDefaults();
    testLODPriorityPlayerShip();
    testLODSystemDefaults();
    testLODSystemPriorityComputation();
    testLODSystemForceVisible();
    testLODSystemDistanceQuery();
}
