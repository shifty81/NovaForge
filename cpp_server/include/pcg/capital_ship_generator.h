#ifndef EVE_PCG_CAPITAL_SHIP_GENERATOR_H
#define EVE_PCG_CAPITAL_SHIP_GENERATOR_H

#include "pcg_context.h"
#include "deck_graph.h"
#include "elevator_system.h"
#include "hull_mesher.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

// ── Full generated capital ship ─────────────────────────────────────
struct GeneratedCapitalShip {
    uint64_t                  shipId;
    int                       shipClass;  ///< 0=Frigate .. 5=Titan
    std::vector<Deck>         decks;
    std::vector<ElevatorNode> elevators;
    HullMesh                  hull;
    bool                      valid;
};

/**
 * @brief Deterministic capital-ship interior generator.
 *
 * Ties together decks, rooms, elevators, and hull mesh into a single
 * coherent GeneratedCapitalShip.  The same PCGContext always yields
 * the same ship.
 *
 * Generation pipeline:
 *   1. Select ship class (or use override)
 *   2. generateDeckStack → decks with rooms and corridors
 *   3. generateElevator per deck (if > 1 deck)
 *   4. generateHullMesh from all decks
 *   5. valid = decks.size() >= 2
 */
class CapitalShipGenerator {
public:
    /** Generate a ship with a randomly selected class. */
    static GeneratedCapitalShip generate(const PCGContext& ctx);

    /** Generate a ship with an explicit class override. */
    static GeneratedCapitalShip generate(const PCGContext& ctx, int shipClass);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_CAPITAL_SHIP_GENERATOR_H
