#include "pcg/npc_encounter_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Constants ──────────────────────────────────────────────────────
static constexpr int   MIN_WAVES          = 1;
static constexpr int   MAX_WAVES          = 5;
static constexpr int   MIN_SHIPS_PER_WAVE = 2;
static constexpr int   MAX_SHIPS_PER_WAVE = 8;
static constexpr float BASE_DELAY         = 15.0f;  // seconds between waves
static constexpr float BASE_SPAWN_RADIUS  = 30.0f;  // metres

// ── Public API ─────────────────────────────────────────────────────

NPCEncounter NPCEncounterGenerator::generate(const PCGContext& ctx,
                                             float secLevel) {
    DeterministicRNG rng(ctx.seed);
    int waveCount = rng.range(MIN_WAVES, MAX_WAVES);
    return generate(ctx, waveCount, secLevel);
}

NPCEncounter NPCEncounterGenerator::generate(const PCGContext& ctx,
                                             int waveCount,
                                             float secLevel) {
    DeterministicRNG rng(ctx.seed);
    // Consume the same first roll for consistency.
    (void)rng.range(MIN_WAVES, MAX_WAVES);

    int waves = std::max(MIN_WAVES, std::min(waveCount, MAX_WAVES));

    NPCEncounter encounter{};
    encounter.encounterId = ctx.seed;
    encounter.faction     = selectFaction(rng, secLevel);
    encounter.totalShips  = 0;
    encounter.valid       = false;

    float totalBounty = 0.0f;

    encounter.waves.reserve(static_cast<size_t>(waves));
    for (int w = 0; w < waves; ++w) {
        EncounterWave wave{};
        wave.waveIndex    = w;
        wave.triggerDelay = (w == 0)
                          ? 0.0f
                          : BASE_DELAY + rng.rangeFloat(0.0f, 15.0f);
        wave.spawnRadius  = BASE_SPAWN_RADIUS + rng.rangeFloat(0.0f, 20.0f);

        // Ships per wave: later waves get slightly more.
        int shipsInWave = rng.range(MIN_SHIPS_PER_WAVE,
                                    MIN_SHIPS_PER_WAVE + w + 2);
        if (shipsInWave > MAX_SHIPS_PER_WAVE) shipsInWave = MAX_SHIPS_PER_WAVE;

        wave.ships.reserve(static_cast<size_t>(shipsInWave));
        for (int s = 0; s < shipsInWave; ++s) {
            HullClass hull = selectHullForWave(rng, w, secLevel);
            uint64_t shipSeed = deriveSeed(ctx.seed,
                    static_cast<uint64_t>(w * 1000 + s));
            PCGContext shipCtx{ shipSeed, ctx.version };
            GeneratedShip ship = ShipGenerator::generate(shipCtx, hull);
            totalBounty += bountyForShip(ship);
            wave.ships.push_back(ship);
        }

        encounter.totalShips += shipsInWave;
        encounter.waves.push_back(std::move(wave));
    }

    encounter.estimatedBounty  = totalBounty;
    encounter.difficultyRating = 1.0f - secLevel;  // lower sec → harder
    encounter.valid = !encounter.waves.empty() && encounter.totalShips > 0;

    return encounter;
}

float NPCEncounterGenerator::calculateBounty(const NPCEncounter& encounter) {
    float total = 0.0f;
    for (const auto& wave : encounter.waves) {
        for (const auto& ship : wave.ships) {
            total += bountyForShip(ship);
        }
    }
    return total;
}

// ── Internals ──────────────────────────────────────────────────────

NPCFaction NPCEncounterGenerator::selectFaction(DeterministicRNG& rng,
                                                float secLevel) {
    float roll = rng.nextFloat();

    if (secLevel >= 0.5f) {
        // High-sec: mostly pirates, occasional rogue drones.
        if (roll < 0.60f) return NPCFaction::Pirate;
        if (roll < 0.85f) return NPCFaction::Rogue;
        return NPCFaction::Mercenary;
    }

    if (secLevel > 0.2f) {
        // Low-sec: mix of pirates, mercs, and rogues.
        if (roll < 0.35f) return NPCFaction::Pirate;
        if (roll < 0.60f) return NPCFaction::Mercenary;
        if (roll < 0.80f) return NPCFaction::Rogue;
        return NPCFaction::Navy;
    }

    // Null-sec: sleepers, heavy pirates, navy patrols.
    if (roll < 0.30f) return NPCFaction::Sleeper;
    if (roll < 0.55f) return NPCFaction::Pirate;
    if (roll < 0.75f) return NPCFaction::Mercenary;
    if (roll < 0.90f) return NPCFaction::Rogue;
    return NPCFaction::Navy;
}

HullClass NPCEncounterGenerator::selectHullForWave(DeterministicRNG& rng,
                                                    int waveIndex,
                                                    float secLevel) {
    float roll = rng.nextFloat();

    // Later waves and lower sec spawn bigger ships.
    float escalation = static_cast<float>(waveIndex) * 0.1f
                     + (1.0f - secLevel) * 0.3f;

    float adjustedRoll = roll - escalation;

    if (adjustedRoll < 0.25f) return HullClass::Battlecruiser;
    if (adjustedRoll < 0.50f) return HullClass::Cruiser;
    if (adjustedRoll < 0.70f) return HullClass::Destroyer;
    return HullClass::Frigate;
}

float NPCEncounterGenerator::bountyForShip(const GeneratedShip& ship) {
    switch (ship.hullClass) {
        case HullClass::Frigate:       return 5000.0f;
        case HullClass::Destroyer:     return 15000.0f;
        case HullClass::Cruiser:       return 50000.0f;
        case HullClass::Battlecruiser: return 150000.0f;
        case HullClass::Battleship:    return 500000.0f;
        case HullClass::Capital:       return 2000000.0f;
    }
    return 0.0f;
}

} // namespace pcg
} // namespace atlas
