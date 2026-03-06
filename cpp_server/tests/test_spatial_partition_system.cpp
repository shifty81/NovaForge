// Tests for: Spatial Partition System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/spatial_partition_system.h"

using namespace atlas;

// ==================== Spatial Partition System Tests ====================

static void testSpatialPartitionCreate() {
    std::cout << "\n=== SpatialPartition: Create ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    assertTrue(sys.initialize("grid1", 1000.0f), "Init succeeds");
    assertTrue(sys.getEntityCount("grid1") == 0, "No entities initially");
    assertTrue(sys.getTotalQueries("grid1") == 0, "No queries initially");
    assertTrue(sys.getTotalRebuilds("grid1") == 0, "No rebuilds initially");
}

static void testSpatialPartitionInsert() {
    std::cout << "\n=== SpatialPartition: Insert ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    assertTrue(sys.insertEntity("grid1", "ship1", 500.0f, 0.0f, 0.0f), "Insert ship1");
    assertTrue(sys.insertEntity("grid1", "ship2", 1500.0f, 0.0f, 0.0f), "Insert ship2");
    assertTrue(sys.getEntityCount("grid1") == 2, "2 entities");
    assertTrue(!sys.insertEntity("grid1", "ship1", 0.0f, 0.0f, 0.0f), "Duplicate rejected");
}

static void testSpatialPartitionRemove() {
    std::cout << "\n=== SpatialPartition: Remove ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    sys.insertEntity("grid1", "ship1", 0.0f, 0.0f, 0.0f);
    assertTrue(sys.removeEntity("grid1", "ship1"), "Remove succeeds");
    assertTrue(sys.getEntityCount("grid1") == 0, "0 entities after remove");
    assertTrue(!sys.removeEntity("grid1", "ship1"), "Double remove fails");
}

static void testSpatialPartitionCellAssignment() {
    std::cout << "\n=== SpatialPartition: CellAssignment ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    // Cell (0,0,0) for position (500, 500, 500)
    sys.insertEntity("grid1", "a", 500.0f, 500.0f, 500.0f);
    // Cell (1,0,0) for position (1500, 0, 0)
    sys.insertEntity("grid1", "b", 1500.0f, 0.0f, 0.0f);
    // Cell (0,0,0) for position (999, 0, 0)
    sys.insertEntity("grid1", "c", 999.0f, 0.0f, 0.0f);
    assertTrue(sys.getCellEntityCount("grid1", 0, 0, 0) == 2, "2 entities in cell (0,0,0)");
    assertTrue(sys.getCellEntityCount("grid1", 1, 0, 0) == 1, "1 entity in cell (1,0,0)");
    assertTrue(sys.getCellEntityCount("grid1", 2, 2, 2) == 0, "0 entities in empty cell");
}

static void testSpatialPartitionRadiusQuery() {
    std::cout << "\n=== SpatialPartition: RadiusQuery ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    sys.insertEntity("grid1", "a", 0.0f, 0.0f, 0.0f);
    sys.insertEntity("grid1", "b", 100.0f, 0.0f, 0.0f);
    sys.insertEntity("grid1", "c", 5000.0f, 0.0f, 0.0f);
    assertTrue(sys.getEntitiesInRadiusCount("grid1", 0.0f, 0.0f, 0.0f, 200.0f) == 2, "2 in radius 200");
    assertTrue(sys.getEntitiesInRadiusCount("grid1", 0.0f, 0.0f, 0.0f, 50.0f) == 1, "1 in radius 50");
    assertTrue(sys.getEntitiesInRadiusCount("grid1", 0.0f, 0.0f, 0.0f, 10000.0f) == 3, "3 in radius 10000");
}

static void testSpatialPartitionUpdatePosition() {
    std::cout << "\n=== SpatialPartition: UpdatePosition ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    sys.insertEntity("grid1", "ship1", 500.0f, 0.0f, 0.0f);
    assertTrue(sys.getCellEntityCount("grid1", 0, 0, 0) == 1, "In cell (0,0,0) initially");
    assertTrue(sys.updatePosition("grid1", "ship1", 1500.0f, 0.0f, 0.0f), "Update position succeeds");
    assertTrue(sys.getCellEntityCount("grid1", 0, 0, 0) == 0, "0 in old cell");
    assertTrue(sys.getCellEntityCount("grid1", 1, 0, 0) == 1, "1 in new cell");
    assertTrue(!sys.updatePosition("grid1", "nonexistent", 0.0f, 0.0f, 0.0f), "Missing entity fails");
}

