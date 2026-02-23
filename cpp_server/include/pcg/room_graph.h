#ifndef EVE_PCG_ROOM_GRAPH_H
#define EVE_PCG_ROOM_GRAPH_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

// ── Room types for ship / station interiors ─────────────────────────
enum class RoomType : uint32_t {
    Cockpit,
    Reactor,
    CrewQuarters,
    Engine,
    WeaponControl,
    Corridor,
};

// ── Single room within a deck ───────────────────────────────────────
struct RoomNode {
    int      roomId;
    RoomType type;
    float    dimX, dimY, dimZ;   ///< Dimensions in metres (no glm on server).
    float    posX, posY, posZ;   ///< Local position within the deck.
};

/**
 * @brief Generate rooms for a single deck.
 *
 * Produces 2-5 rooms depending on @p shipClass, positioned along the
 * X axis with consistent spacing.  Types are assigned cyclically.
 *
 * @param deckIndex  Zero-based deck index (mixed into seed).
 * @param ctx        PCG context carrying the derived seed.
 * @param shipClass  Ship class 0-5 (higher = more rooms).
 * @return Deterministic vector of RoomNode.
 */
std::vector<RoomNode> generateRoomsForDeck(int deckIndex,
                                           const PCGContext& ctx,
                                           int shipClass);

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_ROOM_GRAPH_H
