// Tests for: BookmarkSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/bookmark_system.h"

using namespace atlas;

// ==================== BookmarkSystem Tests ====================

static void testBookmarkInit() {
    std::cout << "\n=== Bookmark: Init ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getBookmarkCount("e1") == 0, "Zero bookmarks initially");
    assertTrue(sys.getTotalBookmarksCreated("e1") == 0, "Zero total created");
    assertTrue(sys.getFolderNames("e1").empty(), "No folders initially");
    assertTrue(!sys.hasBookmark("e1", "bm1"), "No bookmark yet");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testBookmarkAdd() {
    std::cout << "\n=== Bookmark: Add ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    assertTrue(sys.addBookmark("e1", "bm1", "Safe Spot Alpha", BT::Location,
               100.0f, 200.0f, 300.0f, "Jita", "PvE"), "Add bookmark");
    assertTrue(sys.getBookmarkCount("e1") == 1, "1 bookmark");
    assertTrue(sys.hasBookmark("e1", "bm1"), "Has bm1");
    assertTrue(sys.getBookmarkLabel("e1", "bm1") == "Safe Spot Alpha", "Label matches");
    assertTrue(sys.getBookmarkFolder("e1", "bm1") == "PvE", "Folder matches");
    assertTrue(sys.getTotalBookmarksCreated("e1") == 1, "1 total created");
    assertTrue(sys.addBookmark("e1", "bm2", "Wreck Site", BT::Wreck,
               50.0f, 60.0f, 70.0f, "Amarr", "Loot"), "Add second bookmark");
    assertTrue(sys.getBookmarkCount("e1") == 2, "2 bookmarks");
}

static void testBookmarkAddValidation() {
    std::cout << "\n=== Bookmark: AddValidation ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    assertTrue(!sys.addBookmark("e1", "", "Label", BT::Location,
               0,0,0, "Sys", "F"), "Empty id rejected");
    assertTrue(!sys.addBookmark("e1", "bm1", "", BT::Location,
               0,0,0, "Sys", "F"), "Empty label rejected");
    assertTrue(sys.addBookmark("e1", "bm1", "Ok", BT::Location,
               0,0,0, "Sys", "F"), "Valid add");
    assertTrue(!sys.addBookmark("e1", "bm1", "Dup", BT::Location,
               0,0,0, "Sys", "F"), "Duplicate id rejected");
    assertTrue(sys.getBookmarkCount("e1") == 1, "Still 1 bookmark");
    assertTrue(!sys.addBookmark("missing", "bm9", "X", BT::Location,
               0,0,0, "S", ""), "Missing entity rejected");
    // Empty folder is allowed
    assertTrue(sys.addBookmark("e1", "bm2", "Root", BT::Station,
               0,0,0, "S", ""), "Empty folder allowed");
    assertTrue(sys.getBookmarkFolder("e1", "bm2") == "", "Folder is empty string");
}

static void testBookmarkCapacity() {
    std::cout << "\n=== Bookmark: Capacity ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxBookmarks("e1", 3);

    using BT = components::BookmarkState::BookmarkType;
    assertTrue(sys.addBookmark("e1", "b1", "L1", BT::Location, 0,0,0, "S", ""), "Add 1");
    assertTrue(sys.addBookmark("e1", "b2", "L2", BT::Location, 0,0,0, "S", ""), "Add 2");
    assertTrue(sys.addBookmark("e1", "b3", "L3", BT::Location, 0,0,0, "S", ""), "Add 3 at cap");
    assertTrue(!sys.addBookmark("e1", "b4", "L4", BT::Location, 0,0,0, "S", ""),
               "Add 4 rejected at capacity");
    assertTrue(sys.getBookmarkCount("e1") == 3, "Still 3");
    assertTrue(sys.hasBookmark("e1", "b1"), "b1 still present (no purge)");
}

