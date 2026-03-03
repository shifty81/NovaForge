#pragma once
/**
 * @file multi_selection_manager.h
 * @brief Multi-entity selection manager for the editor.
 *
 * MultiSelectionManager tracks which entities the designer has
 * selected, supporting single-click, shift-click (toggle), and
 * select-all / clear workflows.  Bulk helpers provide the selected
 * set as a sorted vector for deterministic iteration.
 *
 * This is purely editor-side state — it does not interact with the
 * DeltaEditStore directly.  Bulk operations that modify entities
 * should flow through the UndoableCommandBus.
 */

#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace atlas::editor {

/**
 * MultiSelectionManager — tracks the editor's selected entity set.
 *
 * Usage:
 *   MultiSelectionManager sel;
 *   sel.Select(1);
 *   sel.Select(2);
 *   sel.Toggle(1);     // deselects 1
 *   auto ids = sel.SelectedSorted();
 */
class MultiSelectionManager {
public:
    /** Add an entity to the selection.  No-op if already selected. */
    void Select(uint32_t entityID) {
        m_selected.insert(entityID);
    }

    /** Remove an entity from the selection.  Returns false if not found. */
    bool Deselect(uint32_t entityID) {
        return m_selected.erase(entityID) > 0;
    }

    /** Toggle selection state for an entity.  Returns true if now selected. */
    bool Toggle(uint32_t entityID) {
        auto it = m_selected.find(entityID);
        if (it != m_selected.end()) {
            m_selected.erase(it);
            return false;
        }
        m_selected.insert(entityID);
        return true;
    }

    /** Whether a specific entity is currently selected. */
    bool IsSelected(uint32_t entityID) const {
        return m_selected.count(entityID) > 0;
    }

    /** Number of currently selected entities. */
    size_t Count() const { return m_selected.size(); }

    /** Whether the selection is empty. */
    bool Empty() const { return m_selected.empty(); }

    /** Clear the entire selection. */
    void ClearSelection() { m_selected.clear(); }

    /**
     * Replace the current selection with a list of entity IDs.
     * Useful for box-select or programmatic selection.
     */
    void SetSelection(const std::vector<uint32_t>& entities) {
        m_selected.clear();
        m_selected.insert(entities.begin(), entities.end());
    }

    /**
     * Add multiple entities to the current selection.
     */
    void SelectMultiple(const std::vector<uint32_t>& entities) {
        m_selected.insert(entities.begin(), entities.end());
    }

    /**
     * Get the selected entity IDs sorted in ascending order.
     * Deterministic ordering is useful for batch commands.
     */
    std::vector<uint32_t> SelectedSorted() const {
        std::vector<uint32_t> result(m_selected.begin(), m_selected.end());
        std::sort(result.begin(), result.end());
        return result;
    }

    /** Raw read-only access to the selection set. */
    const std::unordered_set<uint32_t>& Selected() const { return m_selected; }

private:
    std::unordered_set<uint32_t> m_selected;
};

} // namespace atlas::editor
