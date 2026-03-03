#pragma once
/**
 * @file undoable_command_bus.h
 * @brief Command bus with integrated undo/redo support.
 *
 * Wraps EditorCommandBus and UndoStack so that commands implementing
 * IUndoableCommand are automatically recorded for undo/redo, while
 * plain ICommand instances are executed fire-and-forget.
 */

#include <memory>
#include <queue>
#include <string>
#include <vector>
#include "editor/editor_command_bus.h"

namespace atlas::editor {

/**
 * Extension of ICommand that supports undo.
 *
 * Any command posted to UndoableCommandBus that derives from this
 * interface will be recorded in the undo history automatically.
 */
struct IUndoableCommand : public ICommand {
    /** Revert the effect of Execute(). */
    virtual void Undo() = 0;
};

/**
 * Command bus that records IUndoableCommand instances for undo/redo.
 *
 * Usage:
 *   UndoableCommandBus bus;
 *   bus.PostCommand(std::make_unique<MyUndoableCmd>(args...));
 *   bus.ProcessCommands();  // executes & records
 *   bus.Undo();             // reverts
 *   bus.Redo();             // re-applies
 *
 * Plain ICommand objects are executed normally but not recorded.
 */
class UndoableCommandBus {
public:
    explicit UndoableCommandBus(size_t maxHistory = 64)
        : m_maxHistory(maxHistory) {}

    /** Enqueue a command for deferred execution. */
    void PostCommand(std::unique_ptr<ICommand> cmd);

    /** Execute and drain all pending commands in FIFO order. */
    void ProcessCommands();

    /** Number of commands waiting to be processed. */
    size_t PendingCount() const { return m_queue.size(); }

    // ── Undo / Redo ─────────────────────────────────────────────────

    /** Undo the most recently executed undoable command. */
    bool Undo();

    /** Redo the most recently undone command. */
    bool Redo();

    /** Whether Undo() would succeed. */
    bool CanUndo() const { return m_undoIndex > 0; }

    /** Whether Redo() would succeed. */
    bool CanRedo() const { return m_undoIndex < m_history.size(); }

    /** Number of undoable actions. */
    size_t UndoCount() const { return m_undoIndex; }

    /** Number of redoable actions. */
    size_t RedoCount() const { return m_history.size() - m_undoIndex; }

    /** Description of the next action that Undo() would revert. */
    std::string UndoDescription() const;

    /** Description of the next action that Redo() would re-apply. */
    std::string RedoDescription() const;

    /** Clear all undo/redo history. */
    void ClearHistory();

    /** Maximum undo history depth. */
    size_t MaxHistory() const { return m_maxHistory; }

private:
    std::queue<std::unique_ptr<ICommand>> m_queue;
    std::vector<std::unique_ptr<IUndoableCommand>> m_history;
    size_t m_undoIndex = 0;
    size_t m_maxHistory = 64;
};

} // namespace atlas::editor
