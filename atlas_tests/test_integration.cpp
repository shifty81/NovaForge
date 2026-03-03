/**
 * Integration tests that validate multi-system interactions:
 *
 *   - UndoableSetComponentCommand + UndoableCommandBus
 *   - UndoableRemoveComponentCommand + UndoableCommandBus
 *   - Multi-step undo/redo across entity command types
 *   - EditorEventBus + GameStateManager (state-change events)
 *
 * These tests exercise the full editor architectural stack together,
 * ensuring that the pieces introduced in the cleanup roadmap compose
 * correctly.
 */

#include "../engine/ecs/EntityCommands.h"
#include "../engine/ecs/ECS.h"
#include "../engine/core/GameStateManager.h"
#include "../cpp_client/include/editor/undoable_command_bus.h"
#include "../cpp_client/include/editor/editor_event_bus.h"
#include <cassert>
#include <string>
#include <memory>

using namespace atlas::ecs;
using namespace atlas::editor;

// ── Test components (local to this TU) ──────────────────────────────

namespace {
struct Hull { float integrity = 100.0f; };
struct Speed { float value = 0.0f; };
} // namespace

// ── UndoableSetComponentCommand tests ───────────────────────────────

void test_uset_cmd_executes_and_records() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();

    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{75.0f}));
    bus.ProcessCommands();

    Hull* h = world.GetComponent<Hull>(eid);
    assert(h != nullptr);
    assert(h->integrity > 74.9f && h->integrity < 75.1f);
    assert(bus.CanUndo());
}

void test_uset_cmd_undo_removes_new() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();

    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{60.0f}));
    bus.ProcessCommands();
    assert(world.HasComponent<Hull>(eid));

    bus.Undo();
    assert(!world.HasComponent<Hull>(eid));
}

void test_uset_cmd_undo_restores_previous() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Hull>(eid, Hull{100.0f});

    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{50.0f}));
    bus.ProcessCommands();

    Hull* h = world.GetComponent<Hull>(eid);
    assert(h != nullptr && h->integrity > 49.9f && h->integrity < 50.1f);

    bus.Undo();
    h = world.GetComponent<Hull>(eid);
    assert(h != nullptr && h->integrity > 99.9f && h->integrity < 100.1f);
}

void test_uset_cmd_redo() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Hull>(eid, Hull{100.0f});

    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{25.0f}));
    bus.ProcessCommands();

    bus.Undo();
    bus.Redo();

    Hull* h = world.GetComponent<Hull>(eid);
    assert(h != nullptr && h->integrity > 24.9f && h->integrity < 25.1f);
}

void test_uset_cmd_description() {
    World world;
    UndoableSetComponentCommand<Hull> cmd(world, 1, Hull{});
    assert(std::string(cmd.Description()) == "Set Component (Undoable)");
}

// ── UndoableRemoveComponentCommand tests ────────────────────────────

void test_uremove_cmd_executes_and_records() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Speed>(eid, Speed{120.0f});

    bus.PostCommand(std::make_unique<UndoableRemoveComponentCommand<Speed>>(
        world, eid));
    bus.ProcessCommands();

    assert(!world.HasComponent<Speed>(eid));
    assert(bus.CanUndo());
}

void test_uremove_cmd_undo_restores() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Speed>(eid, Speed{80.0f});

    bus.PostCommand(std::make_unique<UndoableRemoveComponentCommand<Speed>>(
        world, eid));
    bus.ProcessCommands();
    assert(!world.HasComponent<Speed>(eid));

    bus.Undo();
    Speed* s = world.GetComponent<Speed>(eid);
    assert(s != nullptr && s->value > 79.9f && s->value < 80.1f);
}

void test_uremove_cmd_redo() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Speed>(eid, Speed{55.0f});

    bus.PostCommand(std::make_unique<UndoableRemoveComponentCommand<Speed>>(
        world, eid));
    bus.ProcessCommands();

    bus.Undo();
    assert(world.HasComponent<Speed>(eid));

    bus.Redo();
    assert(!world.HasComponent<Speed>(eid));
}

void test_uremove_cmd_noop_no_component() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();

    bus.PostCommand(std::make_unique<UndoableRemoveComponentCommand<Speed>>(
        world, eid));
    bus.ProcessCommands();

    // Command was recorded but undo should be safe (noop)
    bus.Undo();
    assert(!world.HasComponent<Speed>(eid));
}

