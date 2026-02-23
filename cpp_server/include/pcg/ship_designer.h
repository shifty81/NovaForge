#ifndef EVE_PCG_SHIP_DESIGNER_H
#define EVE_PCG_SHIP_DESIGNER_H

#include "deck_graph.h"
#include "room_graph.h"
#include <cstdint>
#include <vector>

namespace atlas {
namespace pcg {

// ── Player-applied room override ────────────────────────────────────
struct RoomOverride {
    int      roomId;
    RoomType newType;
};

// ── Serialisable designer save state ────────────────────────────────
struct ShipDesignerSave {
    uint32_t                  pcgVersion;
    int                       shipClass;
    uint64_t                  seed;
    std::vector<RoomOverride> roomOverrides;
};

/**
 * @brief Apply saved room overrides to an existing deck layout.
 *
 * Iterates through @p save.roomOverrides and replaces the RoomType
 * of any room whose ID matches.
 *
 * @param decks  Mutable deck stack to modify in place.
 * @param save   Designer save containing overrides.
 */
void applyDesignerOverrides(std::vector<Deck>& decks,
                            const ShipDesignerSave& save);

/**
 * @brief Capture the current layout as a saveable snapshot.
 *
 * @param decks      Current deck stack.
 * @param shipClass  Ship class used during generation.
 * @param seed       Root seed used during generation.
 * @return ShipDesignerSave ready for serialisation.
 */
ShipDesignerSave saveShipLayout(const std::vector<Deck>& decks,
                                int shipClass,
                                uint64_t seed);

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SHIP_DESIGNER_H
