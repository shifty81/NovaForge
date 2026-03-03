#pragma once
/**
 * @file undoable_command_bus.h
 * @brief Command bus that automatically records undo/redo for every command.
 *
 * Wraps EditorCommandBus and UndoStack so that each executed command
 * is automatically pushed onto the undo stack.  Commands must implement
 * both Execute and Undo to participate; commands without Undo are
 * executed but not recorded.
 */

#include "editor_command_bus.h"
#include "../../editor/ui/UndoStack.h"
#include <memory>
#include <queue>

namespace atlas::editor {

/**
 * Extended command interface that supports undo.
 *
 * Commands that implement this interface get automatic undo/redo
 * recording when processed through UndoableCommandBus.
 */
struct IUndoableCommand : public ICommand {
    /** Reverse the effect of Execute(). */
    virtual void Undo() = 0;
};

/**
 * Command bus with integrated undo/redo history.
 *
 * Usage:
 *   UndoableCommandBus bus;
 *   bus.PostCommand(std::make_unique<MyUndoableCmd>(...));
 *   bus.ProcessCommands();  // executes & records in UndoStack
 *   bus.Undo();             // reverts last command
 *   bus.Redo();             // re-applies last undone command
 */
class UndoableCommandBus {
public:
    explicit UndoableCommandBus(size_t maxUndoDepth = 64)
        : m_undoStack(maxUndoDepth) {}

    /** Enqueue a command for deferred execution. */
    void PostCommand(std::unique_ptr<ICommand> cmd);

    /**
     * Execute and drain all pending commands.
     * IUndoableCommand instances are recorded in the undo stack.
     * Plain ICommand instances are executed without recording.
     */
    void ProcessCommands();

    /** Number of commands waiting to be processed. */
    size_t PendingCount() const;

    /** Undo the most recent undoable command. Returns true on success. */
    bool Undo();

    /** Redo the most recently undone command. Returns true on success. */
    bool Redo();

    bool CanUndo() const { return m_undoStack.CanUndo(); }
    bool CanRedo() const { return m_undoStack.CanRedo(); }
    size_t UndoCount() const { return m_undoStack.UndoCount(); }
    size_t RedoCount() const { return m_undoStack.RedoCount(); }

    std::string UndoDescription() const { return m_undoStack.UndoDescription(); }
    std::string RedoDescription() const { return m_undoStack.RedoDescription(); }

    /** Clear undo/redo history. */
    void ClearHistory() { m_undoStack.Clear(); }

    /** Access the underlying UndoStack (read-only). */
    const UndoStack& GetUndoStack() const { return m_undoStack; }

private:
    std::queue<std::unique_ptr<ICommand>> m_queue;
    UndoStack m_undoStack;
};

} // namespace atlas::editor
