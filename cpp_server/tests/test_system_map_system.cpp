// Tests for: SystemMapSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/system_map_system.h"

using namespace atlas;

// ==================== SystemMapSystem Tests ====================

static void testMapAddCelestial() {
    std::cout << "\n=== SystemMap: Add Celestial ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    assertTrue(sys.addCelestial("system_1", "star_1", "Sol", "Star", 0.0f, 0.0f, 0.0f, 6960.0f),
               "Add star succeeds");
    assertTrue(sys.getCelestialCount("system_1") == 1, "1 celestial");
    assertTrue(!sys.addCelestial("system_1", "star_1", "Sol2", "Star", 0.0f, 0.0f, 0.0f, 100.0f),
               "Duplicate rejected");
}

static void testMapRemoveCelestial() {
    std::cout << "\n=== SystemMap: Remove Celestial ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    sys.addCelestial("system_1", "planet_1", "Earth", "Planet", 1000.0f, 0.0f, 0.0f, 63.7f);
    assertTrue(sys.removeCelestial("system_1", "planet_1"), "Remove succeeds");
    assertTrue(sys.getCelestialCount("system_1") == 0, "0 celestials");
    assertTrue(!sys.removeCelestial("system_1", "planet_1"), "Remove nonexistent fails");
}

static void testMapAddBookmark() {
    std::cout << "\n=== SystemMap: Add Bookmark ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    assertTrue(sys.addBookmark("system_1", "bm_1", "Safe Spot", "Combat", 500.0f, 200.0f, 100.0f),
               "Add bookmark succeeds");
    assertTrue(sys.getBookmarkCount("system_1") == 1, "1 bookmark");
    assertTrue(sys.getTotalBookmarksCreated("system_1") == 1, "1 total created");

    assertTrue(!sys.addBookmark("system_1", "bm_1", "Dup", "Combat", 0.0f, 0.0f, 0.0f),
               "Duplicate rejected");
}

static void testMapRemoveBookmark() {
    std::cout << "\n=== SystemMap: Remove Bookmark ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    sys.addBookmark("system_1", "bm_1", "Safe Spot", "Combat", 100.0f, 0.0f, 0.0f);
    assertTrue(sys.removeBookmark("system_1", "bm_1"), "Remove succeeds");
    assertTrue(sys.getBookmarkCount("system_1") == 0, "0 bookmarks");
    // total_bookmarks_created doesn't decrease
    assertTrue(sys.getTotalBookmarksCreated("system_1") == 1, "Total created still 1");
}

static void testMapAddSignature() {
    std::cout << "\n=== SystemMap: Add Signature ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    assertTrue(sys.addSignature("system_1", "sig_1", "Anomaly", 0.5f, 300.0f, 400.0f, 0.0f),
               "Add signature succeeds");
    assertTrue(sys.getSignatureCount("system_1") == 1, "1 signature");
}

static void testMapResolveSignature() {
    std::cout << "\n=== SystemMap: Resolve Signature ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    sys.addSignature("system_1", "sig_1", "Wormhole", 0.3f, 100.0f, 200.0f, 300.0f);
    assertTrue(sys.resolveSignature("system_1", "sig_1"), "Resolve succeeds");

    auto* map = e->getComponent<components::SystemMap>();
    assertTrue(map->signatures[0].resolved, "Signature marked resolved");
    assertTrue(approxEqual(map->signatures[0].scan_strength, 1.0f), "Scan strength set to 1.0");
}

static void testMapDistanceBetween() {
    std::cout << "\n=== SystemMap: Distance Between ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    sys.addCelestial("system_1", "star_1", "Sol", "Star", 0.0f, 0.0f, 0.0f, 100.0f);
    sys.addCelestial("system_1", "planet_1", "Earth", "Planet", 300.0f, 400.0f, 0.0f, 10.0f);

    float dist = sys.getDistanceBetween("system_1", "star_1", "planet_1");
    assertTrue(approxEqual(dist, 500.0f), "Distance is 500 (3-4-5 triangle)");
}

