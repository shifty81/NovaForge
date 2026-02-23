#ifndef EVE_PCG_NPC_ENCOUNTER_GENERATOR_H
#define EVE_PCG_NPC_ENCOUNTER_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── NPC faction identifiers ─────────────────────────────────────────
enum class NPCFaction : uint32_t {
    Pirate,
    Navy,
    Rogue,
    Sleeper,
    Mercenary,
};

// ── Single wave of hostiles ─────────────────────────────────────────
struct EncounterWave {
    int                        waveIndex;
    std::vector<GeneratedShip> ships;
    float                      triggerDelay;   ///< Seconds after previous wave.
    float                      spawnRadius;    ///< Spawn spread (metres).
};

// ── Full NPC encounter ──────────────────────────────────────────────
struct NPCEncounter {
    uint64_t                     encounterId;
    NPCFaction                   faction;
    std::vector<EncounterWave>   waves;
    int                          totalShips;
    float                        estimatedBounty; ///< Total ISK reward.
    float                        difficultyRating; ///< 0.0 – 1.0 normalized.
    bool                         valid;
};

/**
 * @brief Deterministic NPC encounter generator.
 *
 * Creates multi-wave hostile encounters with faction-appropriate
 * ship compositions.  Difficulty and ship classes scale with the
 * security level (lower sec → harder fights, better loot).
 */
class NPCEncounterGenerator {
public:
    /**
     * @brief Generate an encounter with procedural wave count.
     * @param ctx       PCG context.
     * @param secLevel  Security level 0.0 (null) – 1.0 (high-sec).
     */
    static NPCEncounter generate(const PCGContext& ctx, float secLevel);

    /**
     * @brief Generate with explicit wave count.
     * @param ctx        PCG context.
     * @param waveCount  Number of hostile waves.
     * @param secLevel   Security level.
     */
    static NPCEncounter generate(const PCGContext& ctx,
                                 int waveCount,
                                 float secLevel);

    /** Compute total estimated bounty. */
    static float calculateBounty(const NPCEncounter& encounter);

private:
    static NPCFaction selectFaction(DeterministicRNG& rng, float secLevel);
    static HullClass  selectHullForWave(DeterministicRNG& rng,
                                        int waveIndex,
                                        float secLevel);
    static float      bountyForShip(const GeneratedShip& ship);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_NPC_ENCOUNTER_GENERATOR_H
