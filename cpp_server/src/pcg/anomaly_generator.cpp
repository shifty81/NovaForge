#include "pcg/anomaly_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Constants ──────────────────────────────────────────────────────
static constexpr int   MIN_NODES      = 5;
static constexpr int   MAX_NODES      = 30;
static constexpr float BASE_RADIUS    = 50.0f;
static constexpr float TWO_PI         = 6.2831853f;

// ── Public API ─────────────────────────────────────────────────────

AnomalySite AnomalyGenerator::generate(const PCGContext& ctx,
                                       float secLevel) {
    DeterministicRNG rng(ctx.seed);
    AnomalySiteType type = selectSiteType(rng, secLevel);
    return generate(ctx, type, secLevel);
}

AnomalySite AnomalyGenerator::generate(const PCGContext& ctx,
                                       AnomalySiteType type,
                                       float secLevel) {
    DeterministicRNG rng(ctx.seed);
    // Consume the same first roll to keep RNG state consistent.
    (void)rng.nextFloat();

    AnomalySite site{};
    site.siteId      = ctx.seed;
    site.type        = type;
    site.difficulty  = selectDifficulty(rng, secLevel);
    site.siteRadius  = BASE_RADIUS + rng.rangeFloat(0.0f, 100.0f);
    site.waveCount   = (type == AnomalySiteType::CombatSite)
                     ? selectWaveCount(rng, site.difficulty)
                     : 0;

    // Sites in lower-sec are harder to find.
    site.requiresScan = (secLevel < 0.5f) || rng.chance(0.3f);

    // Node count scales with difficulty.
    int nodeCount = MIN_NODES + static_cast<int>(site.difficulty) * 3;
    if (nodeCount > MAX_NODES) nodeCount = MAX_NODES;
    nodeCount = rng.range(nodeCount, nodeCount + 5);
    if (nodeCount > MAX_NODES) nodeCount = MAX_NODES;

    site.nodes.reserve(static_cast<size_t>(nodeCount));
    float totalValue = 0.0f;
    for (int i = 0; i < nodeCount; ++i) {
        AnomalyNode node = generateNode(rng, i, type, site.siteRadius);
        totalValue += node.value;
        site.nodes.push_back(node);
    }
    site.totalValue = totalValue;

    return site;
}

float AnomalyGenerator::calculateTotalValue(const AnomalySite& site) {
    float total = 0.0f;
    for (const auto& n : site.nodes) {
        total += n.value;
    }
    return total;
}

// ── Internals ──────────────────────────────────────────────────────

AnomalySiteType AnomalyGenerator::selectSiteType(DeterministicRNG& rng,
                                                  float secLevel) {
    float roll = rng.nextFloat();

    if (secLevel >= 0.5f) {
        // High-sec: mostly combat and ore sites.
        if (roll < 0.35f) return AnomalySiteType::CombatSite;
        if (roll < 0.55f) return AnomalySiteType::OreSite;
        if (roll < 0.70f) return AnomalySiteType::DataSite;
        if (roll < 0.85f) return AnomalySiteType::RelicSite;
        if (roll < 0.95f) return AnomalySiteType::GasSite;
        return AnomalySiteType::WormholeSignature;
    }

    if (secLevel > 0.2f) {
        // Low-sec: balanced mix, more wormholes.
        if (roll < 0.25f) return AnomalySiteType::CombatSite;
        if (roll < 0.40f) return AnomalySiteType::OreSite;
        if (roll < 0.55f) return AnomalySiteType::GasSite;
        if (roll < 0.70f) return AnomalySiteType::DataSite;
        if (roll < 0.85f) return AnomalySiteType::RelicSite;
        return AnomalySiteType::WormholeSignature;
    }

    // Null-sec: rarer, more valuable sites.
    if (roll < 0.30f) return AnomalySiteType::CombatSite;
    if (roll < 0.45f) return AnomalySiteType::RelicSite;
    if (roll < 0.60f) return AnomalySiteType::GasSite;
    if (roll < 0.75f) return AnomalySiteType::DataSite;
    if (roll < 0.85f) return AnomalySiteType::OreSite;
    return AnomalySiteType::WormholeSignature;
}

AnomalyDifficulty AnomalyGenerator::selectDifficulty(DeterministicRNG& rng,
                                                      float secLevel) {
    float roll = rng.nextFloat();

    if (secLevel >= 0.5f) {
        if (roll < 0.40f) return AnomalyDifficulty::Trivial;
        if (roll < 0.75f) return AnomalyDifficulty::Easy;
        if (roll < 0.95f) return AnomalyDifficulty::Medium;
        return AnomalyDifficulty::Hard;
    }

    if (secLevel > 0.2f) {
        if (roll < 0.15f) return AnomalyDifficulty::Easy;
        if (roll < 0.50f) return AnomalyDifficulty::Medium;
        if (roll < 0.85f) return AnomalyDifficulty::Hard;
        return AnomalyDifficulty::Extreme;
    }

    // Null-sec
    if (roll < 0.10f) return AnomalyDifficulty::Medium;
    if (roll < 0.45f) return AnomalyDifficulty::Hard;
    return AnomalyDifficulty::Extreme;
}

int AnomalyGenerator::selectWaveCount(DeterministicRNG& rng,
                                       AnomalyDifficulty diff) {
    switch (diff) {
        case AnomalyDifficulty::Trivial: return rng.range(1, 2);
        case AnomalyDifficulty::Easy:    return rng.range(1, 3);
        case AnomalyDifficulty::Medium:  return rng.range(2, 4);
        case AnomalyDifficulty::Hard:    return rng.range(3, 5);
        case AnomalyDifficulty::Extreme: return rng.range(4, 6);
    }
    return 1;
}

AnomalyNode AnomalyGenerator::generateNode(DeterministicRNG& rng,
                                            int nodeId,
                                            AnomalySiteType type,
                                            float siteRadius) {
    AnomalyNode node{};
    node.nodeId = nodeId;

    // Position within a sphere.
    float angle = rng.rangeFloat(0.0f, TWO_PI);
    float dist  = rng.rangeFloat(0.0f, siteRadius);
    float elev  = rng.rangeFloat(-0.5f, 0.5f);
    node.posX = dist * std::cos(angle);
    node.posY = dist * elev;
    node.posZ = dist * std::sin(angle);

    // Content type depends on site type.
    switch (type) {
        case AnomalySiteType::CombatSite:
            node.contentType = rng.chance(0.7f) ? 0 : 1; // hostile or container
            node.value = rng.rangeFloat(500.0f, 5000.0f);
            break;
        case AnomalySiteType::GasSite:
            node.contentType = 2; // cloud
            node.value = rng.rangeFloat(1000.0f, 8000.0f);
            break;
        case AnomalySiteType::RelicSite:
            node.contentType = rng.chance(0.6f) ? 1 : 3; // container or debris
            node.value = rng.rangeFloat(2000.0f, 15000.0f);
            break;
        case AnomalySiteType::DataSite:
            node.contentType = 1; // container
            node.value = rng.rangeFloat(1500.0f, 10000.0f);
            break;
        case AnomalySiteType::OreSite:
            node.contentType = 3; // debris / ore cluster
            node.value = rng.rangeFloat(800.0f, 4000.0f);
            break;
        case AnomalySiteType::WormholeSignature:
            node.contentType = 3; // debris
            node.value = rng.rangeFloat(3000.0f, 20000.0f);
            break;
    }

    return node;
}

} // namespace pcg
} // namespace atlas
