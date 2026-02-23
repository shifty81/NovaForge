#ifndef EVE_PCG_SALVAGE_SYSTEM_H
#define EVE_PCG_SALVAGE_SYSTEM_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Salvage categories ──────────────────────────────────────────────
enum class SalvageCategory : uint32_t {
    HullPlate,
    PowerCore,
    EngineComponent,
    WeaponModule,
    DataDrive,
    RareMaterial,
    FactionBlueprint,
    AlienArtifact,
};

// ── Salvage node data ───────────────────────────────────────────────
struct SalvageNode {
    int nodeId;
    SalvageCategory category;
    float value;           // base ISK value
    bool requiresScan;     // hidden until scanned
    bool requiresCutter;   // needs cutter tool to extract
    float posX, posY, posZ;
};

// ── Salvage field data ──────────────────────────────────────────────
struct SalvageField {
    uint64_t fieldId;
    std::vector<SalvageNode> nodes;
    int totalNodes;
    int hiddenNodes;
};

/**
 * @brief Procedural salvage/loot generation for wrecks and debris fields.
 *
 * Given a PCGContext the system produces a deterministic SalvageField
 * containing scattered salvage nodes with varying rarity and value.
 */
class SalvageSystem {
public:
    static SalvageField generateSalvageField(const PCGContext& ctx, int fieldSize);
    static SalvageNode generateSalvageNode(DeterministicRNG& rng, int nodeId);
    static float calculateTotalValue(const SalvageField& field);

private:
    static SalvageCategory selectCategory(DeterministicRNG& rng);
    static float rollValue(DeterministicRNG& rng, SalvageCategory category);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SALVAGE_SYSTEM_H
