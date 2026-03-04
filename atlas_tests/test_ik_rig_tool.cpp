/**
 * Tests for IKRigTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - AddChain / RemoveChain post commands, record AddObject / RemoveObject edits
 *   - SetTarget records MoveObject edits with correct position
 *   - SetConstraint records SetProperty edits with "chainName.constraintName"
 *   - SetSolverIterations / SetSolverTolerance record SetProperty edits
 *   - Undo reverses all operations correctly
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/ik_rig_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// IKRigTool tests
// ══════════════════════════════════════════════════════════════════

void test_ik_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    IKRigTool tool(bus, store);
    assert(std::string(tool.Name()) == "IK Rig");
}

void test_ik_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    IKRigTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_ik_add_chain() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.AddChain(1, "left_arm");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::AddObject);
    assert(edit.entityID == 1);
    assert(edit.objectType == "ik_chain");
    assert(edit.propertyName == "left_arm");
}

void test_ik_add_chain_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.AddChain(1, "left_arm");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].type == DeltaEditType::RemoveObject);
    assert(store.Edits()[1].objectType == "ik_chain");
    assert(store.Edits()[1].propertyName == "left_arm");
}

void test_ik_remove_chain() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.RemoveChain(1, "right_leg");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::RemoveObject);
    assert(edit.entityID == 1);
    assert(edit.objectType == "ik_chain");
    assert(edit.propertyName == "right_leg");
}

void test_ik_remove_chain_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.RemoveChain(1, "right_leg");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].type == DeltaEditType::AddObject);
    assert(store.Edits()[1].objectType == "ik_chain");
    assert(store.Edits()[1].propertyName == "right_leg");
}

void test_ik_set_target() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.SetTarget(1, "left_arm", 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::MoveObject);
    assert(edit.entityID == 1);
    assert(edit.objectType == "ik_chain");
    assert(edit.propertyName == "left_arm");
    assert(edit.position[0] == 1.0f);
    assert(edit.position[1] == 2.0f);
    assert(edit.position[2] == 3.0f);
}

void test_ik_set_target_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.SetTarget(1, "left_arm", 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f);
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].type == DeltaEditType::MoveObject);
    assert(store.Edits()[1].position[0] == 0.0f);
    assert(store.Edits()[1].position[1] == 0.0f);
    assert(store.Edits()[1].position[2] == 0.0f);
}

void test_ik_set_constraint() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.SetConstraint(1, "left_arm", "max_angle", "90", "45");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.objectType == "ik_chain");
    assert(edit.propertyName == "left_arm.max_angle");
    assert(edit.propertyValue == "45");
}

void test_ik_set_constraint_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.SetConstraint(1, "left_arm", "max_angle", "90", "45");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].type == DeltaEditType::SetProperty);
    assert(store.Edits()[1].propertyName == "left_arm.max_angle");
    assert(store.Edits()[1].propertyValue == "90");
}

void test_ik_solver_iterations() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.SetSolverIterations(1, "10", "20");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "solver_iterations");
    assert(edit.propertyValue == "20");
}

void test_ik_solver_tolerance() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    IKRigTool tool(bus, store);

    tool.SetSolverTolerance(1, "0.01", "0.001");
    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "solver_tolerance");
    assert(edit.propertyValue == "0.001");
}
