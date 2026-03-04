#pragma once
/**
 * @file ik_rig_tool.h
 * @brief Concrete ITool for inverse kinematics rigging.
 *
 * IKRigTool lets the designer manage IK chains, set target positions,
 * configure constraints, and tune solver parameters live.  Every
 * modification is posted to the UndoableCommandBus and recorded in the
 * DeltaEditStore so edits persist on top of the PCG seed.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <cstdint>

namespace atlas::editor {

/**
 * Undoable command: add an IK chain to an entity.
 */
class AddIKChainCommand : public IUndoableCommand {
public:
    AddIKChainCommand(atlas::ecs::DeltaEditStore& store,
                      uint32_t entityID,
                      const std::string& chainName)
        : m_store(store), m_entityID(entityID),
          m_chainName(chainName) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type       = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID   = m_entityID;
        edit.objectType = "ik_chain";
        edit.propertyName  = m_chainName;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type       = atlas::ecs::DeltaEditType::RemoveObject;
        edit.entityID   = m_entityID;
        edit.objectType = "ik_chain";
        edit.propertyName  = m_chainName;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Add IK Chain"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_chainName;
};

/**
 * Undoable command: remove an IK chain from an entity.
 */
class RemoveIKChainCommand : public IUndoableCommand {
public:
    RemoveIKChainCommand(atlas::ecs::DeltaEditStore& store,
                         uint32_t entityID,
                         const std::string& chainName)
        : m_store(store), m_entityID(entityID),
          m_chainName(chainName) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type       = atlas::ecs::DeltaEditType::RemoveObject;
        edit.entityID   = m_entityID;
        edit.objectType = "ik_chain";
        edit.propertyName  = m_chainName;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type       = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID   = m_entityID;
        edit.objectType = "ik_chain";
        edit.propertyName  = m_chainName;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Remove IK Chain"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_chainName;
};

/**
 * Undoable command: set target position for an IK chain.
 */
class SetIKTargetCommand : public IUndoableCommand {
public:
    SetIKTargetCommand(atlas::ecs::DeltaEditStore& store,
                       uint32_t entityID,
                       const std::string& chainName,
                       float x, float y, float z,
                       float oldX, float oldY, float oldZ)
        : m_store(store), m_entityID(entityID),
          m_chainName(chainName),
          m_x(x), m_y(y), m_z(z),
          m_oldX(oldX), m_oldY(oldY), m_oldZ(oldZ) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.objectType  = "ik_chain";
        edit.propertyName = m_chainName;
        edit.position[0] = m_x;
        edit.position[1] = m_y;
        edit.position[2] = m_z;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.objectType  = "ik_chain";
        edit.propertyName = m_chainName;
        edit.position[0] = m_oldX;
        edit.position[1] = m_oldY;
        edit.position[2] = m_oldZ;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set IK Target"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_chainName;
    float m_x, m_y, m_z;
    float m_oldX, m_oldY, m_oldZ;
};

/**
 * Undoable command: set a constraint parameter on an IK chain.
 */
class SetIKConstraintCommand : public IUndoableCommand {
public:
    SetIKConstraintCommand(atlas::ecs::DeltaEditStore& store,
                           uint32_t entityID,
                           const std::string& chainName,
                           const std::string& constraintName,
                           const std::string& oldValue,
                           const std::string& newValue)
        : m_store(store), m_entityID(entityID),
          m_chainName(chainName),
          m_constraintName(constraintName),
          m_oldValue(oldValue), m_newValue(newValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.objectType    = "ik_chain";
        edit.propertyName  = m_chainName + "." + m_constraintName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.objectType    = "ik_chain";
        edit.propertyName  = m_chainName + "." + m_constraintName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set IK Constraint"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_chainName;
    std::string m_constraintName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * Undoable command: set an IK solver property (iterations, tolerance, etc.).
 */
class SetIKSolverPropertyCommand : public IUndoableCommand {
public:
    SetIKSolverPropertyCommand(atlas::ecs::DeltaEditStore& store,
                               uint32_t entityID,
                               const std::string& propertyName,
                               const std::string& oldValue,
                               const std::string& newValue)
        : m_store(store), m_entityID(entityID),
          m_propertyName(propertyName),
          m_oldValue(oldValue), m_newValue(newValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = m_propertyName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = m_propertyName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set IK Solver Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_propertyName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * IKRigTool — concrete ITool for inverse kinematics rigging.
 *
 * Provides helpers that create undoable commands for managing IK chains,
 * setting target positions, configuring constraints, and tuning solver
 * parameters (iterations, tolerance).
 */
class IKRigTool : public ITool {
public:
    IKRigTool(UndoableCommandBus& bus,
              atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "IK Rig"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────
    void AddChain(uint32_t entityID,
                  const std::string& chainName) {
        m_bus.PostCommand(std::make_unique<AddIKChainCommand>(
            m_store, entityID, chainName));
    }

    void RemoveChain(uint32_t entityID,
                     const std::string& chainName) {
        m_bus.PostCommand(std::make_unique<RemoveIKChainCommand>(
            m_store, entityID, chainName));
    }

    void SetTarget(uint32_t entityID,
                   const std::string& chainName,
                   float x, float y, float z,
                   float oldX, float oldY, float oldZ) {
        m_bus.PostCommand(std::make_unique<SetIKTargetCommand>(
            m_store, entityID, chainName, x, y, z, oldX, oldY, oldZ));
    }

    void SetConstraint(uint32_t entityID,
                       const std::string& chainName,
                       const std::string& constraintName,
                       const std::string& oldValue,
                       const std::string& newValue) {
        m_bus.PostCommand(std::make_unique<SetIKConstraintCommand>(
            m_store, entityID, chainName, constraintName, oldValue, newValue));
    }

    void SetSolverIterations(uint32_t entityID,
                             const std::string& oldIter,
                             const std::string& newIter) {
        m_bus.PostCommand(std::make_unique<SetIKSolverPropertyCommand>(
            m_store, entityID, "solver_iterations", oldIter, newIter));
    }

    void SetSolverTolerance(uint32_t entityID,
                            const std::string& oldTol,
                            const std::string& newTol) {
        m_bus.PostCommand(std::make_unique<SetIKSolverPropertyCommand>(
            m_store, entityID, "solver_tolerance", oldTol, newTol));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
