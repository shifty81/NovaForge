#pragma once
/**
 * @file batch_operations_tool.h
 * @brief Concrete ITool for applying mass operations to multiple entities.
 *
 * BatchOperationsTool lets the designer apply the same property change
 * or transformation to a set of entities at once.  Every batch operation
 * is posted to the UndoableCommandBus as a single undoable action, and
 * each individual change is recorded in the DeltaEditStore so edits
 * persist on top of the PCG seed.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/**
 * Undoable command: set the same property on multiple entities.
 */
class BatchSetPropertyCommand : public IUndoableCommand {
public:
    using EntityOldValue = std::pair<uint32_t, std::string>;

    BatchSetPropertyCommand(atlas::ecs::DeltaEditStore& store,
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

    const char* Description() const override { return "Batch Set Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    std::vector<EntityOldValue> m_targets;
    std::string m_propertyName;
    std::string m_newValue;
};

/**
 * Undoable command: apply a position transform to multiple entities.
 */
class BatchTransformCommand : public IUndoableCommand {
public:
    struct EntityTransform {
        uint32_t entityID;
        float oldX, oldY, oldZ;
        float newX, newY, newZ;
    };

    BatchTransformCommand(atlas::ecs::DeltaEditStore& store,
                          const std::vector<EntityTransform>& transforms)
        : m_store(store), m_transforms(transforms) {}

    void Execute() override {
        for (const auto& t : m_transforms) {
            atlas::ecs::DeltaEdit edit{};
            edit.type        = atlas::ecs::DeltaEditType::MoveObject;
            edit.entityID    = t.entityID;
            edit.position[0] = t.newX;
            edit.position[1] = t.newY;
            edit.position[2] = t.newZ;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        for (const auto& t : m_transforms) {
            atlas::ecs::DeltaEdit edit{};
            edit.type        = atlas::ecs::DeltaEditType::MoveObject;
            edit.entityID    = t.entityID;
            edit.position[0] = t.oldX;
            edit.position[1] = t.oldY;
            edit.position[2] = t.oldZ;
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Batch Transform"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    std::vector<EntityTransform> m_transforms;
};

/**
 * BatchOperationsTool — concrete ITool for mass entity operations.
 *
 * Provides helpers that create undoable commands for setting the same
 * property on many entities at once, or applying position transforms
 * to a batch of entities.  All changes are recorded in the
 * DeltaEditStore for persistence on top of the PCG seed.
 */
class BatchOperationsTool : public ITool {
public:
    BatchOperationsTool(UndoableCommandBus& bus,
                        atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Batch Operations"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void SetPropertyOnMany(
            const std::vector<std::pair<uint32_t, std::string>>& targets,
            const std::string& propName,
            const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<BatchSetPropertyCommand>(
            m_store, targets, propName, newVal));
    }

    void TransformMany(
            const std::vector<BatchTransformCommand::EntityTransform>& transforms) {
        m_bus.PostCommand(std::make_unique<BatchTransformCommand>(
            m_store, transforms));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
