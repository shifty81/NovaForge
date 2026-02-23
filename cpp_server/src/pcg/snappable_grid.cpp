#include "pcg/snappable_grid.h"
#include "pcg/hash_utils.h"

namespace atlas {
namespace pcg {

// ── Construction ───────────────────────────────────────────────────

SnappableGrid::SnappableGrid(int width, int height, int depth, float cellSize)
    : width_(width), height_(height), depth_(depth), cellSize_(cellSize)
{
    cells_.resize(static_cast<size_t>(width_ * height_ * depth_));
    for (int z = 0; z < depth_; ++z) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                auto& c     = cells_[static_cast<size_t>(cellIndex(x, y, z))];
                c.x         = x;
                c.y         = y;
                c.z         = z;
                c.occupied  = false;
                c.hasHiddenResource = false;
                c.contentType = 0;
            }
        }
    }
}

// ── Accessors ──────────────────────────────────────────────────────

int   SnappableGrid::width()    const { return width_;    }
int   SnappableGrid::height()   const { return height_;   }
int   SnappableGrid::depth()    const { return depth_;    }
float SnappableGrid::cellSize() const { return cellSize_; }

// ── Cell access ────────────────────────────────────────────────────

int SnappableGrid::cellIndex(int x, int y, int z) const {
    return x + y * width_ + z * width_ * height_;
}

bool SnappableGrid::inBounds(int x, int y, int z) const {
    return x >= 0 && x < width_
        && y >= 0 && y < height_
        && z >= 0 && z < depth_;
}

GridCell* SnappableGrid::getCell(int x, int y, int z) {
    if (!inBounds(x, y, z)) return nullptr;
    return &cells_[static_cast<size_t>(cellIndex(x, y, z))];
}

const GridCell* SnappableGrid::getCell(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return nullptr;
    return &cells_[static_cast<size_t>(cellIndex(x, y, z))];
}

// ── Content placement ──────────────────────────────────────────────

bool SnappableGrid::placeContent(int x, int y, int z, uint32_t contentType) {
    if (!inBounds(x, y, z)) return false;
    auto& c = cells_[static_cast<size_t>(cellIndex(x, y, z))];
    if (c.occupied) return false;
    c.occupied    = true;
    c.contentType = contentType;
    return true;
}

bool SnappableGrid::removeContent(int x, int y, int z) {
    if (!inBounds(x, y, z)) return false;
    auto& c = cells_[static_cast<size_t>(cellIndex(x, y, z))];
    if (!c.occupied) return false;
    c.occupied        = false;
    c.hasHiddenResource = false;
    c.contentType     = 0;
    return true;
}

// ── Procedural sector generation ───────────────────────────────────

void SnappableGrid::generateSector(const PCGContext& ctx) {
    DeterministicRNG rng(ctx.seed);

    for (int z = 0; z < depth_; ++z) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                auto& c = cells_[static_cast<size_t>(cellIndex(x, y, z))];

                // Derive a per-cell seed so order of evaluation
                // cannot affect neighbouring cells.
                uint64_t cellSeed = deriveSeed(ctx.seed,
                    hash64(static_cast<uint64_t>(x),
                           static_cast<uint64_t>(y),
                           static_cast<uint64_t>(z), 0ULL));
                DeterministicRNG cellRng(cellSeed);

                float roll = cellRng.nextFloat();

                if (roll < 0.02f) {
                    // ~2 % asteroid containing a hidden resource
                    c.occupied          = true;
                    c.hasHiddenResource = true;
                    c.contentType       = 2; // asteroid
                } else if (roll < 0.17f) {
                    // ~15 % asteroid
                    c.occupied    = true;
                    c.contentType = 2;
                } else if (roll < 0.22f) {
                    // ~5 % debris
                    c.occupied    = true;
                    c.contentType = 3;
                } else {
                    c.occupied    = false;
                    c.contentType = 0;
                }
            }
        }
    }
}

} // namespace pcg
} // namespace atlas
