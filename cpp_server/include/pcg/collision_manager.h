#ifndef EVE_PCG_COLLISION_MANAGER_H
#define EVE_PCG_COLLISION_MANAGER_H

#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Axis-aligned bounding box ───────────────────────────────────────
struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

// ── Collision volume ────────────────────────────────────────────────
struct CollisionVolume {
    int objectId;
    AABB bounds;
    bool isStatic;    // true for walls/floors, false for elevators/doors
    bool isWalkable;
};

/**
 * @brief Simplified collision meshes for procedural modules.
 *
 * Manages a collection of AABB collision volumes and provides
 * point, box, and region queries against them.
 */
class CollisionManager {
public:
    void addVolume(const CollisionVolume& volume);
    void removeVolume(int objectId);
    void clear();

    bool testPoint(float x, float y, float z) const;
    bool testAABB(const AABB& box) const;
    std::vector<int> queryRegion(const AABB& region) const;

    const std::vector<CollisionVolume>& volumes() const;
    size_t volumeCount() const;

    static AABB computeRoomAABB(float posX, float posY, float posZ,
                                 float dimX, float dimY, float dimZ);

private:
    std::vector<CollisionVolume> volumes_;

    static bool aabbOverlap(const AABB& a, const AABB& b);
    static bool pointInAABB(float x, float y, float z, const AABB& box);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_COLLISION_MANAGER_H
