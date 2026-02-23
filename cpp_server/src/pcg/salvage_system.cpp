#include "pcg/salvage_system.h"

#include <algorithm>

namespace atlas {
namespace pcg {

// ── Public API ─────────────────────────────────────────────────────

SalvageField SalvageSystem::generateSalvageField(const PCGContext& ctx, int fieldSize) {
    DeterministicRNG rng(ctx.seed);

    int count = std::max(10, std::min(fieldSize, 50));

    SalvageField field{};
    field.fieldId    = ctx.seed;
    field.totalNodes = count;
    field.hiddenNodes = 0;

    field.nodes.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        SalvageNode node = generateSalvageNode(rng, i);
        if (node.requiresScan) {
            ++field.hiddenNodes;
        }
        field.nodes.push_back(node);
    }

    return field;
}

SalvageNode SalvageSystem::generateSalvageNode(DeterministicRNG& rng, int nodeId) {
    SalvageNode node{};
    node.nodeId        = nodeId;
    node.category      = selectCategory(rng);
    node.value         = rollValue(rng, node.category);
    node.requiresScan  = rng.chance(0.20f);
    node.requiresCutter = rng.chance(0.30f);
    node.posX          = rng.rangeFloat(-50.0f, 50.0f);
    node.posY          = rng.rangeFloat(-50.0f, 50.0f);
    node.posZ          = rng.rangeFloat(-50.0f, 50.0f);
    return node;
}

float SalvageSystem::calculateTotalValue(const SalvageField& field) {
    float total = 0.0f;
    for (const auto& node : field.nodes) {
        total += node.value;
    }
    return total;
}

// ── Internals ──────────────────────────────────────────────────────

SalvageCategory SalvageSystem::selectCategory(DeterministicRNG& rng) {
    // Distribution: 25% HullPlate, 20% PowerCore, 15% EngineComponent,
    // 15% WeaponModule, 10% DataDrive, 8% RareMaterial,
    // 5% FactionBlueprint, 2% AlienArtifact
    float roll = rng.nextFloat();
    if (roll < 0.25f) return SalvageCategory::HullPlate;
    if (roll < 0.45f) return SalvageCategory::PowerCore;
    if (roll < 0.60f) return SalvageCategory::EngineComponent;
    if (roll < 0.75f) return SalvageCategory::WeaponModule;
    if (roll < 0.85f) return SalvageCategory::DataDrive;
    if (roll < 0.93f) return SalvageCategory::RareMaterial;
    if (roll < 0.98f) return SalvageCategory::FactionBlueprint;
    return SalvageCategory::AlienArtifact;
}

float SalvageSystem::rollValue(DeterministicRNG& rng, SalvageCategory category) {
    switch (category) {
        case SalvageCategory::HullPlate:        return rng.rangeFloat(100.0f, 500.0f);
        case SalvageCategory::PowerCore:        return rng.rangeFloat(200.0f, 1000.0f);
        case SalvageCategory::EngineComponent:  return rng.rangeFloat(300.0f, 1500.0f);
        case SalvageCategory::WeaponModule:     return rng.rangeFloat(500.0f, 2000.0f);
        case SalvageCategory::DataDrive:        return rng.rangeFloat(800.0f, 3000.0f);
        case SalvageCategory::RareMaterial:     return rng.rangeFloat(2000.0f, 8000.0f);
        case SalvageCategory::FactionBlueprint: return rng.rangeFloat(5000.0f, 20000.0f);
        case SalvageCategory::AlienArtifact:    return rng.rangeFloat(10000.0f, 50000.0f);
    }
    return 0.0f;
}

} // namespace pcg
} // namespace atlas
