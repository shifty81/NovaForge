#ifndef EVE_PCG_HULL_MESHER_H
#define EVE_PCG_HULL_MESHER_H

#include "deck_graph.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

// ── Vertex for server-side hull geometry ────────────────────────────
struct HullVertex {
    float x, y, z;
};

// ── Triangle-indexed mesh ───────────────────────────────────────────
struct HullMesh {
    std::vector<HullVertex> vertices;
    std::vector<uint32_t>   indices;
};

/**
 * @brief Generate a bounding-box hull mesh from a deck layout.
 *
 * For every room on every deck an axis-aligned box is emitted with
 * @p padding added to each side.  The result is a simple but
 * functional approximation of the ship's outer hull.
 *
 * @param decks    Full deck stack with rooms.
 * @param padding  Extra metres added around each room AABB.
 * @return Triangle-indexed HullMesh.
 */
HullMesh generateHullMesh(const std::vector<Deck>& decks, float padding);

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_HULL_MESHER_H
