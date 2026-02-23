#ifndef EVE_PCG_DECK_GRAPH_H
#define EVE_PCG_DECK_GRAPH_H

#include "pcg_context.h"
#include "room_graph.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

// ── Corridor connecting two rooms on the same deck ──────────────────
struct Corridor {
    int fromRoomId;
    int toRoomId;
};

// ── Single deck in the ship ─────────────────────────────────────────
struct Deck {
    int                   index;
    float                 heightOffset;   ///< Vertical offset from ship origin.
    std::vector<RoomNode> rooms;
    std::vector<Corridor> corridors;
};

/**
 * @brief Generate a vertical stack of decks for a ship.
 *
 * Deck count is derived from @p shipClass + 2, clamped to [2, 8].
 * Each deck is separated by 3.0 m in height.  Rooms and corridors
 * are populated per deck.
 *
 * @param shipClass  Ship class 0-5.
 * @param ctx        PCG context.
 * @return Deterministic vector of Deck.
 */
std::vector<Deck> generateDeckStack(int shipClass, const PCGContext& ctx);

/**
 * @brief Connect each consecutive room pair with a corridor.
 *
 * @param rooms  Room list (must share the same deck).
 * @return Vector of Corridor edges.
 */
std::vector<Corridor> connectRooms(const std::vector<RoomNode>& rooms);

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_DECK_GRAPH_H
