#pragma once
/**
 * @file visual_diff_tool.h
 * @brief Editor-side tool for comparing current DeltaEdits against the PCG baseline.
 *
 * VisualDiffTool analyzes the DeltaEditStore and produces a structured
 * summary of all modifications:  added objects, removed objects, moved
 * objects, and property changes.  This lets the designer see exactly
 * what has been modified since the last PCG generation, enabling
 * informed commit/rollback decisions.
 *
 * This is a read-only analysis tool — it does not modify the
 * DeltaEditStore or post commands.
 */

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../engine/ecs/DeltaEditStore.h"

namespace atlas::editor {

/**
 * A single diff entry representing one logical change.
 */
struct DiffEntry {
    atlas::ecs::DeltaEditType type;
    uint32_t    entityID    = 0;
    std::string objectType;
    std::string description;   ///< Human-readable summary
};

/**
 * Aggregated diff summary of all changes in a DeltaEditStore.
 */
struct DiffSummary {
    std::vector<DiffEntry> entries;
    uint32_t addedCount    = 0;
    uint32_t removedCount  = 0;
    uint32_t movedCount    = 0;
    uint32_t propertyCount = 0;

    uint32_t TotalChanges() const {
        return addedCount + removedCount + movedCount + propertyCount;
    }
};

/**
 * VisualDiffTool — computes a structured diff of DeltaEditStore changes.
 *
 * Usage:
 *   DeltaEditStore store(42);
 *   // ... record some edits ...
 *   VisualDiffTool diff;
 *   DiffSummary summary = diff.ComputeDiff(store);
 *   // summary.addedCount, summary.entries, etc.
 */
class VisualDiffTool {
public:
    /**
     * Compute a diff summary from all edits in the store.
     * Counts are categorized by DeltaEditType.
     */
    DiffSummary ComputeDiff(const atlas::ecs::DeltaEditStore& store) const {
        DiffSummary summary;
        for (const auto& edit : store.Edits()) {
            DiffEntry entry;
            entry.type     = edit.type;
            entry.entityID = edit.entityID;
            entry.objectType = edit.objectType;

            switch (edit.type) {
                case atlas::ecs::DeltaEditType::AddObject:
                    entry.description = "Added " + edit.objectType +
                                        " (entity " + std::to_string(edit.entityID) + ")";
                    summary.addedCount++;
                    break;
                case atlas::ecs::DeltaEditType::RemoveObject:
                    entry.description = "Removed entity " + std::to_string(edit.entityID);
                    summary.removedCount++;
                    break;
                case atlas::ecs::DeltaEditType::MoveObject:
                    entry.description = "Moved entity " + std::to_string(edit.entityID) +
                                        " to (" + std::to_string(edit.position[0]) + ", " +
                                        std::to_string(edit.position[1]) + ", " +
                                        std::to_string(edit.position[2]) + ")";
                    summary.movedCount++;
                    break;
                case atlas::ecs::DeltaEditType::SetProperty:
                    entry.description = "Set " + edit.propertyName + " = " +
                                        edit.propertyValue + " on entity " +
                                        std::to_string(edit.entityID);
                    summary.propertyCount++;
                    break;
            }
            summary.entries.push_back(std::move(entry));
        }
        return summary;
    }

    /**
     * Collect unique entity IDs that have been modified.
     */
    std::vector<uint32_t> AffectedEntities(const atlas::ecs::DeltaEditStore& store) const {
        std::unordered_map<uint32_t, bool> seen;
        for (const auto& edit : store.Edits())
            seen[edit.entityID] = true;
        std::vector<uint32_t> result;
        result.reserve(seen.size());
        for (const auto& [id, _] : seen)
            result.push_back(id);
        std::sort(result.begin(), result.end());
        return result;
    }

    /**
     * Filter entries for a specific entity.
     */
    std::vector<DiffEntry> EntriesForEntity(
            const DiffSummary& summary, uint32_t entityID) const {
        std::vector<DiffEntry> result;
        for (const auto& e : summary.entries)
            if (e.entityID == entityID)
                result.push_back(e);
        return result;
    }

    /**
     * Filter entries by edit type.
     */
    std::vector<DiffEntry> EntriesByType(
            const DiffSummary& summary,
            atlas::ecs::DeltaEditType type) const {
        std::vector<DiffEntry> result;
        for (const auto& e : summary.entries)
            if (e.type == type)
                result.push_back(e);
        return result;
    }
};

} // namespace atlas::editor
