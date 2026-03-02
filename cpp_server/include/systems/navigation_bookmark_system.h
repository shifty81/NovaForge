#ifndef NOVAFORGE_SYSTEMS_NAVIGATION_BOOKMARK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NAVIGATION_BOOKMARK_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages player-saved location bookmarks for quick navigation
 *
 * Handles bookmark creation, removal, categorization, favorites,
 * and notes for saved navigation waypoints.
 */
class NavigationBookmarkSystem : public ecs::System {
public:
    explicit NavigationBookmarkSystem(ecs::World* world);
    ~NavigationBookmarkSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "NavigationBookmarkSystem"; }

    bool initializeBookmarks(const std::string& entity_id);
    bool addBookmark(const std::string& entity_id, const std::string& bookmark_id,
                     const std::string& label, const std::string& system_id,
                     float x, float y, float z);
    bool removeBookmark(const std::string& entity_id, const std::string& bookmark_id);
    bool setCategory(const std::string& entity_id, const std::string& bookmark_id,
                     const std::string& category);
    bool setNotes(const std::string& entity_id, const std::string& bookmark_id,
                  const std::string& notes);
    bool toggleFavorite(const std::string& entity_id, const std::string& bookmark_id);
    int getBookmarkCount(const std::string& entity_id) const;
    int getFavoriteCount(const std::string& entity_id) const;
    int getCategoryCount(const std::string& entity_id, const std::string& category) const;
    std::string getLabel(const std::string& entity_id, const std::string& bookmark_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NAVIGATION_BOOKMARK_SYSTEM_H