static void testBookmarkRemove() {
    std::cout << "\n=== Bookmark: Remove ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    sys.addBookmark("e1", "bm1", "L1", BT::Location, 0,0,0, "S", "F");
    sys.addBookmark("e1", "bm2", "L2", BT::Station, 0,0,0, "S", "F");

    assertTrue(sys.removeBookmark("e1", "bm1"), "Remove bm1");
    assertTrue(sys.getBookmarkCount("e1") == 1, "1 left");
    assertTrue(!sys.hasBookmark("e1", "bm1"), "bm1 gone");
    assertTrue(sys.hasBookmark("e1", "bm2"), "bm2 present");
    assertTrue(!sys.removeBookmark("e1", "bm1"), "Remove already removed fails");
    assertTrue(!sys.removeBookmark("e1", "unknown"), "Remove unknown fails");
}

static void testBookmarkRename() {
    std::cout << "\n=== Bookmark: Rename ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    sys.addBookmark("e1", "bm1", "OldName", BT::Location, 0,0,0, "S", "F");

    assertTrue(sys.renameBookmark("e1", "bm1", "NewName"), "Rename succeeds");
    assertTrue(sys.getBookmarkLabel("e1", "bm1") == "NewName", "Label updated");
    assertTrue(!sys.renameBookmark("e1", "bm1", ""), "Empty new label rejected");
    assertTrue(sys.getBookmarkLabel("e1", "bm1") == "NewName", "Label unchanged");
    assertTrue(!sys.renameBookmark("e1", "bm99", "X"), "Rename missing bookmark fails");
    assertTrue(!sys.renameBookmark("missing", "bm1", "X"), "Rename missing entity fails");
}

static void testBookmarkMoveToFolder() {
    std::cout << "\n=== Bookmark: MoveToFolder ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    sys.addBookmark("e1", "bm1", "L1", BT::Location, 0,0,0, "S", "Alpha");

    assertTrue(sys.getBookmarkFolder("e1", "bm1") == "Alpha", "Initial folder");
    assertTrue(sys.moveToFolder("e1", "bm1", "Beta"), "Move succeeds");
    assertTrue(sys.getBookmarkFolder("e1", "bm1") == "Beta", "Folder updated");
    assertTrue(sys.moveToFolder("e1", "bm1", ""), "Move to root succeeds");
    assertTrue(sys.getBookmarkFolder("e1", "bm1") == "", "Folder is empty");
    assertTrue(!sys.moveToFolder("e1", "bm99", "X"), "Move missing bookmark fails");
    assertTrue(!sys.moveToFolder("missing", "bm1", "X"), "Move missing entity fails");
}

static void testBookmarkClearFolder() {
    std::cout << "\n=== Bookmark: ClearFolder ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    sys.addBookmark("e1", "bm1", "L1", BT::Location, 0,0,0, "S", "Alpha");
    sys.addBookmark("e1", "bm2", "L2", BT::Location, 0,0,0, "S", "Alpha");
    sys.addBookmark("e1", "bm3", "L3", BT::Location, 0,0,0, "S", "Beta");
    sys.addBookmark("e1", "bm4", "L4", BT::Location, 0,0,0, "S", "Beta");

    assertTrue(sys.clearFolder("e1", "Alpha"), "ClearFolder Alpha");
    assertTrue(sys.getBookmarkCount("e1") == 2, "2 left after clear Alpha");
    assertTrue(!sys.hasBookmark("e1", "bm1"), "bm1 gone");
    assertTrue(!sys.hasBookmark("e1", "bm2"), "bm2 gone");
    assertTrue(sys.hasBookmark("e1", "bm3"), "bm3 still present");
    assertTrue(sys.hasBookmark("e1", "bm4"), "bm4 still present");
}