static void testSpatialPartitionRebuild() {
    std::cout << "\n=== SpatialPartition: Rebuild ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    auto* entity = world.getEntity("grid1");
    auto* sp = entity->getComponent<components::SpatialPartition>();
    sp->rebuild_interval = 2.0f;
    sys.insertEntity("grid1", "ship1", 500.0f, 0.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(sys.getTotalRebuilds("grid1") == 0, "No rebuild at 1s");
    sys.update(1.5f);
    assertTrue(sys.getTotalRebuilds("grid1") == 1, "1 rebuild at 2.5s");
    sys.update(2.0f);
    assertTrue(sys.getTotalRebuilds("grid1") == 2, "2 rebuilds at 4.5s");
}

static void testSpatialPartitionMaxEntries() {
    std::cout << "\n=== SpatialPartition: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    auto* entity = world.getEntity("grid1");
    auto* sp = entity->getComponent<components::SpatialPartition>();
    sp->max_entries = 2;
    sys.insertEntity("grid1", "a", 0.0f, 0.0f, 0.0f);
    sys.insertEntity("grid1", "b", 100.0f, 0.0f, 0.0f);
    assertTrue(!sys.insertEntity("grid1", "c", 200.0f, 0.0f, 0.0f), "Max entries enforced");
}

static void testSpatialPartitionNegativeCoords() {
    std::cout << "\n=== SpatialPartition: NegativeCoords ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    sys.insertEntity("grid1", "a", -500.0f, -500.0f, -500.0f);
    sys.insertEntity("grid1", "b", -1500.0f, 0.0f, 0.0f);
    assertTrue(sys.getCellEntityCount("grid1", -1, -1, -1) == 1, "1 entity in cell (-1,-1,-1)");
    assertTrue(sys.getCellEntityCount("grid1", -2, 0, 0) == 1, "1 entity in cell (-2,0,0)");
}

static void testSpatialPartitionQueryCounter() {
    std::cout << "\n=== SpatialPartition: QueryCounter ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    world.createEntity("grid1");
    sys.initialize("grid1", 1000.0f);
    sys.insertEntity("grid1", "a", 0.0f, 0.0f, 0.0f);
    sys.getEntitiesInCell("grid1", 0, 0, 0);
    sys.getEntitiesInRadius("grid1", 0.0f, 0.0f, 0.0f, 100.0f);
    sys.getEntitiesInRadiusCount("grid1", 0.0f, 0.0f, 0.0f, 100.0f);
    assertTrue(sys.getTotalQueries("grid1") == 3, "3 queries tracked");
}

static void testSpatialPartitionMissing() {
    std::cout << "\n=== SpatialPartition: Missing ===" << std::endl;
    ecs::World world;
    systems::SpatialPartitionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 1000.0f), "Init fails on missing");
    assertTrue(!sys.insertEntity("nonexistent", "a", 0, 0, 0), "Insert fails on missing");
    assertTrue(!sys.removeEntity("nonexistent", "a"), "Remove fails on missing");
    assertTrue(!sys.updatePosition("nonexistent", "a", 0, 0, 0), "UpdatePosition fails on missing");
    assertTrue(sys.getEntityCount("nonexistent") == 0, "0 entities on missing");
    assertTrue(sys.getEntitiesInCell("nonexistent", 0, 0, 0).empty(), "Empty cell query on missing");
    assertTrue(sys.getEntitiesInRadius("nonexistent", 0, 0, 0, 100).empty(), "Empty radius query on missing");
    assertTrue(sys.getEntitiesInRadiusCount("nonexistent", 0, 0, 0, 100) == 0, "0 radius count on missing");
    assertTrue(sys.getCellEntityCount("nonexistent", 0, 0, 0) == 0, "0 cell count on missing");
    assertTrue(sys.getTotalQueries("nonexistent") == 0, "0 queries on missing");
    assertTrue(sys.getTotalRebuilds("nonexistent") == 0, "0 rebuilds on missing");
}


void run_spatial_partition_system_tests() {
    testSpatialPartitionCreate();
    testSpatialPartitionInsert();
    testSpatialPartitionRemove();
    testSpatialPartitionCellAssignment();
    testSpatialPartitionRadiusQuery();
    testSpatialPartitionUpdatePosition();
    testSpatialPartitionRebuild();
    testSpatialPartitionMaxEntries();
    testSpatialPartitionNegativeCoords();
    testSpatialPartitionQueryCounter();
    testSpatialPartitionMissing();
}
