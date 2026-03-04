#include "systems/navigation_bookmark_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::NavigationBookmark::Bookmark* findBookmark(
    components::NavigationBookmark* nb, const std::string& bookmark_id) {
    for (auto& b : nb->bookmarks) {
        if (b.bookmark_id == bookmark_id) return &b;
    }
    return nullptr;
}
} // anonymous namespace

NavigationBookmarkSystem::NavigationBookmarkSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void NavigationBookmarkSystem::updateComponent(ecs::Entity& /*entity*/, components::NavigationBookmark& /*bookmark*/, float /*delta_time*/) {
    // Bookmarks are passive data; no per-frame processing needed
}

bool NavigationBookmarkSystem::initializeBookmarks(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NavigationBookmark>();
    entity->addComponent(std::move(comp));
    return true;
}

bool NavigationBookmarkSystem::addBookmark(const std::string& entity_id,
    const std::string& bookmark_id, const std::string& label,
    const std::string& system_id, float x, float y, float z) {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return false;
    if (static_cast<int>(nb->bookmarks.size()) >= nb->max_bookmarks) return false;
    if (findBookmark(nb, bookmark_id)) return false; // duplicate

    components::NavigationBookmark::Bookmark bm;
    bm.bookmark_id = bookmark_id;
    bm.label = label;
    bm.system_id = system_id;
    bm.x = x;
    bm.y = y;
    bm.z = z;
    nb->bookmarks.push_back(bm);
    nb->total_created++;
    return true;
}

bool NavigationBookmarkSystem::removeBookmark(const std::string& entity_id,
    const std::string& bookmark_id) {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return false;

    auto it = std::remove_if(nb->bookmarks.begin(), nb->bookmarks.end(),
        [&](const components::NavigationBookmark::Bookmark& b) {
            return b.bookmark_id == bookmark_id;
        });
    if (it == nb->bookmarks.end()) return false;
    nb->bookmarks.erase(it, nb->bookmarks.end());
    return true;
}

bool NavigationBookmarkSystem::setCategory(const std::string& entity_id,
    const std::string& bookmark_id, const std::string& category) {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return false;
    auto* bm = findBookmark(nb, bookmark_id);
    if (!bm) return false;
    bm->category = category;
    return true;
}

bool NavigationBookmarkSystem::setNotes(const std::string& entity_id,
    const std::string& bookmark_id, const std::string& notes) {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return false;
    auto* bm = findBookmark(nb, bookmark_id);
    if (!bm) return false;
    bm->notes = notes;
    return true;
}

bool NavigationBookmarkSystem::toggleFavorite(const std::string& entity_id,
    const std::string& bookmark_id) {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return false;
    auto* bm = findBookmark(nb, bookmark_id);
    if (!bm) return false;
    bm->is_favorite = !bm->is_favorite;
    return true;
}

int NavigationBookmarkSystem::getBookmarkCount(const std::string& entity_id) const {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return 0;
    return static_cast<int>(nb->bookmarks.size());
}

int NavigationBookmarkSystem::getFavoriteCount(const std::string& entity_id) const {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return 0;
    int count = 0;
    for (const auto& b : nb->bookmarks) {
        if (b.is_favorite) count++;
    }
    return count;
}

int NavigationBookmarkSystem::getCategoryCount(const std::string& entity_id,
    const std::string& category) const {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return 0;
    int count = 0;
    for (const auto& b : nb->bookmarks) {
        if (b.category == category) count++;
    }
    return count;
}

std::string NavigationBookmarkSystem::getLabel(const std::string& entity_id,
    const std::string& bookmark_id) const {
    auto* nb = getComponentFor(entity_id);
    if (!nb) return "";
    for (const auto& b : nb->bookmarks) {
        if (b.bookmark_id == bookmark_id) return b.label;
    }
    return "";
}

} // namespace systems
} // namespace atlas