static void testMapDistanceCrossType() {
    std::cout << "\n=== SystemMap: Distance Cross Type ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    addComp<components::SystemMap>(e);

    sys.addCelestial("system_1", "gate_1", "Jump Gate", "Gate", 0.0f, 0.0f, 0.0f, 5.0f);
    sys.addBookmark("system_1", "bm_1", "Warp To", "Travel", 100.0f, 0.0f, 0.0f);

    float dist = sys.getDistanceBetween("system_1", "gate_1", "bm_1");
    assertTrue(approxEqual(dist, 100.0f), "Distance between celestial and bookmark");
}

static void testMapMaxCapacity() {
    std::cout << "\n=== SystemMap: Max Capacity ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    auto* map = addComp<components::SystemMap>(e);
    map->max_celestials = 2;
    map->max_bookmarks = 2;
    map->max_signatures = 2;

    sys.addCelestial("system_1", "c1", "A", "Star", 0.0f, 0.0f, 0.0f, 1.0f);
    sys.addCelestial("system_1", "c2", "B", "Planet", 1.0f, 0.0f, 0.0f, 1.0f);
    assertTrue(!sys.addCelestial("system_1", "c3", "C", "Moon", 2.0f, 0.0f, 0.0f, 1.0f),
               "Celestial max enforced");

    sys.addBookmark("system_1", "b1", "A", "F", 0.0f, 0.0f, 0.0f);
    sys.addBookmark("system_1", "b2", "B", "F", 1.0f, 0.0f, 0.0f);
    assertTrue(!sys.addBookmark("system_1", "b3", "C", "F", 2.0f, 0.0f, 0.0f),
               "Bookmark max enforced");

    sys.addSignature("system_1", "s1", "Anomaly", 0.5f, 0.0f, 0.0f, 0.0f);
    sys.addSignature("system_1", "s2", "Wormhole", 0.3f, 1.0f, 0.0f, 0.0f);
    assertTrue(!sys.addSignature("system_1", "s3", "Relic", 0.1f, 2.0f, 0.0f, 0.0f),
               "Signature max enforced");
}

static void testMapMissingEntity() {
    std::cout << "\n=== SystemMap: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    assertTrue(!sys.addCelestial("nope", "c1", "A", "Star", 0.0f, 0.0f, 0.0f, 1.0f), "Add fails");
    assertTrue(!sys.addBookmark("nope", "b1", "A", "F", 0.0f, 0.0f, 0.0f), "Bookmark fails");
    assertTrue(!sys.addSignature("nope", "s1", "A", 0.5f, 0.0f, 0.0f, 0.0f), "Sig fails");
    assertTrue(sys.getCelestialCount("nope") == 0, "0 celestials");
    assertTrue(sys.getBookmarkCount("nope") == 0, "0 bookmarks");
    assertTrue(sys.getSignatureCount("nope") == 0, "0 signatures");
    assertTrue(approxEqual(sys.getDistanceBetween("nope", "a", "b"), 0.0f), "0 distance");
}

static void testMapElapsedTime() {
    std::cout << "\n=== SystemMap: Elapsed Time ===" << std::endl;
    ecs::World world;
    systems::SystemMapSystem sys(&world);

    auto* e = world.createEntity("system_1");
    auto* map = addComp<components::SystemMap>(e);

    sys.update(5.0f);
    assertTrue(approxEqual(map->elapsed, 5.0f), "Elapsed is 5.0");

    sys.addBookmark("system_1", "bm_1", "Test", "Travel", 0.0f, 0.0f, 0.0f);
    assertTrue(approxEqual(map->bookmarks[0].created_at, 5.0f), "Bookmark timestamp is 5.0");
}

void run_system_map_system_tests() {
    testMapAddCelestial();
    testMapRemoveCelestial();
    testMapAddBookmark();
    testMapRemoveBookmark();
    testMapAddSignature();
    testMapResolveSignature();
    testMapDistanceBetween();
    testMapDistanceCrossType();
    testMapMaxCapacity();
    testMapMissingEntity();
    testMapElapsedTime();
}
