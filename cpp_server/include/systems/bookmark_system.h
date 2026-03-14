#ifndef NOVAFORGE_SYSTEMS_BOOKMARK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BOOKMARK_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Space-location bookmark management system
 *
 * Allows players to save, organise, rename, and remove spatial bookmarks
 * grouped by folder.  Bookmarks carry 3-D coordinates, a system name, and
 * a type (Location, Container, Wreck, Station, Anomaly).  The bookmark list
 * is capped at max_bookmarks (default 500); inserts are rejected when
 * capacity is reached.  Lifetime counter total_bookmarks_created is
 * maintained independently.
 */
class BookmarkSystem
    : public ecs::SingleComponentSystem<components::BookmarkState> {
public:
    explicit BookmarkSystem(ecs::World* world);
    ~BookmarkSystem() override = default;

    std::string getName() const override { return "BookmarkSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Bookmark management ---
    bool addBookmark(const std::string& entity_id,
                     const std::string& bookmark_id,
                     const std::string& label,
                     components::BookmarkState::BookmarkType type,
                     float x, float y, float z,
                     const std::string& system_name,
                     const std::string& folder);
    bool removeBookmark(const std::string& entity_id,
                        const std::string& bookmark_id);
    bool renameBookmark(const std::string& entity_id,
                        const std::string& bookmark_id,
                        const std::string& new_label);
    bool moveToFolder(const std::string& entity_id,
                      const std::string& bookmark_id,
                      const std::string& folder);
    bool clearFolder(const std::string& entity_id,
                     const std::string& folder);
    bool clearAll(const std::string& entity_id);

    // --- Configuration ---
    bool setMaxBookmarks(const std::string& entity_id, int max_bookmarks);

    // --- Queries ---
    int  getBookmarkCount(const std::string& entity_id) const;
    int  getBookmarkCountInFolder(const std::string& entity_id,
                                  const std::string& folder) const;
    bool hasBookmark(const std::string& entity_id,
                     const std::string& bookmark_id) const;
    std::string getBookmarkLabel(const std::string& entity_id,
                                 const std::string& bookmark_id) const;
    std::string getBookmarkFolder(const std::string& entity_id,
                                  const std::string& bookmark_id) const;
    std::vector<std::string> getFolderNames(const std::string& entity_id) const;
    int  getTotalBookmarksCreated(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::BookmarkState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BOOKMARK_SYSTEM_H
