/**
 * Tests for LiveEditMode:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - Default policy is PauseOnEdit, default time scale is 1.0
 *   - SetPolicy changes policy and records DeltaEdit; undo restores old policy
 *   - SetTimeScale changes scale and records DeltaEdit; clamps to [0.0, 1.0]
 *   - BeginEdit/EndEdit pauses/resumes sim under PauseOnEdit
 *   - FullSpeed mode does not affect simulation during edits
 *   - EffectiveTimeScale varies by policy and editing state
 *   - EditCount increments with each BeginEdit
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/live_edit_mode.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// LiveEditMode tests
// ══════════════════════════════════════════════════════════════════

void test_live_edit_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    assert(std::string(tool.Name()) == "Live Edit Mode");
}

void test_live_edit_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_live_edit_default_policy() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    assert(tool.Policy() == LiveEditPolicy::PauseOnEdit);
}

void test_live_edit_set_policy() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    tool.SetPolicy(LiveEditPolicy::FullSpeed);
    bus.ProcessCommands();

    assert(tool.Policy() == LiveEditPolicy::FullSpeed);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 0);
    assert(edit.propertyName == "live_edit_policy");
    assert(edit.propertyValue == "FullSpeed");
}

void test_live_edit_set_policy_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    tool.SetPolicy(LiveEditPolicy::FullSpeed);
    bus.ProcessCommands();
    assert(tool.Policy() == LiveEditPolicy::FullSpeed);

    bus.Undo();
    assert(tool.Policy() == LiveEditPolicy::PauseOnEdit);
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "PauseOnEdit");
}

void test_live_edit_default_time_scale() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    assert(tool.TimeScale() == 1.0f);
}

void test_live_edit_set_time_scale() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    tool.SetTimeScale(0.5f);
    bus.ProcessCommands();

    assert(tool.TimeScale() == 0.5f);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 0);
    assert(edit.propertyName == "time_scale");
}

void test_live_edit_set_time_scale_clamp() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);

    // Above 1.0 should clamp to 1.0 — same as default, so no command posted
    tool.SetTimeScale(5.0f);
    assert(bus.PendingCount() == 0);

    // Below 0.0 should clamp to 0.0
    tool.SetTimeScale(-1.0f);
    bus.ProcessCommands();
    assert(tool.TimeScale() == 0.0f);
}

void test_live_edit_begin_end_pause() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    tool.Activate();

    assert(simCtrl.IsRunning());

    tool.BeginEdit();
    assert(simCtrl.IsPaused());

    tool.EndEdit();
    assert(simCtrl.IsRunning());
}

void test_live_edit_begin_end_fullspeed() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    tool.Activate();

    tool.SetPolicy(LiveEditPolicy::FullSpeed);
    bus.ProcessCommands();

    assert(simCtrl.IsRunning());
    tool.BeginEdit();
    assert(simCtrl.IsRunning());
    tool.EndEdit();
    assert(simCtrl.IsRunning());
}

void test_live_edit_effective_scale_pause() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    tool.Activate();

    // PauseOnEdit is the default
    tool.BeginEdit();
    assert(tool.EffectiveTimeScale() == 0.0f);
    tool.EndEdit();
}

void test_live_edit_effective_scale_slow() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    tool.Activate();

    tool.SetPolicy(LiveEditPolicy::SlowMotion);
    bus.ProcessCommands();

    // Default time scale is 1.0, so effective = 1.0 * 0.25 = 0.25
    tool.BeginEdit();
    assert(tool.EffectiveTimeScale() == 1.0f * 0.25f);
    tool.EndEdit();
}

void test_live_edit_effective_scale_full() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    tool.Activate();

    tool.SetPolicy(LiveEditPolicy::FullSpeed);
    bus.ProcessCommands();

    tool.BeginEdit();
    assert(tool.EffectiveTimeScale() == tool.TimeScale());
    tool.EndEdit();
}

void test_live_edit_edit_count() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    SimulationStepController simCtrl;
    LiveEditMode tool(bus, store, simCtrl);
    tool.Activate();

    assert(tool.EditCount() == 0);

    tool.BeginEdit();
    assert(tool.EditCount() == 1);
    tool.EndEdit();

    tool.BeginEdit();
    assert(tool.EditCount() == 2);
    tool.EndEdit();

    tool.BeginEdit();
    assert(tool.EditCount() == 3);
    tool.EndEdit();
}
