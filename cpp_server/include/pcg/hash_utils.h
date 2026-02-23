#ifndef EVE_PCG_HASH_UTILS_H
#define EVE_PCG_HASH_UTILS_H

#include <cstdint>

namespace atlas {
namespace pcg {

/**
 * @brief Deterministic hash utilities for PCG seed derivation.
 *
 * Combines multiple values into a single 64-bit seed.  Used to create
 * the hierarchical seed tree:
 *
 *   UniverseSeed
 *    └── SystemSeed(system_id)
 *         └── SectorSeed(sector_coords)
 *              └── ObjectSeed(object_id)
 */

/** Combine two 64-bit values into a new hash (splitmix-based). */
inline uint64_t hashCombine(uint64_t a, uint64_t b) {
    uint64_t x = a ^ (b + 0x9E3779B97F4A7C15ULL + (a << 6) + (a >> 2));
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;
    return x;
}

/** Combine four 64-bit values into a single seed. */
inline uint64_t hash64(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t h = hashCombine(a, b);
    h = hashCombine(h, c);
    h = hashCombine(h, d);
    return h;
}

/** Derive a child seed from parent seed + object id. */
inline uint64_t deriveSeed(uint64_t parentSeed, uint64_t objectId) {
    return hashCombine(parentSeed, objectId);
}

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_HASH_UTILS_H
