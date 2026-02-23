#include "pcg/ship_designer.h"

namespace atlas {
namespace pcg {

// ── Current PCG version tag ────────────────────────────────────────
static constexpr uint32_t CURRENT_PCG_VERSION = 1;

// ── Public API ─────────────────────────────────────────────────────

void applyDesignerOverrides(std::vector<Deck>& decks,
                            const ShipDesignerSave& save) {
    for (const RoomOverride& ovr : save.roomOverrides) {
        for (Deck& deck : decks) {
            for (RoomNode& room : deck.rooms) {
                if (room.roomId == ovr.roomId) {
                    room.type = ovr.newType;
                }
            }
        }
    }
}

ShipDesignerSave saveShipLayout(const std::vector<Deck>& /*decks*/,
                                int shipClass,
                                uint64_t seed) {
    ShipDesignerSave save{};
    save.pcgVersion = CURRENT_PCG_VERSION;
    save.shipClass  = shipClass;
    save.seed       = seed;
    // roomOverrides left empty — caller adds overrides as needed.
    return save;
}

} // namespace pcg
} // namespace atlas
