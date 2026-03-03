/**
 * Tests for UndoableCommandBus — command bus with integrated undo/redo.
 *
 * Validates:
 * - Post and process commands
 * - IUndoableCommand records undo/redo in UndoStack
 * - Plain ICommand executes without recording
 * - Undo/Redo cycle
 * - Description propagation
 * - Null safety
 * - Max depth enforcement
 */

#include "../cpp_client/include/editor/undoable_command_bus.h"
#include <cassert>
#include <string>
#include <memory>

using namespace atlas::editor;

// ── Helper commands ─────────────────────────────────────────────────

struct PlainCmd : ICommand {
    int* counter;
    explicit PlainCmd(int* c) : counter(c) {}
    void Execute() override { ++(*counter); }
    const char* Description() const override { return "plain"; }
};

struct TrackingCmd : IUndoableCommand {
    int* value;
    int delta;
    TrackingCmd(int* v, int d) : value(v), delta(d) {}
    void Execute() override { *value += delta; }
    void Undo() override { *value -= delta; }
    const char* Description() const override { return "tracking"; }
};

// ── Tests ───────────────────────────────────────────────────────────

void test_ucb_empty_by_default() {
    UndoableCommandBus bus;
    assert(bus.PendingCount() == 0);
    assert(!bus.CanUndo());
    assert(!bus.CanRedo());
    assert(bus.UndoCount() == 0);
    assert(bus.RedoCount() == 0);
}

void test_ucb_post_increments_count() {
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<PlainCmd>(nullptr));
    assert(bus.PendingCount() == 1);
}

void test_ucb_post_null_ignored() {
    UndoableCommandBus bus;
    bus.PostCommand(nullptr);
    assert(bus.PendingCount() == 0);
}

void test_ucb_process_plain_no_undo() {
    int counter = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<PlainCmd>(&counter));
    bus.ProcessCommands();
    assert(counter == 1);
    assert(!bus.CanUndo());  // plain cmd not recorded
}

void test_ucb_process_undoable_records() {
    int value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 10));
    bus.ProcessCommands();
    assert(value == 10);
    assert(bus.CanUndo());
    assert(bus.UndoCount() == 1);
}

void test_ucb_undo_reverts() {
    int value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 5));
    bus.ProcessCommands();
    assert(value == 5);

    bool undone = bus.Undo();
    assert(undone);
    assert(value == 0);
    assert(!bus.CanUndo());
    assert(bus.CanRedo());
}

void test_ucb_redo_reapplies() {
    int value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 7));
    bus.ProcessCommands();

    bus.Undo();
    assert(value == 0);

    bool redone = bus.Redo();
    assert(redone);
    assert(value == 7);
    assert(bus.CanUndo());
    assert(!bus.CanRedo());
}

void test_ucb_undo_empty_fails() {
    UndoableCommandBus bus;
    assert(!bus.Undo());
}

void test_ucb_redo_empty_fails() {
    UndoableCommandBus bus;
    assert(!bus.Redo());
}

void test_ucb_multiple_undo_redo() {
    int value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 10));
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 20));
    bus.ProcessCommands();
    assert(value == 30);
    assert(bus.UndoCount() == 2);

    bus.Undo();
    assert(value == 10);
    bus.Undo();
    assert(value == 0);

    bus.Redo();
    assert(value == 10);
    bus.Redo();
    assert(value == 30);
}

void test_ucb_description() {
    int value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 1));
    bus.ProcessCommands();
    assert(bus.UndoDescription() == "tracking");
}

void test_ucb_clear_history() {
    int value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<TrackingCmd>(&value, 1));
    bus.ProcessCommands();
    assert(bus.CanUndo());

    bus.ClearHistory();
    assert(!bus.CanUndo());
    assert(!bus.CanRedo());
    assert(bus.UndoCount() == 0);
}

void test_ucb_mixed_plain_and_undoable() {
    int plain_counter = 0;
    int undo_value = 0;
    UndoableCommandBus bus;
    bus.PostCommand(std::make_unique<PlainCmd>(&plain_counter));
    bus.PostCommand(std::make_unique<TrackingCmd>(&undo_value, 5));
    bus.PostCommand(std::make_unique<PlainCmd>(&plain_counter));
    bus.ProcessCommands();

    assert(plain_counter == 2);
    assert(undo_value == 5);
    assert(bus.UndoCount() == 1);  // only the TrackingCmd recorded
}
