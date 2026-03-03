/**
 * Tests for AnimationEditorTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetKeyframe posts command, records DeltaEdit with anim: prefix
 *   - SetKeyframe undo restores previous value
 *   - RemoveKeyframe posts command, records empty value
 *   - RemoveKeyframe undo restores previous value
 *   - SetBlendWeight posts command, records anim:blend: prefix
 *   - SetBlendWeight undo restores previous weight
 *   - Multiple keyframes on same entity
 *   - Multiple entities with different keyframes
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/animation_editor_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// AnimationEditorTool tests
// ══════════════════════════════════════════════════════════════════

void test_anim_editor_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    AnimationEditorTool tool(bus, store);
    assert(std::string(tool.Name()) == "Animation Editor");
}

void test_anim_editor_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    AnimationEditorTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_anim_editor_set_keyframe() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.Activate();
    tool.SetKeyframe(1, "spine", 0.5f, "rot=0,15,0");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    // property name should start with "anim:spine@"
    assert(edit.propertyName.substr(0, 11) == "anim:spine@");
    assert(edit.propertyValue == "rot=0,15,0");
}

void test_anim_editor_set_keyframe_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.SetKeyframe(1, "left_arm", 1.0f, "pos=0.2,0,0.1", "pos=0,0,0");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "pos=0.2,0,0.1");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "pos=0,0,0");
}

void test_anim_editor_remove_keyframe() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.RemoveKeyframe(1, "right_arm", 2.0f, "rot=30,0,0");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyValue == "");
}

void test_anim_editor_remove_keyframe_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.RemoveKeyframe(1, "head", 0.0f, "rot=10,0,0");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "rot=10,0,0");
}

void test_anim_editor_set_blend_weight() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.SetBlendWeight(1, "walk_run", "0.75");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "anim:blend:walk_run");
    assert(edit.propertyValue == "0.75");
}

void test_anim_editor_set_blend_weight_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.SetBlendWeight(1, "idle_combat", "0.6", "0.3");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "0.6");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "anim:blend:idle_combat");
    assert(store.Edits()[1].propertyValue == "0.3");
}

void test_anim_editor_multiple_keyframes() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.SetKeyframe(1, "spine", 0.0f, "rot=0,0,0");
    tool.SetKeyframe(1, "spine", 1.0f, "rot=0,45,0");
    tool.SetKeyframe(1, "left_leg", 0.5f, "rot=30,0,0");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 1);
    assert(store.Edits()[2].entityID == 1);
}

void test_anim_editor_multiple_entities() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    AnimationEditorTool tool(bus, store);

    tool.SetKeyframe(1, "spine", 0.0f, "rot=0,0,0");
    tool.SetKeyframe(2, "turret_mount", 0.5f, "rot=0,90,0");
    tool.SetBlendWeight(3, "cape_physics", "1.0");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[2].entityID == 3);
}
