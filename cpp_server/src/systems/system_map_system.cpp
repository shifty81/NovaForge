#include "systems/system_map_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

SystemMapSystem::SystemMapSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SystemMapSystem::updateComponent(ecs::Entity& /*entity*/,
    components::SystemMap& map, float delta_time) {
    if (!map.active) return;
    map.elapsed += delta_time;
}

bool SystemMapSystem::addCelestial(const std::string& entity_id,
    const std::string& celestial_id, const std::string& name,
    const std::string& type, float x, float y, float z, float radius) {
    auto* map = getComponentFor(entity_id);
    if (!map) return false;

    // Duplicate check
    for (const auto& c : map->celestials) {
        if (c.celestial_id == celestial_id) return false;
    }
    if (static_cast<int>(map->celestials.size()) >= map->max_celestials) return false;

    components::SystemMap::Celestial cel;
    cel.celestial_id = celestial_id;
    cel.name = name;
    cel.type = type;
    cel.x = x;
    cel.y = y;
    cel.z = z;
    cel.radius = radius;
    map->celestials.push_back(cel);
    return true;
}

bool SystemMapSystem::removeCelestial(const std::string& entity_id,
    const std::string& celestial_id) {
    auto* map = getComponentFor(entity_id);
    if (!map) return false;

    auto it = std::remove_if(map->celestials.begin(), map->celestials.end(),
        [&celestial_id](const components::SystemMap::Celestial& c) {
            return c.celestial_id == celestial_id;
        });
    if (it == map->celestials.end()) return false;
    map->celestials.erase(it, map->celestials.end());
    return true;
}

bool SystemMapSystem::addBookmark(const std::string& entity_id,
    const std::string& bookmark_id, const std::string& label,
    const std::string& folder, float x, float y, float z) {
    auto* map = getComponentFor(entity_id);
    if (!map) return false;

    for (const auto& b : map->bookmarks) {
        if (b.bookmark_id == bookmark_id) return false;
    }
    if (static_cast<int>(map->bookmarks.size()) >= map->max_bookmarks) return false;

    components::SystemMap::Bookmark bm;
    bm.bookmark_id = bookmark_id;
    bm.label = label;
    bm.folder = folder;
    bm.x = x;
    bm.y = y;
    bm.z = z;
    bm.created_at = map->elapsed;
    map->bookmarks.push_back(bm);
    map->total_bookmarks_created++;
    return true;
}

bool SystemMapSystem::removeBookmark(const std::string& entity_id,
    const std::string& bookmark_id) {
    auto* map = getComponentFor(entity_id);
    if (!map) return false;

    auto it = std::remove_if(map->bookmarks.begin(), map->bookmarks.end(),
        [&bookmark_id](const components::SystemMap::Bookmark& b) {
            return b.bookmark_id == bookmark_id;
        });
    if (it == map->bookmarks.end()) return false;
    map->bookmarks.erase(it, map->bookmarks.end());
    return true;
}

bool SystemMapSystem::addSignature(const std::string& entity_id,
    const std::string& sig_id, const std::string& type,
    float scan_strength, float x, float y, float z) {
    auto* map = getComponentFor(entity_id);
    if (!map) return false;

    for (const auto& s : map->signatures) {
        if (s.sig_id == sig_id) return false;
    }
    if (static_cast<int>(map->signatures.size()) >= map->max_signatures) return false;

    components::SystemMap::Signature sig;
    sig.sig_id = sig_id;
    sig.type = type;
    sig.scan_strength = std::max(0.0f, std::min(1.0f, scan_strength));
    sig.x = x;
    sig.y = y;
    sig.z = z;
    map->signatures.push_back(sig);
    map->total_signatures_scanned++;
    return true;
}

bool SystemMapSystem::resolveSignature(const std::string& entity_id,
    const std::string& sig_id) {
    auto* map = getComponentFor(entity_id);
    if (!map) return false;

    for (auto& s : map->signatures) {
        if (s.sig_id == sig_id) {
            s.resolved = true;
            s.scan_strength = 1.0f;
            return true;
        }
    }
    return false;
}

int SystemMapSystem::getCelestialCount(const std::string& entity_id) const {
    auto* map = getComponentFor(entity_id);
    return map ? static_cast<int>(map->celestials.size()) : 0;
}

int SystemMapSystem::getBookmarkCount(const std::string& entity_id) const {
    auto* map = getComponentFor(entity_id);
    return map ? static_cast<int>(map->bookmarks.size()) : 0;
}

int SystemMapSystem::getSignatureCount(const std::string& entity_id) const {
    auto* map = getComponentFor(entity_id);
    return map ? static_cast<int>(map->signatures.size()) : 0;
}

float SystemMapSystem::getDistanceBetween(const std::string& entity_id,
    const std::string& id_a, const std::string& id_b) const {
    auto* map = getComponentFor(entity_id);
    if (!map) return 0.0f;

    float ax = 0, ay = 0, az = 0, bx = 0, by = 0, bz = 0;
    bool found_a = false, found_b = false;

    // Search celestials, bookmarks, and signatures
    for (const auto& c : map->celestials) {
        if (c.celestial_id == id_a) { ax = c.x; ay = c.y; az = c.z; found_a = true; }
        if (c.celestial_id == id_b) { bx = c.x; by = c.y; bz = c.z; found_b = true; }
    }
    for (const auto& b : map->bookmarks) {
        if (b.bookmark_id == id_a) { ax = b.x; ay = b.y; az = b.z; found_a = true; }
        if (b.bookmark_id == id_b) { bx = b.x; by = b.y; bz = b.z; found_b = true; }
    }
    for (const auto& s : map->signatures) {
        if (s.sig_id == id_a) { ax = s.x; ay = s.y; az = s.z; found_a = true; }
        if (s.sig_id == id_b) { bx = s.x; by = s.y; bz = s.z; found_b = true; }
    }

    if (!found_a || !found_b) return 0.0f;

    float dx = ax - bx;
    float dy = ay - by;
    float dz = az - bz;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

int SystemMapSystem::getTotalBookmarksCreated(const std::string& entity_id) const {
    auto* map = getComponentFor(entity_id);
    return map ? map->total_bookmarks_created : 0;
}

} // namespace systems
} // namespace atlas
