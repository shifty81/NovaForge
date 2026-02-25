#pragma once
#include <functional>
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief A single undoable/redoable action.
 *
 * Each action stores a description, an undo callback, and a redo callback.
 * The editor records actions as the user makes changes; Undo() / Redo()
 * walk the history in reverse / forward order.
 */
struct UndoAction {
    std::string description;
    std::function<void()> undo;
    std::function<void()> redo;
};

/**
 * @brief Fixed-capacity undo/redo history stack.
 *
 * - PushAction() appends a new action and clears the redo future.
 * - Undo() reverts the most recent action.
 * - Redo() re-applies the most recently undone action.
 * - Clear() resets the stack.
 *
 * The stack is capped at a configurable depth (default 64).  When the
 * cap is exceeded the oldest action is discarded.
 */
class UndoStack {
public:
    explicit UndoStack(size_t maxDepth = 64) : m_maxDepth(maxDepth) {}

    /** Record a new undoable action.  Clears any redo future. */
    void PushAction(UndoAction action);

    /** Undo the most recent action.  Returns true if an action was undone. */
    bool Undo();

    /** Redo the most recently undone action.  Returns true if an action was redone. */
    bool Redo();

    /** Clear all history. */
    void Clear();

    /** Number of actions that can be undone. */
    size_t UndoCount() const { return m_undoIndex; }

    /** Number of actions that can be redone. */
    size_t RedoCount() const { return m_actions.size() - m_undoIndex; }

    /** True if Undo() would succeed. */
    bool CanUndo() const { return m_undoIndex > 0; }

    /** True if Redo() would succeed. */
    bool CanRedo() const { return m_undoIndex < m_actions.size(); }

    /** Description of the next action that Undo() would revert ("" if none). */
    std::string UndoDescription() const;

    /** Description of the next action that Redo() would re-apply ("" if none). */
    std::string RedoDescription() const;

    /** Maximum depth. */
    size_t MaxDepth() const { return m_maxDepth; }

private:
    std::vector<UndoAction> m_actions;
    size_t m_undoIndex = 0;   ///< Points past the last undoable action
    size_t m_maxDepth  = 64;
};

} // namespace atlas::editor
