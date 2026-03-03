#pragma once
/**
 * @file asset_stats_panel.h
 * @brief Panel that aggregates and displays asset hierarchy and usage stats.
 *
 * AssetStatsPanel collects lightweight per-entity metrics (type, memory
 * estimate, collision count, physics body count) and exposes query
 * helpers so the editor can render a stats overlay or optimisation
 * report.  The panel is purely read-only editor state and does not
 * modify the DeltaEditStore.
 */

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::editor {

/**
 * Stats record for a single asset / entity.
 */
struct AssetStats {
    uint32_t    entityID       = 0;
    std::string assetType;          // e.g. "ship", "prop", "character"
    std::string name;               // human-readable label
    uint32_t    memoryBytes    = 0;
    uint32_t    collisionCount = 0;
    uint32_t    physicsBodyCount = 0;
    uint32_t    childCount     = 0; // direct children in hierarchy
};

/**
 * AssetStatsPanel — aggregates per-entity metrics for the editor.
 *
 * Usage:
 *   AssetStatsPanel panel;
 *   panel.AddAsset({1, "ship", "Frigate", 102400, 12, 3, 5});
 *   panel.AddAsset({2, "prop", "Crate",     4096,  1, 1, 0});
 *   auto total = panel.TotalMemory();
 *   auto ships = panel.FilterByType("ship");
 */
class AssetStatsPanel {
public:
    /** Register a new asset stat entry. */
    void AddAsset(const AssetStats& stats) {
        m_assets[stats.entityID] = stats;
    }

    /** Remove an asset by entity ID.  Returns false if not found. */
    bool RemoveAsset(uint32_t entityID) {
        return m_assets.erase(entityID) > 0;
    }

    /** Update an existing asset's stats.  Returns false if not found. */
    bool UpdateAsset(const AssetStats& stats) {
        auto it = m_assets.find(stats.entityID);
        if (it == m_assets.end()) return false;
        it->second = stats;
        return true;
    }

    /** Get stats for a specific entity.  Returns nullptr if not found. */
    const AssetStats* Get(uint32_t entityID) const {
        auto it = m_assets.find(entityID);
        if (it == m_assets.end()) return nullptr;
        return &it->second;
    }

    /** Total number of tracked assets. */
    size_t Count() const { return m_assets.size(); }

    /** Total estimated memory across all tracked assets. */
    uint64_t TotalMemory() const {
        uint64_t sum = 0;
        for (const auto& [id, s] : m_assets)
            sum += s.memoryBytes;
        return sum;
    }

    /** Total collision shapes across all tracked assets. */
    uint32_t TotalCollisions() const {
        uint32_t sum = 0;
        for (const auto& [id, s] : m_assets)
            sum += s.collisionCount;
        return sum;
    }

    /** Total physics bodies across all tracked assets. */
    uint32_t TotalPhysicsBodies() const {
        uint32_t sum = 0;
        for (const auto& [id, s] : m_assets)
            sum += s.physicsBodyCount;
        return sum;
    }

    /** Filter assets by type (e.g. "ship", "prop"). */
    std::vector<const AssetStats*> FilterByType(const std::string& type) const {
        std::vector<const AssetStats*> result;
        for (const auto& [id, s] : m_assets)
            if (s.assetType == type)
                result.push_back(&s);
        return result;
    }

    /** Get all assets sorted by memory usage (descending). */
    std::vector<const AssetStats*> SortedByMemory() const {
        std::vector<const AssetStats*> all;
        all.reserve(m_assets.size());
        for (const auto& [id, s] : m_assets)
            all.push_back(&s);
        std::sort(all.begin(), all.end(),
                  [](const AssetStats* a, const AssetStats* b) {
                      return a->memoryBytes > b->memoryBytes;
                  });
        return all;
    }

    /** Collect unique asset types present. */
    std::vector<std::string> UniqueTypes() const {
        std::unordered_map<std::string, bool> seen;
        for (const auto& [id, s] : m_assets)
            seen[s.assetType] = true;
        std::vector<std::string> types;
        types.reserve(seen.size());
        for (const auto& [t, _] : seen)
            types.push_back(t);
        return types;
    }

    /** Clear all tracked assets. */
    void Clear() { m_assets.clear(); }

private:
    std::unordered_map<uint32_t, AssetStats> m_assets;
};

} // namespace atlas::editor
