// Tests for: NavigationBookmark System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/navigation_bookmark_system.h"

using namespace atlas;

// ==================== NavigationBookmark System Tests ====================

static void testNavBookmarkCreate() {
    std::cout << "\n=== NavBookmark: Create ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeBookmarks("player1"), "Init bookmarks succeeds");
    assertTrue(sys.getBookmarkCount("player1") == 0, "No bookmarks initially");
    assertTrue(sys.getFavoriteCount("player1") == 0, "No favorites initially");
}

static void testNavBookmarkAdd() {
    std::cout << "\n=== NavBookmark: Add ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    assertTrue(sys.addBookmark("player1", "bm1", "Mining Spot", "system_alpha", 100.0f, 200.0f, 300.0f), "Add bookmark succeeds");
    assertTrue(sys.getBookmarkCount("player1") == 1, "1 bookmark");
    assertTrue(sys.getLabel("player1", "bm1") == "Mining Spot", "Label is Mining Spot");
}

static void testNavBookmarkDuplicate() {
    std::cout << "\n=== NavBookmark: Duplicate ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Spot A", "sys1", 0, 0, 0);
    assertTrue(!sys.addBookmark("player1", "bm1", "Spot B", "sys2", 1, 1, 1), "Duplicate ID rejected");
    assertTrue(sys.getBookmarkCount("player1") == 1, "Still 1 bookmark");
}

static void testNavBookmarkRemove() {
    std::cout << "\n=== NavBookmark: Remove ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Spot A", "sys1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "Spot B", "sys2", 1, 1, 1);
    assertTrue(sys.removeBookmark("player1", "bm1"), "Remove succeeds");
    assertTrue(sys.getBookmarkCount("player1") == 1, "1 bookmark remains");
    assertTrue(!sys.removeBookmark("player1", "bm1"), "Remove nonexistent fails");
}

static void testNavBookmarkCategory() {
    std::cout << "\n=== NavBookmark: Category ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Spot A", "sys1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "Spot B", "sys2", 1, 1, 1);
    assertTrue(sys.getCategoryCount("player1", "Personal") == 2, "Both default to Personal");
    assertTrue(sys.setCategory("player1", "bm1", "Corp"), "Set category succeeds");
    assertTrue(sys.getCategoryCount("player1", "Corp") == 1, "1 Corp bookmark");
    assertTrue(sys.getCategoryCount("player1", "Personal") == 1, "1 Personal bookmark");
}

static void testNavBookmarkNotes() {
    std::cout << "\n=== NavBookmark: Notes ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Asteroid Belt", "sys1", 0, 0, 0);
    assertTrue(sys.setNotes("player1", "bm1", "Rich in Ferrite"), "Set notes succeeds");
    assertTrue(!sys.setNotes("player1", "bm_bad", "test"), "Set notes on nonexistent fails");
}

static void testNavBookmarkFavorite() {
    std::cout << "\n=== NavBookmark: Favorite ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Home", "sys1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "Mine", "sys2", 1, 1, 1);
    assertTrue(sys.getFavoriteCount("player1") == 0, "No favorites yet");
    assertTrue(sys.toggleFavorite("player1", "bm1"), "Toggle favorite succeeds");
    assertTrue(sys.getFavoriteCount("player1") == 1, "1 favorite");
    assertTrue(sys.toggleFavorite("player1", "bm1"), "Toggle back succeeds");
    assertTrue(sys.getFavoriteCount("player1") == 0, "0 favorites after untoggle");
}

static void testNavBookmarkMaxLimit() {
    std::cout << "\n=== NavBookmark: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    // Set a low max for testing
    auto* entity = world.getEntity("player1");
    entity->getComponent<components::NavigationBookmark>()->max_bookmarks = 3;
    sys.addBookmark("player1", "bm1", "A", "s1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "B", "s2", 0, 0, 0);
    sys.addBookmark("player1", "bm3", "C", "s3", 0, 0, 0);
    assertTrue(!sys.addBookmark("player1", "bm4", "D", "s4", 0, 0, 0), "Max limit enforced");
    assertTrue(sys.getBookmarkCount("player1") == 3, "Still 3 bookmarks");
}

static void testNavBookmarkMultiple() {
    std::cout << "\n=== NavBookmark: Multiple ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Station Alpha", "sys1", 10, 20, 30);
    sys.addBookmark("player1", "bm2", "Asteroid Belt", "sys1", 40, 50, 60);
    sys.addBookmark("player1", "bm3", "Pirate Zone", "sys2", 70, 80, 90);
    assertTrue(sys.getBookmarkCount("player1") == 3, "3 bookmarks added");
    sys.setCategory("player1", "bm3", "Shared");
    sys.toggleFavorite("player1", "bm1");
    sys.toggleFavorite("player1", "bm2");
    assertTrue(sys.getFavoriteCount("player1") == 2, "2 favorites");
    assertTrue(sys.getCategoryCount("player1", "Shared") == 1, "1 shared");
}

static void testNavBookmarkMissing() {
    std::cout << "\n=== NavBookmark: Missing ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    assertTrue(!sys.initializeBookmarks("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.addBookmark("nonexistent", "bm1", "A", "s1", 0, 0, 0), "Add fails on missing");
    assertTrue(!sys.removeBookmark("nonexistent", "bm1"), "Remove fails on missing");
    assertTrue(!sys.setCategory("nonexistent", "bm1", "Corp"), "SetCategory fails on missing");
    assertTrue(!sys.toggleFavorite("nonexistent", "bm1"), "Toggle fails on missing");
    assertTrue(sys.getBookmarkCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getFavoriteCount("nonexistent") == 0, "0 favorites on missing");
    assertTrue(sys.getCategoryCount("nonexistent", "Personal") == 0, "0 category on missing");
    assertTrue(sys.getLabel("nonexistent", "bm1") == "", "Empty label on missing");
}


void run_navigation_bookmark_system_tests() {
    testNavBookmarkCreate();
    testNavBookmarkAdd();
    testNavBookmarkDuplicate();
    testNavBookmarkRemove();
    testNavBookmarkCategory();
    testNavBookmarkNotes();
    testNavBookmarkFavorite();
    testNavBookmarkMaxLimit();
    testNavBookmarkMultiple();
    testNavBookmarkMissing();
}
