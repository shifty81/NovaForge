#include "editor/undoable_command_bus.h"

namespace atlas::editor {

void UndoableCommandBus::PostCommand(std::unique_ptr<ICommand> cmd) {
    if (cmd) {
        m_queue.push(std::move(cmd));
    }
}

void UndoableCommandBus::ProcessCommands() {
    while (!m_queue.empty()) {
        auto cmd = std::move(m_queue.front());
        m_queue.pop();

        // Try to cast to IUndoableCommand for undo recording.
        auto* undoable = dynamic_cast<IUndoableCommand*>(cmd.get());
        cmd->Execute();

        if (undoable) {
            // Transfer ownership to a shared_ptr so both undo and redo
            // closures can reference the same command.
            std::shared_ptr<ICommand> shared(std::move(cmd));
            auto* undoPtr = dynamic_cast<IUndoableCommand*>(shared.get());
            m_undoStack.PushAction(UndoAction{
                undoPtr->Description(),
                [shared, undoPtr]() { undoPtr->Undo(); },
                [shared, undoPtr]() { undoPtr->Execute(); }
            });
        }
    }
}

size_t UndoableCommandBus::PendingCount() const {
    return m_queue.size();
}

bool UndoableCommandBus::Undo() {
    return m_undoStack.Undo();
}

bool UndoableCommandBus::Redo() {
    return m_undoStack.Redo();
}

} // namespace atlas::editor
