#pragma once
/**
 * @file scene_bookmark_manager.h
 * @brief Manager for saving and restoring named scene-view bookmarks.
 *
 * SceneBookmarkManager captures the current camera position, look
 * direction, and an optional set of selected entity IDs so the
 * designer can quickly jump back to a saved viewpoint during editing.
 * Bookmarks are lightweight and independent of the DeltaEditStore —
 * they are purely editor-side convenience state.
 */

#include <cstdint>
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * A single scene bookmark: camera state + optional selection.
 */
struct SceneBookmark {
    std::string label;
    float camX = 0.0f, camY = 0.0f, camZ = 0.0f;
    float lookX = 0.0f, lookY = 0.0f, lookZ = -1.0f;
    std::vector<uint32_t> selectedEntities;
};

/**
 * SceneBookmarkManager — stores named camera/selection bookmarks.
 *
 * Usage:
 *   SceneBookmarkManager mgr;
 *   mgr.Save("Bridge view", 10, 20, 30, 0, 0, -1, {1, 2, 3});
 *   // ... navigate elsewhere ...
 *   const auto& bm = mgr.Get(0);
 *   // restore camera to bm.camX, camY, camZ ...
 */
class SceneBookmarkManager {
public:
    /** Save a new bookmark with the given camera state and selection. */
    void Save(const std::string& label,
              float camX, float camY, float camZ,
              float lookX, float lookY, float lookZ,
              const std::vector<uint32_t>& selectedEntities = {}) {
        SceneBookmark bm;
        bm.label    = label;
        bm.camX     = camX;
        bm.camY     = camY;
        bm.camZ     = camZ;
        bm.lookX    = lookX;
        bm.lookY    = lookY;
        bm.lookZ    = lookZ;
        bm.selectedEntities = selectedEntities;
        m_bookmarks.push_back(std::move(bm));
    }

    /** Number of stored bookmarks. */
    size_t Count() const { return m_bookmarks.size(); }

    /** Retrieve a bookmark by index.  Returns nullptr if out of range. */
    const SceneBookmark* Get(size_t index) const {
        if (index >= m_bookmarks.size()) return nullptr;
        return &m_bookmarks[index];
    }

    /** Remove a bookmark by index.  Returns false if out of range. */
    bool Remove(size_t index) {
        if (index >= m_bookmarks.size()) return false;
        m_bookmarks.erase(m_bookmarks.begin() + static_cast<ptrdiff_t>(index));
        return true;
    }

    /** Update an existing bookmark's label. Returns false if out of range. */
    bool Rename(size_t index, const std::string& newLabel) {
        if (index >= m_bookmarks.size()) return false;
        m_bookmarks[index].label = newLabel;
        return true;
    }

    /** Clear all bookmarks. */
    void Clear() { m_bookmarks.clear(); }

    /** Read-only access to all bookmarks. */
    const std::vector<SceneBookmark>& Bookmarks() const { return m_bookmarks; }

private:
    std::vector<SceneBookmark> m_bookmarks;
};

} // namespace atlas::editor