static void testBookmarkClearAll() {
    std::cout << "\n=== Bookmark: ClearAll ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    sys.addBookmark("e1", "bm1", "L1", BT::Location, 0,0,0, "S", "F");
    sys.addBookmark("e1", "bm2", "L2", BT::Station, 0,0,0, "S", "G");

    assertTrue(sys.clearAll("e1"), "ClearAll succeeds");
    assertTrue(sys.getBookmarkCount("e1") == 0, "0 bookmarks after clearAll");
    assertTrue(sys.getTotalBookmarksCreated("e1") == 2, "Total created preserved");
    assertTrue(!sys.hasBookmark("e1", "bm1"), "bm1 gone after clearAll");
}

static void testBookmarkFolderNames() {
    std::cout << "\n=== Bookmark: FolderNames ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using BT = components::BookmarkState::BookmarkType;
    sys.addBookmark("e1", "bm1", "L1", BT::Location, 0,0,0, "S", "Alpha");
    sys.addBookmark("e1", "bm2", "L2", BT::Location, 0,0,0, "S", "Alpha");
    sys.addBookmark("e1", "bm3", "L3", BT::Location, 0,0,0, "S", "Beta");
    sys.addBookmark("e1", "bm4", "L4", BT::Location, 0,0,0, "S", "");  // root

    auto folders = sys.getFolderNames("e1");
    assertTrue(folders.size() == 2, "2 unique non-empty folders");
    // set-based — sorted alphabetically
    bool hasAlpha = false, hasBeta = false;
    for (const auto& f : folders) {
        if (f == "Alpha") hasAlpha = true;
        if (f == "Beta") hasBeta = true;
    }
    assertTrue(hasAlpha, "Contains Alpha");
    assertTrue(hasBeta, "Contains Beta");
    assertTrue(sys.getBookmarkCountInFolder("e1", "Alpha") == 2, "2 in Alpha");
    assertTrue(sys.getBookmarkCountInFolder("e1", "") == 1, "1 in root");
}

static void testBookmarkMissing() {
    std::cout << "\n=== Bookmark: Missing ===" << std::endl;
    ecs::World world;
    systems::BookmarkSystem sys(&world);

    using BT = components::BookmarkState::BookmarkType;
    assertTrue(!sys.addBookmark("none", "bm1", "L", BT::Location,
               0,0,0, "S", "F"), "Add fails on missing");
    assertTrue(!sys.removeBookmark("none", "bm1"), "Remove fails on missing");
    assertTrue(!sys.renameBookmark("none", "bm1", "X"), "Rename fails on missing");
    assertTrue(!sys.moveToFolder("none", "bm1", "X"), "MoveToFolder fails on missing");
    assertTrue(!sys.clearFolder("none", "F"), "ClearFolder fails on missing");
    assertTrue(!sys.clearAll("none"), "ClearAll fails on missing");
    assertTrue(!sys.setMaxBookmarks("none", 10), "SetMax fails on missing");
    assertTrue(sys.getBookmarkCount("none") == 0, "0 count on missing");
    assertTrue(sys.getBookmarkCountInFolder("none", "F") == 0, "0 folder count on missing");
    assertTrue(!sys.hasBookmark("none", "bm1"), "No bookmark on missing");
    assertTrue(sys.getBookmarkLabel("none", "bm1") == "", "Empty label on missing");
    assertTrue(sys.getBookmarkFolder("none", "bm1") == "", "Empty folder on missing");
    assertTrue(sys.getFolderNames("none").empty(), "No folders on missing");
    assertTrue(sys.getTotalBookmarksCreated("none") == 0, "0 total on missing");
    assertTrue(!sys.setMaxBookmarks("none", 0), "SetMax 0 fails");
}

void run_bookmark_system_tests() {
    testBookmarkInit();
    testBookmarkAdd();
    testBookmarkAddValidation();
    testBookmarkCapacity();
    testBookmarkRemove();
    testBookmarkRename();
    testBookmarkMoveToFolder();
    testBookmarkClearFolder();
    testBookmarkClearAll();
    testBookmarkFolderNames();
    testBookmarkMissing();
}
