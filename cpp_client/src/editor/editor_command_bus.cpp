#include "editor/editor_command_bus.h"

namespace atlas::editor {

void EditorCommandBus::PostCommand(std::unique_ptr<ICommand> cmd) {
    if (cmd) {
        m_queue.push(std::move(cmd));
    }
}

void EditorCommandBus::ProcessCommands() {
    while (!m_queue.empty()) {
        auto cmd = std::move(m_queue.front());
        m_queue.pop();
        cmd->Execute();
    }
}

size_t EditorCommandBus::PendingCount() const {
    return m_queue.size();
}

} // namespace atlas::editor
