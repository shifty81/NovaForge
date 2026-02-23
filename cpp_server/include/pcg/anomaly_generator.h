#ifndef EVE_PCG_ANOMALY_GENERATOR_H
#define EVE_PCG_ANOMALY_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Anomaly site types ──────────────────────────────────────────────
enum class AnomalySiteType : uint32_t {
    CombatSite,        ///< NPC hostiles, bounty rewards.
    GasSite,           ///< Harvestable gas clouds.
    RelicSite,         ///< Hacking mini-game, high-value loot.
    DataSite,          ///< Data-core / blueprint loot.
    OreSite,           ///< Rich asteroid cluster.
    WormholeSignature, ///< Leads to wormhole space.
};

// ── Difficulty ratings ──────────────────────────────────────────────
enum class AnomalyDifficulty : uint32_t {
    Trivial,   ///< Frigate-level.
    Easy,      ///< Destroyer-level.
    Medium,    ///< Cruiser-level.
    Hard,      ///< Battlecruiser-level.
    Extreme,   ///< Battleship-level.
};

// ── Positional element inside an anomaly ────────────────────────────
struct AnomalyNode {
    int   nodeId;
    float posX, posY, posZ;
    int   contentType;   ///< 0=hostile, 1=container, 2=cloud, 3=debris.
    float value;         ///< Estimated ISK value.
};

// ── Full generated anomaly site ─────────────────────────────────────
struct AnomalySite {
    uint64_t              siteId;
    AnomalySiteType       type;
    AnomalyDifficulty     difficulty;
    std::vector<AnomalyNode> nodes;
    float                 totalValue;
    float                 siteRadius;
    int                   waveCount;   ///< NPC spawn waves (combat sites).
    bool                  requiresScan; ///< Must be scanned to warp to.
};

/**
 * @brief Deterministic anomaly-site generator.
 *
 * Generates exploration / combat sites with layout, difficulty, and
 * reward scaling based on security level.  Deterministic: same context
 * always yields the same site.
 */
class AnomalyGenerator {
public:
    /**
     * @brief Generate an anomaly site.
     * @param ctx       PCG context.
     * @param secLevel  Security level 0.0 (null) – 1.0 (high-sec).
     */
    static AnomalySite generate(const PCGContext& ctx, float secLevel);

    /**
     * @brief Generate with explicit type override.
     * @param ctx       PCG context.
     * @param type      Desired site type.
     * @param secLevel  Security level.
     */
    static AnomalySite generate(const PCGContext& ctx,
                                AnomalySiteType type,
                                float secLevel);

    /** Calculate total loot value for a site. */
    static float calculateTotalValue(const AnomalySite& site);

private:
    static AnomalySiteType   selectSiteType(DeterministicRNG& rng, float secLevel);
    static AnomalyDifficulty selectDifficulty(DeterministicRNG& rng, float secLevel);
    static int               selectWaveCount(DeterministicRNG& rng,
                                             AnomalyDifficulty diff);
    static AnomalyNode       generateNode(DeterministicRNG& rng,
                                          int nodeId,
                                          AnomalySiteType type,
                                          float siteRadius);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_ANOMALY_GENERATOR_H
