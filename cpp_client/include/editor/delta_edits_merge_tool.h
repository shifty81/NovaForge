#pragma once
/**
 * @file delta_edits_merge_tool.h
 * @brief Concrete ITool for three-way merging of DeltaEditStore instances.
 *
 * DeltaEditsMergeTool detects conflicts between two DeltaEditStore
 * instances (edits targeting the same entityID + propertyName but
 * different values), supports automatic merging of non-conflicting
 * edits, and exposes conflicts for manual resolution.  Every merge
 * operation is posted to the UndoableCommandBus so it can be undone.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <cstdint>

namespace atlas::editor {

/**
 * Describes a single merge conflict between two DeltaEditStore instances.
 */
struct MergeConflict {
    uint32_t    entityID     = 0;
    std::string propertyName;
    std::string oursValue;
    std::string theirsValue;
    bool        resolved     = false;
    bool        acceptTheirs = false;
};

/**
 * Undoable command: merge a batch of DeltaEdits into the store.
 *
 * Execute appends every edit in the batch.  Undo removes the same
 * number of trailing edits that were appended (by clearing and
 * re-recording the edits that existed before the merge).
 */
class MergeEditsCommand : public IUndoableCommand {
public:
    MergeEditsCommand(atlas::ecs::DeltaEditStore& store,
                      std::vector<atlas::ecs::DeltaEdit> editsToMerge)
        : m_store(store), m_editsToMerge(std::move(editsToMerge)) {}

    void Execute() override {
        m_countBefore = m_store.Count();
        for (const auto& edit : m_editsToMerge) {
            m_store.Record(edit);
        }
    }

    void Undo() override {
        // Restore the store to its state before the merge by
        // re-recording only the edits that existed prior.
        const auto& all = m_store.Edits();
        std::vector<atlas::ecs::DeltaEdit> keep(all.begin(),
                                                  all.begin() + m_countBefore);
        m_store.Clear();
        for (const auto& edit : keep) {
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Merge DeltaEdits"; }

private:
    atlas::ecs::DeltaEditStore&          m_store;
    std::vector<atlas::ecs::DeltaEdit>   m_editsToMerge;
    size_t                               m_countBefore = 0;
};

/**
 * DeltaEditsMergeTool — concrete ITool for three-way DeltaEdit merging.
 *
 * Provides helpers to detect conflicts between two DeltaEditStore
 * instances, merge non-conflicting edits automatically, force-merge
 * all edits (theirs wins), or merge with manually-resolved conflicts.
 * All merge operations are recorded via UndoableCommandBus.
 */
class DeltaEditsMergeTool : public ITool {
public:
    DeltaEditsMergeTool(UndoableCommandBus& bus,
                        atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "DeltaEdits Merge"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Conflict detection ──────────────────────────────────────────

    /** Build the latest SetProperty value per (entityID, propertyName). */
    std::vector<MergeConflict> DetectConflicts(
            const atlas::ecs::DeltaEditStore& theirs) const {
        using Key = std::pair<uint32_t, std::string>;
        auto oursMap   = BuildPropertyMap(m_store);
        auto theirsMap = BuildPropertyMap(theirs);

        std::vector<MergeConflict> conflicts;
        for (const auto& [key, theirVal] : theirsMap) {
            auto it = oursMap.find(key);
            if (it != oursMap.end() && it->second != theirVal) {
                MergeConflict c;
                c.entityID     = key.first;
                c.propertyName = key.second;
                c.oursValue    = it->second;
                c.theirsValue  = theirVal;
                conflicts.push_back(std::move(c));
            }
        }
        return conflicts;
    }

    /** Number of conflicts between ours and theirs. */
    size_t ConflictCount(const atlas::ecs::DeltaEditStore& theirs) const {
        return DetectConflicts(theirs).size();
    }

    // ── Merge helpers that post undoable commands ───────────────────

    /** Merge only the edits from theirs that do NOT conflict with ours. */
    void MergeNonConflicting(const atlas::ecs::DeltaEditStore& theirs) {
        auto conflictKeys = BuildConflictKeySet(theirs);
        std::vector<atlas::ecs::DeltaEdit> toMerge;
        for (const auto& edit : theirs.Edits()) {
            if (edit.type == atlas::ecs::DeltaEditType::SetProperty) {
                auto key = std::make_pair(edit.entityID, edit.propertyName);
                if (conflictKeys.count(key)) continue;
            }
            toMerge.push_back(edit);
        }
        if (!toMerge.empty()) {
            m_bus.PostCommand(std::make_unique<MergeEditsCommand>(
                m_store, std::move(toMerge)));
        }
    }

    /** Merge ALL edits from theirs (theirs wins on conflicts). */
    void MergeAll(const atlas::ecs::DeltaEditStore& theirs) {
        std::vector<atlas::ecs::DeltaEdit> toMerge(theirs.Edits().begin(),
                                                     theirs.Edits().end());
        if (!toMerge.empty()) {
            m_bus.PostCommand(std::make_unique<MergeEditsCommand>(
                m_store, std::move(toMerge)));
        }
    }

    /**
     * Merge with manually-resolved conflicts.
     *
     * Non-conflicting edits from theirs are always included.  For each
     * resolved conflict where acceptTheirs is true the theirs edit is
     * included; otherwise it is skipped (ours value stays).
     */
    void MergeResolved(const std::vector<MergeConflict>& conflicts,
                       const atlas::ecs::DeltaEditStore& theirs) {
        // Build a set of conflict keys and their resolution.
        using ResKey = std::pair<uint32_t, std::string>;
        std::map<ResKey, const MergeConflict*> resolutionMap;
        for (const auto& c : conflicts) {
            resolutionMap[{c.entityID, c.propertyName}] = &c;
        }

        std::vector<atlas::ecs::DeltaEdit> toMerge;
        for (const auto& edit : theirs.Edits()) {
            if (edit.type == atlas::ecs::DeltaEditType::SetProperty) {
                auto key = std::make_pair(edit.entityID, edit.propertyName);
                auto it = resolutionMap.find(key);
                if (it != resolutionMap.end()) {
                    // This edit is part of a conflict.
                    if (it->second->resolved && it->second->acceptTheirs) {
                        toMerge.push_back(edit);
                    }
                    // Otherwise skip — keep ours.
                    continue;
                }
            }
            toMerge.push_back(edit);
        }
        if (!toMerge.empty()) {
            m_bus.PostCommand(std::make_unique<MergeEditsCommand>(
                m_store, std::move(toMerge)));
        }
    }

private:
    UndoableCommandBus&          m_bus;
    atlas::ecs::DeltaEditStore&  m_store;
    bool                         m_active = false;

    using Key = std::pair<uint32_t, std::string>;
    using PropertyMap = std::map<Key, std::string>;

    /** Latest SetProperty value per (entityID, propertyName). */
    static PropertyMap BuildPropertyMap(const atlas::ecs::DeltaEditStore& store) {
        PropertyMap pm;
        for (const auto& edit : store.Edits()) {
            if (edit.type == atlas::ecs::DeltaEditType::SetProperty) {
                pm[{edit.entityID, edit.propertyName}] = edit.propertyValue;
            }
        }
        return pm;
    }

    /** Set of (entityID, propertyName) keys that are in conflict. */
    std::set<Key> BuildConflictKeySet(
            const atlas::ecs::DeltaEditStore& theirs) const {
        auto conflicts = DetectConflicts(theirs);
        std::set<Key> keys;
        for (const auto& c : conflicts) {
            keys.emplace(c.entityID, c.propertyName);
        }
        return keys;
    }
};

} // namespace atlas::editor
