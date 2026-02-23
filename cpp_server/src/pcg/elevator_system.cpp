#include "pcg/elevator_system.h"
#include "pcg/hash_utils.h"

namespace atlas {
namespace pcg {

// ── Constants ──────────────────────────────────────────────────────
static constexpr float ELEVATOR_WIDTH  = 2.0f; // metres
static constexpr float ELEVATOR_HEIGHT = 2.8f; // metres (per floor)
static constexpr float BUTTON_SPACING  = 0.12f; // metres between buttons

// ── Public API ─────────────────────────────────────────────────────

ElevatorNode generateElevator(const Deck& deck,
                              int totalDecks,
                              const PCGContext& ctx) {
    uint64_t elSeed = deriveSeed(ctx.seed, static_cast<uint64_t>(deck.index + 1000));
    DeterministicRNG rng(elSeed);

    ElevatorNode node{};
    node.elevatorId = deck.index;
    node.width      = ELEVATOR_WIDTH;
    node.height     = ELEVATOR_HEIGHT;
    node.floorCount = totalDecks;

    // Place the elevator at the first room's position (if available).
    if (!deck.rooms.empty()) {
        const RoomNode& anchor = deck.rooms.front();
        node.basePosX = anchor.posX;
        node.basePosY = anchor.posY;
        node.basePosZ = anchor.posZ;
    } else {
        node.basePosX = 0.0f;
        node.basePosY = 0.0f;
        node.basePosZ = 0.0f;
    }

    // Create one button per floor, arranged vertically on the panel.
    node.buttons.reserve(static_cast<size_t>(totalDecks));
    for (int f = 0; f < totalDecks; ++f) {
        ElevatorButton btn{};
        btn.targetDeck = f;
        btn.posX = 0.0f;
        btn.posY = static_cast<float>(f) * BUTTON_SPACING;
        btn.posZ = 0.0f;
        node.buttons.push_back(btn);
    }

    return node;
}

} // namespace pcg
} // namespace atlas
