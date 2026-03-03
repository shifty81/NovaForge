#pragma once
/**
 * @file map_editor_tool.h
 * @brief Concrete ITool for placing, moving, and removing world objects.
 *
 * MapEditorTool is the primary tool for editing star-system layouts:
 * planets, stations, asteroid fields, resource nodes, etc.  Every
 * modification is posted to the UndoableCommandBus and recorded in a
 * DeltaEditStore so edits persist on top of the PCG seed.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include "../../engine/ecs/ECS.h"
#include <string>

namespace atlas::editor {

/**
 * Undoable command: add a new object to the world.
 */
class AddObjectCommand : public IUndoableCommand {
public:
    AddObjectCommand(atlas::ecs::World& world,
                     atlas::ecs::DeltaEditStore& store,
                     const std::string& objectType,
                     float x, float y, float z)
        : m_world(world), m_store(store),
          m_objectType(objectType), m_x(x), m_y(y), m_z(z) {}

    void Execute() override {
        m_entityID = m_world.CreateEntity();
        atlas::ecs::DeltaEdit edit{};
        edit.type       = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID   = m_entityID;
        edit.objectType = m_objectType;
        edit.position[0] = m_x;
        edit.position[1] = m_y;
        edit.position[2] = m_z;
        m_store.Record(edit);
    }

    void Undo() override {
        if (m_world.IsAlive(m_entityID))
            m_world.DestroyEntity(m_entityID);
    }

    const char* Description() const override { return "Add Object"; }

    atlas::ecs::EntityID CreatedID() const { return m_entityID; }

private:
    atlas::ecs::World& m_world;
    atlas::ecs::DeltaEditStore& m_store;
    std::string m_objectType;
    float m_x, m_y, m_z;
    atlas::ecs::EntityID m_entityID = 0;
};

/**
 * Undoable command: move an existing object to a new position.
 */
class MoveObjectCommand : public IUndoableCommand {
public:
    MoveObjectCommand(atlas::ecs::DeltaEditStore& store,
                      atlas::ecs::EntityID entityID,
                      float oldX, float oldY, float oldZ,
                      float newX, float newY, float newZ)
        : m_store(store), m_entityID(entityID),
          m_oldX(oldX), m_oldY(oldY), m_oldZ(oldZ),
          m_newX(newX), m_newY(newY), m_newZ(newZ) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.position[0] = m_newX;
        edit.position[1] = m_newY;
        edit.position[2] = m_newZ;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.position[0] = m_oldX;
        edit.position[1] = m_oldY;
        edit.position[2] = m_oldZ;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Move Object"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    atlas::ecs::EntityID m_entityID;
    float m_oldX, m_oldY, m_oldZ;
    float m_newX, m_newY, m_newZ;
};

/**
 * Undoable command: remove an object from the world.
 */
class RemoveObjectCommand : public IUndoableCommand {
public:
    RemoveObjectCommand(atlas::ecs::World& world,
                        atlas::ecs::DeltaEditStore& store,
                        atlas::ecs::EntityID entityID)
        : m_world(world), m_store(store), m_entityID(entityID) {}

    void Execute() override {
        m_wasAlive = m_world.IsAlive(m_entityID);
        if (m_wasAlive) {
            m_world.DestroyEntity(m_entityID);
            atlas::ecs::DeltaEdit edit{};
            edit.type     = atlas::ecs::DeltaEditType::RemoveObject;
            edit.entityID = m_entityID;
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

    const char* Description() const override { return "Remove Object"; }

private:
    atlas::ecs::World& m_world;
    atlas::ecs::DeltaEditStore& m_store;
    atlas::ecs::EntityID m_entityID;
    bool m_wasAlive = false;
};

/**
 * MapEditorTool — concrete ITool for world-level object manipulation.
 *
 * Provides helpers that create undoable commands and post them to a
 * command bus, while simultaneously recording DeltaEdits for persistence.
 */
class MapEditorTool : public ITool {
public:
    MapEditorTool(atlas::ecs::World& world,
                  UndoableCommandBus& bus,
                  atlas::ecs::DeltaEditStore& store)
        : m_world(world), m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Map Editor"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────
    void AddObject(const std::string& objectType, float x, float y, float z) {
        m_bus.PostCommand(std::make_unique<AddObjectCommand>(
            m_world, m_store, objectType, x, y, z));
    }

    void MoveObject(atlas::ecs::EntityID id,
                    float oldX, float oldY, float oldZ,
                    float newX, float newY, float newZ) {
        m_bus.PostCommand(std::make_unique<MoveObjectCommand>(
            m_store, id, oldX, oldY, oldZ, newX, newY, newZ));
    }

    void RemoveObject(atlas::ecs::EntityID id) {
        m_bus.PostCommand(std::make_unique<RemoveObjectCommand>(
            m_world, m_store, id));
    }

private:
    atlas::ecs::World& m_world;
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
