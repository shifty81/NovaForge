#include "pcg/collision_manager.h"

#include <algorithm>

namespace atlas {
namespace pcg {

// ── Public API ─────────────────────────────────────────────────────

void CollisionManager::addVolume(const CollisionVolume& volume) {
    volumes_.push_back(volume);
}

void CollisionManager::removeVolume(int objectId) {
    volumes_.erase(
        std::remove_if(volumes_.begin(), volumes_.end(),
            [objectId](const CollisionVolume& v) { return v.objectId == objectId; }),
        volumes_.end());
}

void CollisionManager::clear() {
    volumes_.clear();
}

bool CollisionManager::testPoint(float x, float y, float z) const {
    for (const auto& vol : volumes_) {
        if (pointInAABB(x, y, z, vol.bounds)) {
            return true;
        }
    }
    return false;
}

bool CollisionManager::testAABB(const AABB& box) const {
    for (const auto& vol : volumes_) {
        if (aabbOverlap(vol.bounds, box)) {
            return true;
        }
    }
    return false;
}

std::vector<int> CollisionManager::queryRegion(const AABB& region) const {
    std::vector<int> result;
    for (const auto& vol : volumes_) {
        if (aabbOverlap(vol.bounds, region)) {
            result.push_back(vol.objectId);
        }
    }
    return result;
}

const std::vector<CollisionVolume>& CollisionManager::volumes() const {
    return volumes_;
}

size_t CollisionManager::volumeCount() const {
    return volumes_.size();
}

AABB CollisionManager::computeRoomAABB(float posX, float posY, float posZ,
                                        float dimX, float dimY, float dimZ) {
    float halfX = dimX * 0.5f;
    float halfY = dimY * 0.5f;
    float halfZ = dimZ * 0.5f;
    return { posX - halfX, posY - halfY, posZ - halfZ,
             posX + halfX, posY + halfY, posZ + halfZ };
}

// ── Internals ──────────────────────────────────────────────────────

bool CollisionManager::aabbOverlap(const AABB& a, const AABB& b) {
    if (a.maxX < b.minX || a.minX > b.maxX) return false;
    if (a.maxY < b.minY || a.minY > b.maxY) return false;
    if (a.maxZ < b.minZ || a.minZ > b.maxZ) return false;
    return true;
}

bool CollisionManager::pointInAABB(float x, float y, float z, const AABB& box) {
    return x >= box.minX && x <= box.maxX
        && y >= box.minY && y <= box.maxY
        && z >= box.minZ && z <= box.maxZ;
}

} // namespace pcg
} // namespace atlas
