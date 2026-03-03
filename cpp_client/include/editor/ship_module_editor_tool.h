#pragma once
/**
 * @file ship_module_editor_tool.h
 * @brief Concrete ITool for editing ship modules, rigs, and interiors.
 *
 * ShipModuleEditorTool lets the designer attach, detach, and reposition
 * modules on a ship entity.  Every action is posted to the
 * UndoableCommandBus and recorded in the DeltaEditStore so edits
 * persist on top of the PCG-generated ship layout.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include "../../engine/ecs/ECS.h"
#include <string>

namespace atlas::editor {

/**
 * Undoable command: attach a module to a ship.
 */
class AttachModuleCommand : public IUndoableCommand {
public:
    AttachModuleCommand(atlas::ecs::World& world,
                        atlas::ecs::DeltaEditStore& store,
                        atlas::ecs::EntityID shipID,
                        const std::string& moduleType,
                        const std::string& slotName)
        : m_world(world), m_store(store), m_shipID(shipID),
          m_moduleType(moduleType), m_slotName(slotName) {}

    void Execute() override {
        m_moduleEntity = m_world.CreateEntity();
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID      = m_moduleEntity;
        edit.objectType    = m_moduleType;
        edit.propertyName  = "slot";
        edit.propertyValue = m_slotName;
        m_store.Record(edit);
    }

    void Undo() override {
        if (m_world.IsAlive(m_moduleEntity))
            m_world.DestroyEntity(m_moduleEntity);
    }

    const char* Description() const override { return "Attach Module"; }

    atlas::ecs::EntityID ModuleEntity() const { return m_moduleEntity; }

private:
    atlas::ecs::World& m_world;
    atlas::ecs::DeltaEditStore& m_store;
    atlas::ecs::EntityID m_shipID;
    std::string m_moduleType;
    std::string m_slotName;
    atlas::ecs::EntityID m_moduleEntity = 0;
};

/**
 * Undoable command: detach a module from a ship.
 */
class DetachModuleCommand : public IUndoableCommand {
public:
    DetachModuleCommand(atlas::ecs::World& world,
                        atlas::ecs::DeltaEditStore& store,
                        atlas::ecs::EntityID moduleEntity)
        : m_world(world), m_store(store), m_moduleEntity(moduleEntity) {}

    void Execute() override {
        m_wasAlive = m_world.IsAlive(m_moduleEntity);
        if (m_wasAlive) {
            m_world.DestroyEntity(m_moduleEntity);
            atlas::ecs::DeltaEdit edit{};
            edit.type     = atlas::ecs::DeltaEditType::RemoveObject;
            edit.entityID = m_moduleEntity;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        if (m_wasAlive) {
            // Note: re-creation may assign a different EntityID.
            // Full entity-ID restoration requires ECS-level support
            // (e.g. CreateEntityWithID) which is not yet available.
            m_world.CreateEntity();
        }
    }

    const char* Description() const override { return "Detach Module"; }

private:
    atlas::ecs::World& m_world;
    atlas::ecs::DeltaEditStore& m_store;
    atlas::ecs::EntityID m_moduleEntity;
    bool m_wasAlive = false;
};

/**
 * Undoable command: change a property on a module (e.g. slot or stats).
 */
class SetModulePropertyCommand : public IUndoableCommand {
public:
    SetModulePropertyCommand(atlas::ecs::DeltaEditStore& store,
                             atlas::ecs::EntityID moduleEntity,
                             const std::string& propName,
                             const std::string& oldValue,
                             const std::string& newValue)
        : m_store(store), m_moduleEntity(moduleEntity),
          m_propName(propName), m_oldValue(oldValue), m_newValue(newValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_moduleEntity;
        edit.propertyName  = m_propName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_moduleEntity;
        edit.propertyName  = m_propName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Module Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    atlas::ecs::EntityID m_moduleEntity;
    std::string m_propName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * ShipModuleEditorTool — concrete ITool for ship module manipulation.
 *
 * Provides helpers that create undoable commands for attaching,
 * detaching, and modifying ship modules, rigs, and interior props.
 */
class ShipModuleEditorTool : public ITool {
public:
    ShipModuleEditorTool(atlas::ecs::World& world,
                         UndoableCommandBus& bus,
                         atlas::ecs::DeltaEditStore& store)
        : m_world(world), m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Ship Module Editor"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────
    void AttachModule(atlas::ecs::EntityID shipID,
                      const std::string& moduleType,
                      const std::string& slotName) {
        m_bus.PostCommand(std::make_unique<AttachModuleCommand>(
            m_world, m_store, shipID, moduleType, slotName));
    }

    void DetachModule(atlas::ecs::EntityID moduleEntity) {
        m_bus.PostCommand(std::make_unique<DetachModuleCommand>(
            m_world, m_store, moduleEntity));
    }

    void SetModuleProperty(atlas::ecs::EntityID moduleEntity,
                           const std::string& propName,
                           const std::string& oldValue,
                           const std::string& newValue) {
        m_bus.PostCommand(std::make_unique<SetModulePropertyCommand>(
            m_store, moduleEntity, propName, oldValue, newValue));
    }

private:
    atlas::ecs::World& m_world;
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
