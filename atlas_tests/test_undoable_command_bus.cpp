/**
 * Tests for UndoableCommandBus.
 *
 * Validates:
 * - Basic post/process cycle
 * - IUndoableCommand auto-records into history
 * - Plain ICommand is fire-and-forget (not recorded)
 * - Undo reverts, Redo re-applies
 * - CanUndo / CanRedo / counts
 * - ClearHistory empties the undo stack
 * - Undo/Redo descriptions
 * - Undo clears redo future when new command posted
 * - Depth limit enforcement
 * - Null command safety
 * - Pending count
 * - Multiple undoable commands in sequence
 */

#include "../cpp_client/include/editor/undoable_command_bus.h"
#include <cassert>
#include <string>
#include <memory>

using namespace atlas::editor;

// ── Helpers ─────────────────────────────────────────────────────────

namespace {

/** Simple undoable command that increments/decrements a counter. */
class IncrementCommand : public IUndoableCommand {
public:
    explicit IncrementCommand(int& counter, int amount = 1)
        : m_counter(counter), m_amount(amount) {}

    void Execute() override { m_counter += m_amount; }
    void Undo() override    { m_counter -= m_amount; }
    const char* Description() const override { return "Increment"; }

private:
    int& m_counter;
    int m_amount;
};

/** Plain (non-undoable) command that sets a flag. */
class PlainCommand : public ICommand {
public:
    explicit PlainCommand(bool& flag) : m_flag(flag) {}
    void Execute() override { m_flag = true; }
    const char* Description() const override { return "Plain"; }
private:
    bool& m_flag;
};

} // namespace

// ── Tests ───────────────────────────────────────────────────────────

void test_ucb_post_and_process() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    assert(bus.PendingCount() == 1);
    bus.ProcessCommands();
    assert(counter == 1);
    assert(bus.PendingCount() == 0);
}

void test_ucb_undoable_recorded() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    bus.ProcessCommands();
    assert(bus.UndoCount() == 1);
    assert(bus.CanUndo());
}

void test_ucb_plain_not_recorded() {
    UndoableCommandBus bus;
    bool flag = false;
    bus.PostCommand(std::make_unique<PlainCommand>(flag));
    bus.ProcessCommands();
    assert(flag);
    assert(bus.UndoCount() == 0);
    assert(!bus.CanUndo());
}

void test_ucb_undo() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 5));
    bus.ProcessCommands();
    assert(counter == 5);

    assert(bus.Undo());
    assert(counter == 0);
    assert(!bus.CanUndo());
}

void test_ucb_redo() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 3));
    bus.ProcessCommands();
    bus.Undo();
    assert(counter == 0);

    assert(bus.Redo());
    assert(counter == 3);
    assert(!bus.CanRedo());
}

void test_ucb_undo_redo_counts() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    bus.ProcessCommands();
    assert(bus.UndoCount() == 2);
    assert(bus.RedoCount() == 0);

    bus.Undo();
    assert(bus.UndoCount() == 1);
    assert(bus.RedoCount() == 1);
}

void test_ucb_clear_history() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    bus.ProcessCommands();
    bus.ClearHistory();
    assert(bus.UndoCount() == 0);
    assert(bus.RedoCount() == 0);
    assert(!bus.CanUndo());
}

void test_ucb_undo_description() {
    UndoableCommandBus bus;
    int counter = 0;
    assert(bus.UndoDescription().empty());
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    bus.ProcessCommands();
    assert(bus.UndoDescription() == "Increment");
}

void test_ucb_redo_description() {
    UndoableCommandBus bus;
    int counter = 0;
    assert(bus.RedoDescription().empty());
    bus.PostCommand(std::make_unique<IncrementCommand>(counter));
    bus.ProcessCommands();
    bus.Undo();
    assert(bus.RedoDescription() == "Increment");
}

void test_ucb_new_command_clears_redo() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 1));
    bus.ProcessCommands();
    bus.Undo();
    assert(bus.CanRedo());

    // Posting a new command should discard the redo future.
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 10));
    bus.ProcessCommands();
    assert(!bus.CanRedo());
    assert(counter == 10);
}

void test_ucb_depth_limit() {
    UndoableCommandBus bus(3);  // max 3 entries
    int counter = 0;
    for (int i = 0; i < 5; ++i) {
        bus.PostCommand(std::make_unique<IncrementCommand>(counter));
        bus.ProcessCommands();
    }
    assert(counter == 5);
    assert(bus.UndoCount() == 3);  // oldest 2 dropped
}

void test_ucb_null_command_ignored() {
    UndoableCommandBus bus;
    bus.PostCommand(nullptr);
    assert(bus.PendingCount() == 0);
    bus.ProcessCommands();
    assert(bus.UndoCount() == 0);
}

void test_ucb_multiple_undo_redo() {
    UndoableCommandBus bus;
    int counter = 0;
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 1));
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 2));
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 3));
    bus.ProcessCommands();
    assert(counter == 6);

    bus.Undo();  // undo +3
    assert(counter == 3);
    bus.Undo();  // undo +2
    assert(counter == 1);
    bus.Redo();  // redo +2
    assert(counter == 3);
    bus.Redo();  // redo +3
    assert(counter == 6);
}
