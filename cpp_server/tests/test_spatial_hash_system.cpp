// Tests for: Phase 5 Continued: Spatial Hash System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/spatial_hash_system.h"

using namespace atlas;

// ==================== Phase 5 Continued: Spatial Hash System Tests ====================

static void testSpatialHashBasicIndex() {
    std::cout << "\n=== Spatial Hash: Basic Indexing ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);
    spatial.setCellSize(1000.0f);

    auto* e1 = world.createEntity("sh_e1");
    auto* p1 = addComp<components::Position>(e1);
    p1->x = 500.0f; p1->y = 0.0f; p1->z = 0.0f;

    auto* e2 = world.createEntity("sh_e2");
    auto* p2 = addComp<components::Position>(e2);
    p2->x = 600.0f; p2->y = 0.0f; p2->z = 0.0f;

    auto* e3 = world.createEntity("sh_e3");
    auto* p3 = addComp<components::Position>(e3);
    p3->x = 5000.0f; p3->y = 5000.0f; p3->z = 5000.0f;

    spatial.update(0.0f);

    assertTrue(spatial.getIndexedEntityCount() == 3, "3 entities indexed");
    assertTrue(spatial.getOccupiedCellCount() >= 1, "At least 1 cell occupied");
}

static void testSpatialHashQueryNear() {
    std::cout << "\n=== Spatial Hash: Query Near ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);
    spatial.setCellSize(1000.0f);

    // Two close entities, one far away
    auto* e1 = world.createEntity("near_a");
    auto* p1 = addComp<components::Position>(e1);
    p1->x = 100.0f; p1->y = 0.0f; p1->z = 0.0f;

    auto* e2 = world.createEntity("near_b");
    auto* p2 = addComp<components::Position>(e2);
    p2->x = 200.0f; p2->y = 0.0f; p2->z = 0.0f;

    auto* e3 = world.createEntity("far_c");
    auto* p3 = addComp<components::Position>(e3);
    p3->x = 50000.0f; p3->y = 0.0f; p3->z = 0.0f;

    spatial.update(0.0f);

    auto nearby = spatial.queryNear(150.0f, 0.0f, 0.0f, 500.0f);
    assertTrue(nearby.size() == 2, "Two entities within 500m of query point");

    auto farNearby = spatial.queryNear(90.0f, 0.0f, 0.0f, 30.0f);
    assertTrue(farNearby.size() == 1, "One entity within 30m of offset query point");
}

static void testSpatialHashQueryNeighbours() {
    std::cout << "\n=== Spatial Hash: Query Neighbours ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);
    spatial.setCellSize(1000.0f);

    // Two entities in same cell
    auto* e1 = world.createEntity("nb_a");
    auto* p1 = addComp<components::Position>(e1);
    p1->x = 100.0f; p1->y = 100.0f; p1->z = 100.0f;

    auto* e2 = world.createEntity("nb_b");
    auto* p2 = addComp<components::Position>(e2);
    p2->x = 200.0f; p2->y = 200.0f; p2->z = 200.0f;

    // Entity far away (different cell neighbourhood)
    auto* e3 = world.createEntity("nb_far");
    auto* p3 = addComp<components::Position>(e3);
    p3->x = 50000.0f; p3->y = 50000.0f; p3->z = 50000.0f;

    spatial.update(0.0f);

    auto neighbours = spatial.queryNeighbours("nb_a");
    bool foundB = false;
    bool foundFar = false;
    for (const auto& id : neighbours) {
        if (id == "nb_b") foundB = true;
        if (id == "nb_far") foundFar = true;
    }
    assertTrue(foundB, "Same-cell neighbour found");
    assertTrue(!foundFar, "Far entity not in neighbour set");
}

static void testSpatialHashEmptyWorld() {
    std::cout << "\n=== Spatial Hash: Empty World ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);

    spatial.update(0.0f);

    assertTrue(spatial.getIndexedEntityCount() == 0, "Empty world has 0 indexed entities");
    assertTrue(spatial.getOccupiedCellCount() == 0, "Empty world has 0 occupied cells");

    auto nearby = spatial.queryNear(0.0f, 0.0f, 0.0f, 1000.0f);
    assertTrue(nearby.empty(), "No results from empty world");
}

static void testSpatialHashCellSizeConfig() {
    std::cout << "\n=== Spatial Hash: Cell Size Configuration ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);

    spatial.setCellSize(500.0f);
    assertTrue(approxEqual(spatial.getCellSize(), 500.0f), "Cell size set to 500");

    // Negative size should be rejected
    spatial.setCellSize(-100.0f);
    assertTrue(approxEqual(spatial.getCellSize(), 500.0f), "Negative cell size rejected");
}


void run_spatial_hash_system_tests() {
    testSpatialHashBasicIndex();
    testSpatialHashQueryNear();
    testSpatialHashQueryNeighbours();
    testSpatialHashEmptyWorld();
    testSpatialHashCellSizeConfig();
}
