#include "pcg/room_graph.h"
#include "pcg/hash_utils.h"

namespace atlas {
namespace pcg {

// ── Room-type cycle order ──────────────────────────────────────────
static constexpr RoomType TYPE_CYCLE[] = {
    RoomType::Cockpit,
    RoomType::Reactor,
    RoomType::CrewQuarters,
    RoomType::Engine,
    RoomType::WeaponControl,
    RoomType::Corridor,
};
static constexpr int TYPE_CYCLE_LEN = 6;

// ── Public API ─────────────────────────────────────────────────────

std::vector<RoomNode> generateRoomsForDeck(int deckIndex,
                                           const PCGContext& ctx,
                                           int shipClass) {
    // Derive a deck-specific seed so each deck is independent.
    uint64_t deckSeed = deriveSeed(ctx.seed, static_cast<uint64_t>(deckIndex));
    DeterministicRNG rng(deckSeed);

    // Room count: base 2, plus up to shipClass extra rooms, capped at 5.
    int roomCount = 2 + rng.range(0, shipClass);
    if (roomCount > 5) roomCount = 5;
    if (roomCount < 2) roomCount = 2;

    std::vector<RoomNode> rooms;
    rooms.reserve(static_cast<size_t>(roomCount));

    constexpr float SPACING = 6.0f; // metres between room centres

    for (int i = 0; i < roomCount; ++i) {
        RoomNode node{};
        node.roomId = deckIndex * 100 + i; // unique across decks
        node.type   = TYPE_CYCLE[i % TYPE_CYCLE_LEN];

        // Dimensions: roughly 3-5 metres per axis.
        node.dimX = rng.rangeFloat(3.0f, 5.0f);
        node.dimY = rng.rangeFloat(3.0f, 5.0f);
        node.dimZ = rng.rangeFloat(3.0f, 5.0f);

        // Position rooms along the X axis.
        node.posX = static_cast<float>(i) * SPACING;
        node.posY = 0.0f;
        node.posZ = 0.0f;

        rooms.push_back(node);
    }

    return rooms;
}

} // namespace pcg
} // namespace atlas
