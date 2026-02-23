#include "pcg/deck_graph.h"
#include "pcg/hash_utils.h"

namespace atlas {
namespace pcg {

// ── Constants ──────────────────────────────────────────────────────
static constexpr float DECK_HEIGHT = 3.0f; // metres per deck
static constexpr int   MIN_DECKS   = 2;
static constexpr int   MAX_DECKS   = 8;

// ── Public API ─────────────────────────────────────────────────────

std::vector<Deck> generateDeckStack(int shipClass, const PCGContext& ctx) {
    int deckCount = shipClass + 2;
    if (deckCount < MIN_DECKS) deckCount = MIN_DECKS;
    if (deckCount > MAX_DECKS) deckCount = MAX_DECKS;

    std::vector<Deck> decks;
    decks.reserve(static_cast<size_t>(deckCount));

    for (int i = 0; i < deckCount; ++i) {
        Deck deck{};
        deck.index        = i;
        deck.heightOffset = static_cast<float>(i) * DECK_HEIGHT;
        deck.rooms        = generateRoomsForDeck(i, ctx, shipClass);
        deck.corridors    = connectRooms(deck.rooms);
        decks.push_back(std::move(deck));
    }

    return decks;
}

std::vector<Corridor> connectRooms(const std::vector<RoomNode>& rooms) {
    std::vector<Corridor> corridors;
    if (rooms.size() < 2) return corridors;

    corridors.reserve(rooms.size() - 1);
    for (size_t i = 0; i + 1 < rooms.size(); ++i) {
        corridors.push_back({rooms[i].roomId, rooms[i + 1].roomId});
    }
    return corridors;
}

} // namespace pcg
} // namespace atlas
