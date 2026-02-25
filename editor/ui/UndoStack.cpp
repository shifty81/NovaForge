#include "UndoStack.h"

namespace atlas::editor {

void UndoStack::PushAction(UndoAction action) {
    // Discard any redo future beyond the current position.
    if (m_undoIndex < m_actions.size()) {
        m_actions.erase(m_actions.begin() + static_cast<ptrdiff_t>(m_undoIndex),
                        m_actions.end());
    }

    m_actions.push_back(std::move(action));
    m_undoIndex = m_actions.size();

    // Enforce depth limit by dropping the oldest action.
    if (m_actions.size() > m_maxDepth) {
        m_actions.erase(m_actions.begin());
        --m_undoIndex;
    }
}

bool UndoStack::Undo() {
    if (m_undoIndex == 0) return false;
    --m_undoIndex;
    if (m_actions[m_undoIndex].undo) {
        m_actions[m_undoIndex].undo();
    }
    return true;
}

bool UndoStack::Redo() {
    if (m_undoIndex >= m_actions.size()) return false;
    if (m_actions[m_undoIndex].redo) {
        m_actions[m_undoIndex].redo();
    }
    ++m_undoIndex;
    return true;
}

void UndoStack::Clear() {
    m_actions.clear();
    m_undoIndex = 0;
}

std::string UndoStack::UndoDescription() const {
    if (m_undoIndex == 0) return "";
    return m_actions[m_undoIndex - 1].description;
}

std::string UndoStack::RedoDescription() const {
    if (m_undoIndex >= m_actions.size()) return "";
    return m_actions[m_undoIndex].description;
}

} // namespace atlas::editor
