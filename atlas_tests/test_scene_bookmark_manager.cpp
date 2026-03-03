/**
 * Tests for SceneBookmarkManager:
 *   - Default state (empty)
 *   - Save and retrieve bookmarks
 *   - Multiple bookmarks
 *   - Remove bookmark
 *   - Rename bookmark
 *   - Clear all bookmarks
 *   - Out-of-bounds access returns nullptr / false
 *   - Selection entities preserved
 */

#include <cassert>
#include <string>
#include <vector>
#include "../cpp_client/include/editor/scene_bookmark_manager.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// SceneBookmarkManager tests
// ══════════════════════════════════════════════════════════════════

void test_bookmark_default_empty() {
    SceneBookmarkManager mgr;
    assert(mgr.Count() == 0);
    assert(mgr.Get(0) == nullptr);
}

void test_bookmark_save_and_get() {
    SceneBookmarkManager mgr;
    mgr.Save("Bridge view", 10.0f, 20.0f, 30.0f, 0.0f, 0.0f, -1.0f);

    assert(mgr.Count() == 1);
    const auto* bm = mgr.Get(0);
    assert(bm != nullptr);
    assert(bm->label == "Bridge view");
    assert(bm->camX == 10.0f);
    assert(bm->camY == 20.0f);
    assert(bm->camZ == 30.0f);
    assert(bm->lookX == 0.0f);
    assert(bm->lookY == 0.0f);
    assert(bm->lookZ == -1.0f);
}

void test_bookmark_multiple() {
    SceneBookmarkManager mgr;
    mgr.Save("View A", 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 0.0f);
    mgr.Save("View B", 4.0f, 5.0f, 6.0f, 1.0f, 0.0f, 0.0f);
    mgr.Save("View C", 7.0f, 8.0f, 9.0f, 0.0f, 0.0f, 1.0f);

    assert(mgr.Count() == 3);
    assert(mgr.Get(0)->label == "View A");
    assert(mgr.Get(1)->label == "View B");
    assert(mgr.Get(2)->label == "View C");
}

void test_bookmark_remove() {
    SceneBookmarkManager mgr;
    mgr.Save("View A", 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
    mgr.Save("View B", 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);

    assert(mgr.Remove(0));
    assert(mgr.Count() == 1);
    assert(mgr.Get(0)->label == "View B");
}

void test_bookmark_remove_out_of_bounds() {
    SceneBookmarkManager mgr;
    assert(!mgr.Remove(0));
    assert(!mgr.Remove(99));
}

void test_bookmark_rename() {
    SceneBookmarkManager mgr;
    mgr.Save("Old Name", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);

    assert(mgr.Rename(0, "New Name"));
    assert(mgr.Get(0)->label == "New Name");
}

void test_bookmark_rename_out_of_bounds() {
    SceneBookmarkManager mgr;
    assert(!mgr.Rename(0, "New Name"));
}

void test_bookmark_clear() {
    SceneBookmarkManager mgr;
    mgr.Save("View A", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
    mgr.Save("View B", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
    assert(mgr.Count() == 2);

    mgr.Clear();
    assert(mgr.Count() == 0);
    assert(mgr.Get(0) == nullptr);
}

void test_bookmark_selection_preserved() {
    SceneBookmarkManager mgr;
    std::vector<uint32_t> selection = {10, 20, 30};
    mgr.Save("With selection", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, selection);

    assert(mgr.Count() == 1);
    const auto* bm = mgr.Get(0);
    assert(bm->selectedEntities.size() == 3);
    assert(bm->selectedEntities[0] == 10);
    assert(bm->selectedEntities[1] == 20);
    assert(bm->selectedEntities[2] == 30);
}
