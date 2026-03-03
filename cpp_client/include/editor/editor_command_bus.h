#pragma once
/**
 * @file editor_command_bus.h
 * @brief Decoupled command queue for editor actions.
 *
 * Editor/Tooling systems post ICommand objects; the bus handles deferred
 * execution each frame.  This keeps editor logic decoupled from the
 * concrete subsystems it manipulates and provides a natural extension
 * point for undo/redo in the future.
 */

#include <memory>
#include <queue>
#include <string>

namespace atlas::editor {

/** Abstract command that the bus executes. */
struct ICommand {
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual const char* Description() const = 0;
};

/**
 * Simple FIFO command bus.
 *
 * Usage:
 *   bus.PostCommand(std::make_unique<MyCommand>(args...));
 *   // ... later, once per frame:
 *   bus.ProcessCommands();
 */
class EditorCommandBus {
public:
    /** Enqueue a command for deferred execution. */
    void PostCommand(std::unique_ptr<ICommand> cmd);

    /** Execute and drain all pending commands in FIFO order. */
    void ProcessCommands();

    /** Number of commands waiting to be processed. */
    size_t PendingCount() const;

private:
    std::queue<std::unique_ptr<ICommand>> m_queue;
};

} // namespace atlas::editor
