#include "pcg/capital_ship_generator.h"
#include "pcg/deterministic_rng.h"
#include "pcg/hash_utils.h"

namespace atlas {
namespace pcg {

// ── Constants ──────────────────────────────────────────────────────
static constexpr int   MIN_SHIP_CLASS = 0;
static constexpr int   MAX_SHIP_CLASS = 5;
static constexpr float HULL_PADDING   = 0.5f; // metres

// ── Internal builder ───────────────────────────────────────────────

static GeneratedCapitalShip build(const PCGContext& ctx, int shipClass) {
    GeneratedCapitalShip ship{};
    ship.shipId    = ctx.seed;
    ship.shipClass = shipClass;

    // 1. Generate the deck stack (rooms + corridors are included).
    ship.decks = generateDeckStack(shipClass, ctx);

    // 2. Generate elevators (only meaningful when >1 deck).
    if (ship.decks.size() > 1) {
        int totalDecks = static_cast<int>(ship.decks.size());
        ship.elevators.reserve(ship.decks.size());
        for (const Deck& deck : ship.decks) {
            ship.elevators.push_back(generateElevator(deck, totalDecks, ctx));
        }
    }

    // 3. Generate hull mesh.
    ship.hull = generateHullMesh(ship.decks, HULL_PADDING);

    // 4. Validity: need at least 2 decks.
    ship.valid = ship.decks.size() >= 2;

    return ship;
}

// ── Public API ─────────────────────────────────────────────────────

GeneratedCapitalShip CapitalShipGenerator::generate(const PCGContext& ctx) {
    DeterministicRNG rng(ctx.seed);
    int shipClass = rng.range(MIN_SHIP_CLASS, MAX_SHIP_CLASS);
    return build(ctx, shipClass);
}

GeneratedCapitalShip CapitalShipGenerator::generate(const PCGContext& ctx,
                                                    int shipClass) {
    if (shipClass < MIN_SHIP_CLASS) shipClass = MIN_SHIP_CLASS;
    if (shipClass > MAX_SHIP_CLASS) shipClass = MAX_SHIP_CLASS;
    return build(ctx, shipClass);
}

} // namespace pcg
} // namespace atlas
