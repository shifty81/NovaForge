#ifndef EVE_PCG_INTERIOR_GENERATOR_H
#define EVE_PCG_INTERIOR_GENERATOR_H

#include "pcg_context.h"
#include "interior_budget.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

// ── Interior room node (graph-based, FPS-ready) ─────────────────────

struct InteriorRoom {
    int              roomId;
    InteriorRoomType type;
    float            dimX, dimY, dimZ;   ///< FPS-navigable volume (metres).
    float            posX, posY, posZ;   ///< Local position within the ship.
    int              deckLevel;
    bool             pressurized;
    float            powerDraw;          ///< MW consumed by this room.
    float            heatOutput;         ///< Thermal output.
    int              crewCapacity;
    std::vector<int> connections;        ///< roomIds this room connects to.
};

// ── Interior corridor (typed for width scaling) ─────────────────────

struct InteriorCorridor {
    int   corridorId;
    int   fromRoomId;
    int   toRoomId;
    float width;      ///< metres — scales with ship class.
    float length;     ///< metres — distance between rooms.
    bool  isMainSpine; ///< true for the primary corridor.
};

// ── Complete ship interior ──────────────────────────────────────────

struct GeneratedInterior {
    uint64_t                       shipId;
    int                            shipClass;
    BridgeType                     bridgeType;
    InteriorBudget                 budget;
    std::vector<InteriorRoom>      rooms;
    std::vector<InteriorCorridor>  corridors;
    std::vector<HullInfluence>     hullInfluences;
    int                            deckCount;
    bool                           valid;  ///< Passed FPS validation.
};

/**
 * @brief Full procedural interior generator for all ship classes.
 *
 * Generation pipeline (deterministic, from newupload.txt):
 *   1. Compute interior budget from ship class
 *   2. Select bridge type
 *   3. Place mandatory rooms (cockpit, engineering, ...)
 *   4. Fill remaining budget with class-appropriate rooms
 *   5. Distribute rooms across decks
 *   6. Connect with corridors (main spine + loops for larger ships)
 *   7. Compute hull influences
 *   8. FPS validation pass
 *
 * Same seed → same interior forever.
 */
class InteriorGenerator {
public:
    /**
     * @brief Generate a complete ship interior.
     * @param ctx        PCG context (seed + version).
     * @param shipClass  0=Frigate .. 5=Capital.
     */
    static GeneratedInterior generate(const PCGContext& ctx, int shipClass);

private:
    static void placeMandatoryRooms(DeterministicRNG& rng,
                                    GeneratedInterior& interior);
    static void fillBudgetRooms(DeterministicRNG& rng,
                                GeneratedInterior& interior);
    static void distributeAcrossDecks(DeterministicRNG& rng,
                                      GeneratedInterior& interior);
    static void connectWithCorridors(DeterministicRNG& rng,
                                     GeneratedInterior& interior);
    static void addLoopConnections(DeterministicRNG& rng,
                                   GeneratedInterior& interior);
    static void computeHullInfluences(GeneratedInterior& interior);
    static bool validateFPS(const GeneratedInterior& interior);

    /// Helper: create a room with class-appropriate dimensions.
    static InteriorRoom makeRoom(DeterministicRNG& rng,
                                 int roomId,
                                 InteriorRoomType type,
                                 const InteriorBudget& budget);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_INTERIOR_GENERATOR_H
