#include "pcg/hull_mesher.h"

namespace atlas {
namespace pcg {

// ── Helpers ────────────────────────────────────────────────────────

/** Emit six faces (12 triangles) for an axis-aligned box. */
static void emitBox(HullMesh& mesh,
                    float minX, float minY, float minZ,
                    float maxX, float maxY, float maxZ) {
    uint32_t base = static_cast<uint32_t>(mesh.vertices.size());

    // 8 corners of the box.
    mesh.vertices.push_back({minX, minY, minZ}); // 0
    mesh.vertices.push_back({maxX, minY, minZ}); // 1
    mesh.vertices.push_back({maxX, maxY, minZ}); // 2
    mesh.vertices.push_back({minX, maxY, minZ}); // 3
    mesh.vertices.push_back({minX, minY, maxZ}); // 4
    mesh.vertices.push_back({maxX, minY, maxZ}); // 5
    mesh.vertices.push_back({maxX, maxY, maxZ}); // 6
    mesh.vertices.push_back({minX, maxY, maxZ}); // 7

    // 6 faces × 2 triangles each = 12 triangles.
    auto tri = [&](uint32_t a, uint32_t b, uint32_t c) {
        mesh.indices.push_back(base + a);
        mesh.indices.push_back(base + b);
        mesh.indices.push_back(base + c);
    };

    // Front  (−Z)
    tri(0, 1, 2); tri(0, 2, 3);
    // Back   (+Z)
    tri(5, 4, 7); tri(5, 7, 6);
    // Left   (−X)
    tri(4, 0, 3); tri(4, 3, 7);
    // Right  (+X)
    tri(1, 5, 6); tri(1, 6, 2);
    // Bottom (−Y)
    tri(4, 5, 1); tri(4, 1, 0);
    // Top    (+Y)
    tri(3, 2, 6); tri(3, 6, 7);
}

// ── Public API ─────────────────────────────────────────────────────

HullMesh generateHullMesh(const std::vector<Deck>& decks, float padding) {
    HullMesh mesh;

    for (const Deck& deck : decks) {
        for (const RoomNode& room : deck.rooms) {
            float halfX = room.dimX * 0.5f + padding;
            float halfY = room.dimY * 0.5f + padding;
            float halfZ = room.dimZ * 0.5f + padding;

            float cx = room.posX;
            float cy = room.posY + deck.heightOffset;
            float cz = room.posZ;

            emitBox(mesh,
                    cx - halfX, cy - halfY, cz - halfZ,
                    cx + halfX, cy + halfY, cz + halfZ);
        }
    }

    return mesh;
}

} // namespace pcg
} // namespace atlas
