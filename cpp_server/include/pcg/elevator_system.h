#ifndef EVE_PCG_ELEVATOR_SYSTEM_H
#define EVE_PCG_ELEVATOR_SYSTEM_H

#include "pcg_context.h"
#include "deck_graph.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

// ── Single button inside an elevator panel ──────────────────────────
struct ElevatorButton {
    int   targetDeck;
    float posX, posY, posZ;   ///< Position on the panel.
};

// ── Elevator shaft spanning multiple decks ──────────────────────────
struct ElevatorNode {
    int                        elevatorId;
    float                      basePosX, basePosY, basePosZ;
    float                      width, height;
    int                        floorCount;
    std::vector<ElevatorButton> buttons;
};

/**
 * @brief Generate an elevator shaft for a given deck.
 *
 * The elevator is placed at the position of the deck's first room.
 * Buttons are created for every floor from 0 to @p totalDecks - 1.
 *
 * @param deck        Deck that anchors this elevator.
 * @param totalDecks  Total number of decks in the ship.
 * @param ctx         PCG context.
 * @return Deterministic ElevatorNode.
 */
ElevatorNode generateElevator(const Deck& deck,
                              int totalDecks,
                              const PCGContext& ctx);

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_ELEVATOR_SYSTEM_H