void test_uremove_cmd_description() {
    World world;
    UndoableRemoveComponentCommand<Speed> cmd(world, 1);
    assert(std::string(cmd.Description()) == "Remove Component (Undoable)");
}

// ── Multi-step undo/redo integration ────────────────────────────────

void test_multi_step_undo_redo() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();

    // Step 1: Set Hull to 100
    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{100.0f}));
    bus.ProcessCommands();

    // Step 2: Set Speed to 50
    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Speed>>(
        world, eid, Speed{50.0f}));
    bus.ProcessCommands();

    // Step 3: Modify Hull to 80
    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{80.0f}));
    bus.ProcessCommands();

    assert(bus.UndoCount() == 3);

    // Verify current state
    Hull* h = world.GetComponent<Hull>(eid);
    Speed* s = world.GetComponent<Speed>(eid);
    assert(h != nullptr && h->integrity > 79.9f && h->integrity < 80.1f);
    assert(s != nullptr && s->value > 49.9f && s->value < 50.1f);

    // Undo step 3: Hull back to 100
    bus.Undo();
    h = world.GetComponent<Hull>(eid);
    assert(h != nullptr && h->integrity > 99.9f && h->integrity < 100.1f);

    // Undo step 2: Speed removed
    bus.Undo();
    assert(!world.HasComponent<Speed>(eid));

    // Undo step 1: Hull removed
    bus.Undo();
    assert(!world.HasComponent<Hull>(eid));

    // Redo all three steps
    bus.Redo(); // Hull = 100
    bus.Redo(); // Speed = 50
    bus.Redo(); // Hull = 80

    h = world.GetComponent<Hull>(eid);
    s = world.GetComponent<Speed>(eid);
    assert(h != nullptr && h->integrity > 79.9f && h->integrity < 80.1f);
    assert(s != nullptr && s->value > 49.9f && s->value < 50.1f);
}

void test_undo_set_then_remove() {
    World world;
    UndoableCommandBus bus;
    EntityID eid = world.CreateEntity();

    // Set Hull
    bus.PostCommand(std::make_unique<UndoableSetComponentCommand<Hull>>(
        world, eid, Hull{90.0f}));
    bus.ProcessCommands();

    // Remove Hull
    bus.PostCommand(std::make_unique<UndoableRemoveComponentCommand<Hull>>(
        world, eid));
    bus.ProcessCommands();

    assert(!world.HasComponent<Hull>(eid));

    // Undo remove → Hull restored to 90
    bus.Undo();
    Hull* h = world.GetComponent<Hull>(eid);
    assert(h != nullptr && h->integrity > 89.9f && h->integrity < 90.1f);

    // Undo set → Hull removed entirely
    bus.Undo();
    assert(!world.HasComponent<Hull>(eid));
}

// ── EditorEventBus + GameStateManager integration ───────────────────

void test_gsm_publishes_phase_change_via_event_bus() {
    atlas::GameStateManager gsm;
    EditorEventBus events;

    atlas::GamePhase receivedOld = atlas::GamePhase::MainMenu;
    atlas::GamePhase receivedNew = atlas::GamePhase::MainMenu;
    bool fired = false;

    // Wire GameStateManager phase change to EditorEventBus
    gsm.OnPhaseChange([&](atlas::GamePhase oldP, atlas::GamePhase newP) {
        events.Publish("game.phase_changed",
            std::make_pair(oldP, newP));
    });

    events.Subscribe("game.phase_changed",
        [&](const std::string& /*event*/, const std::any& payload) {
            auto pair = std::any_cast<
                std::pair<atlas::GamePhase, atlas::GamePhase>>(payload);
            receivedOld = pair.first;
            receivedNew = pair.second;
            fired = true;
        });

    gsm.SetPhase(atlas::GamePhase::InSpace);

    assert(fired);
    assert(receivedOld == atlas::GamePhase::MainMenu);
    assert(receivedNew == atlas::GamePhase::InSpace);
}

void test_gsm_credits_change_via_event_bus() {
    atlas::GameStateManager gsm;
    EditorEventBus events;

    int64_t reportedCredits = 0;
    bool fired = false;

    events.Subscribe("game.credits_changed",
        [&](const std::string& /*event*/, const std::any& payload) {
            reportedCredits = std::any_cast<int64_t>(payload);
            fired = true;
        });

    // Simulate a credit change with event publication
    gsm.SetCredits(1000);
    events.Publish("game.credits_changed", gsm.Credits());

    assert(fired);
    assert(reportedCredits == 1000);
}
