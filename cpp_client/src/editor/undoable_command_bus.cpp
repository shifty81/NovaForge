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

        cmd->Execute();

        // If the command is undoable, record it in history.
        auto* undoable = dynamic_cast<IUndoableCommand*>(cmd.get());
        if (undoable) {
            // Discard any redo future beyond the current position.
            if (m_undoIndex < m_history.size()) {
                m_history.erase(
                    m_history.begin() + static_cast<ptrdiff_t>(m_undoIndex),
                    m_history.end());
            }

            // Release from the unique_ptr<ICommand> and re-own as IUndoableCommand.
            cmd.release();
            m_history.emplace_back(undoable);
            m_undoIndex = m_history.size();

            // Enforce depth limit.
            if (m_history.size() > m_maxHistory) {
                m_history.erase(m_history.begin());
                --m_undoIndex;
            }
        }
    }
}

bool UndoableCommandBus::Undo() {
    if (m_undoIndex == 0) return false;
    --m_undoIndex;
    m_history[m_undoIndex]->Undo();
    return true;
}

bool UndoableCommandBus::Redo() {
    if (m_undoIndex >= m_history.size()) return false;
    m_history[m_undoIndex]->Execute();
    ++m_undoIndex;
    return true;
}

std::string UndoableCommandBus::UndoDescription() const {
    if (m_undoIndex == 0) return "";
    return m_history[m_undoIndex - 1]->Description();
}

std::string UndoableCommandBus::RedoDescription() const {
    if (m_undoIndex >= m_history.size()) return "";
    return m_history[m_undoIndex]->Description();
}

void UndoableCommandBus::ClearHistory() {
    m_history.clear();
    m_undoIndex = 0;
}

} // namespace atlas::editor
