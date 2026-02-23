#ifndef EVE_PCG_SNAPPABLE_GRID_H
#define EVE_PCG_SNAPPABLE_GRID_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Grid cell data ──────────────────────────────────────────────────

struct GridCell {
    int x, y, z;               ///< Grid coordinates.
    bool occupied;
    bool hasHiddenResource;
    uint32_t contentType;      ///< 0=empty, 1=station_module, 2=asteroid, 3=debris, 4=wreck.
};

// ── Snappable grid ──────────────────────────────────────────────────

/**
 * @brief 3-D grid-based snapping system for stations, asteroid fields,
 *        and debris.
 *
 * Cells are stored in a flat vector indexed as
 *   x + y*width + z*width*height
 * so look-ups are O(1).  generateSector() fills the grid procedurally
 * using the seed hierarchy from a PCGContext.
 */
class SnappableGrid {
public:
    SnappableGrid(int width, int height, int depth, float cellSize);

    GridCell*       getCell(int x, int y, int z);
    const GridCell* getCell(int x, int y, int z) const;

    bool placeContent(int x, int y, int z, uint32_t contentType);
    bool removeContent(int x, int y, int z);

    /** Procedurally fill sector using deterministic RNG. */
    void generateSector(const PCGContext& ctx);

    int   width()    const;
    int   height()   const;
    int   depth()    const;
    float cellSize() const;

private:
    int   width_, height_, depth_;
    float cellSize_;
    std::vector<GridCell> cells_;

    int  cellIndex(int x, int y, int z) const;
    bool inBounds(int x, int y, int z) const;
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SNAPPABLE_GRID_H
