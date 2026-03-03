#pragma once
/**
 * @file edit_propagation_tool.h
 * @brief Concrete ITool for propagating edits across similar entities.
 *
 * EditPropagationTool lets the designer apply a property change to one
 * entity and optionally propagate it to all entities that share the same
 * object type.  For example, adjusting a ship engine module position
 * and propagating the change to all ships of that type.  Every
 * propagation is posted to the UndoableCommandBus as a single batch
 * and recorded in the DeltaEditStore for persistence.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/**
 * Undoable command: propagate a property value to multiple entities.
 *
 * Records old values so the batch can be undone atomically.
 */
class PropagatePropertyCommand : public IUndoableCommand {
public:
    using EntityOldValue = std::pair<uint32_t, std::string>;

    PropagatePropertyCommand(atlas::ecs::DeltaEditStore& store,
                             const std::vector<EntityOldValue>& targets,
                             const std::string& propertyName,
                             const std::string& newValue)
        : m_store(store), m_targets(targets),
          m_propertyName(propertyName), m_newValue(newValue) {}

    void Execute() override {
        for (const auto& [entityID, oldVal] : m_targets) {
            atlas::ecs::DeltaEdit edit{};
            edit.type          = atlas::ecs::DeltaEditType::SetProperty;
            edit.entityID      = entityID;
            edit.propertyName  = m_propertyName;
            edit.propertyValue = m_newValue;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        for (const auto& [entityID, oldVal] : m_targets) {
            atlas::ecs::DeltaEdit edit{};
            edit.type          = atlas::ecs::DeltaEditType::SetProperty;
            edit.entityID      = entityID;
            edit.propertyName  = m_propertyName;
            edit.propertyValue = oldVal;
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Propagate Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    std::vector<EntityOldValue> m_targets;
    std::string m_propertyName;
    std::string m_newValue;
};

/**
 * Undoable command: propagate a position to multiple entities.
 *
 * Records old positions so the batch can be undone atomically.
 */
class PropagatePositionCommand : public IUndoableCommand {
public:
    struct EntityPosition {
        uint32_t entityID;
        float oldX, oldY, oldZ;
    };

    PropagatePositionCommand(atlas::ecs::DeltaEditStore& store,
                             const std::vector<EntityPosition>& targets,
                             float newX, float newY, float newZ)
        : m_store(store), m_targets(targets),
          m_newX(newX), m_newY(newY), m_newZ(newZ) {}

    void Execute() override {
        for (const auto& t : m_targets) {
            atlas::ecs::DeltaEdit edit{};
            edit.type        = atlas::ecs::DeltaEditType::MoveObject;
            edit.entityID    = t.entityID;
            edit.position[0] = m_newX;
            edit.position[1] = m_newY;
            edit.position[2] = m_newZ;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        for (const auto& t : m_targets) {
            atlas::ecs::DeltaEdit edit{};
            edit.type        = atlas::ecs::DeltaEditType::MoveObject;
            edit.entityID    = t.entityID;
            edit.position[0] = t.oldX;
            edit.position[1] = t.oldY;
            edit.position[2] = t.oldZ;
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Propagate Position"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    std::vector<EntityPosition> m_targets;
    float m_newX, m_newY, m_newZ;
};

/**
 * EditPropagationTool — concrete ITool for propagating edits to similar entities.
 *
 * Provides helpers that create undoable commands for propagating a
 * property value or position to a set of entities that share the same
 * object type.  All changes are recorded in the DeltaEditStore for
 * persistence on top of the PCG seed.
 */
class EditPropagationTool : public ITool {
public:
    EditPropagationTool(UndoableCommandBus& bus,
                        atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Edit Propagation"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void PropagateProperty(
            const std::vector<std::pair<uint32_t, std::string>>& targets,
            const std::string& propName,
            const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<PropagatePropertyCommand>(
            m_store, targets, propName, newVal));
    }

    void PropagatePosition(
            const std::vector<PropagatePositionCommand::EntityPosition>& targets,
            float newX, float newY, float newZ) {
        m_bus.PostCommand(std::make_unique<PropagatePositionCommand>(
            m_store, targets, newX, newY, newZ));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
