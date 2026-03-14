#include "systems/bookmark_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <set>

namespace atlas {
namespace systems {

BookmarkSystem::BookmarkSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void BookmarkSystem::updateComponent(ecs::Entity& /*entity*/,
                                      components::BookmarkState& comp,
                                      float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool BookmarkSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::BookmarkState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Bookmark management
// ---------------------------------------------------------------------------

bool BookmarkSystem::addBookmark(
        const std::string& entity_id,
        const std::string& bookmark_id,
        const std::string& label,
        components::BookmarkState::BookmarkType type,
        float x, float y, float z,
        const std::string& system_name,
        const std::string& folder) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (bookmark_id.empty()) return false;
    if (label.empty()) return false;

    // Duplicate check
    for (const auto& bm : comp->bookmarks) {
        if (bm.bookmark_id == bookmark_id) return false;
    }

    // Capacity check — reject, don't purge
    if (static_cast<int>(comp->bookmarks.size()) >= comp->max_bookmarks) {
        return false;
    }

    components::BookmarkState::Bookmark bm;
    bm.bookmark_id = bookmark_id;
    bm.label       = label;
    bm.type        = type;
    bm.x           = x;
    bm.y           = y;
    bm.z           = z;
    bm.system_name = system_name;
    bm.folder      = folder;
    comp->bookmarks.push_back(bm);

    comp->total_bookmarks_created++;
    return true;
}

bool BookmarkSystem::removeBookmark(const std::string& entity_id,
                                     const std::string& bookmark_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->bookmarks.begin(), comp->bookmarks.end(),
        [&](const components::BookmarkState::Bookmark& bm) {
            return bm.bookmark_id == bookmark_id;
        });
    if (it == comp->bookmarks.end()) return false;
    comp->bookmarks.erase(it);
    return true;
}

bool BookmarkSystem::renameBookmark(const std::string& entity_id,
                                     const std::string& bookmark_id,
                                     const std::string& new_label) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_label.empty()) return false;
    for (auto& bm : comp->bookmarks) {
        if (bm.bookmark_id == bookmark_id) {
            bm.label = new_label;
            return true;
        }
    }
    return false;
}

bool BookmarkSystem::moveToFolder(const std::string& entity_id,
                                   const std::string& bookmark_id,
                                   const std::string& folder) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& bm : comp->bookmarks) {
        if (bm.bookmark_id == bookmark_id) {
            bm.folder = folder;
            return true;
        }
    }
    return false;
}

bool BookmarkSystem::clearFolder(const std::string& entity_id,
                                  const std::string& folder) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->bookmarks.erase(
        std::remove_if(comp->bookmarks.begin(), comp->bookmarks.end(),
            [&](const components::BookmarkState::Bookmark& bm) {
                return bm.folder == folder;
            }),
        comp->bookmarks.end());
    return true;
}

bool BookmarkSystem::clearAll(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->bookmarks.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool BookmarkSystem::setMaxBookmarks(const std::string& entity_id,
                                      int max_bookmarks) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_bookmarks <= 0) return false;
    comp->max_bookmarks = max_bookmarks;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int BookmarkSystem::getBookmarkCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->bookmarks.size()) : 0;
}

int BookmarkSystem::getBookmarkCountInFolder(const std::string& entity_id,
                                              const std::string& folder) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& bm : comp->bookmarks) {
        if (bm.folder == folder) count++;
    }
    return count;
}

bool BookmarkSystem::hasBookmark(const std::string& entity_id,
                                  const std::string& bookmark_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& bm : comp->bookmarks) {
        if (bm.bookmark_id == bookmark_id) return true;
    }
    return false;
}

std::string BookmarkSystem::getBookmarkLabel(const std::string& entity_id,
                                              const std::string& bookmark_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& bm : comp->bookmarks) {
        if (bm.bookmark_id == bookmark_id) return bm.label;
    }
    return "";
}

std::string BookmarkSystem::getBookmarkFolder(const std::string& entity_id,
                                               const std::string& bookmark_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& bm : comp->bookmarks) {
        if (bm.bookmark_id == bookmark_id) return bm.folder;
    }
    return "";
}

std::vector<std::string> BookmarkSystem::getFolderNames(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return {};
    std::set<std::string> folders;
    for (const auto& bm : comp->bookmarks) {
        if (!bm.folder.empty()) folders.insert(bm.folder);
    }
    return {folders.begin(), folders.end()};
}

int BookmarkSystem::getTotalBookmarksCreated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_bookmarks_created : 0;
}

} // namespace systems
} // namespace atlas
