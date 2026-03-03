#pragma once
/**
 * @file npc_spawner_tool.h
 * @brief Concrete ITool for spawning and removing NPCs in the world.
 *
 * NPCSpawnerTool lets the designer place NPCs (fleet captains, traders,
 * pirates, security patrols, etc.) with configurable templates.  Every
 * spawn/despawn is posted to the UndoableCommandBus and recorded in the
 * DeltaEditStore so edits persist on top of the PCG seed.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include "../../engine/ecs/ECS.h"
#include <string>

namespace atlas::editor {

/**
 * Undoable command: spawn a new NPC at a given position.
 */
class SpawnNPCCommand : public IUndoableCommand {
public:
    SpawnNPCCommand(atlas::ecs::World& world,
                    atlas::ecs::DeltaEditStore& store,
                    const std::string& npcTemplate,
                    const std::string& faction,
                    float x, float y, float z)
        : m_world(world), m_store(store),
          m_npcTemplate(npcTemplate), m_faction(faction),
          m_x(x), m_y(y), m_z(z) {}

    void Execute() override {
        m_entityID = m_world.CreateEntity();
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID    = m_entityID;
        edit.objectType  = "npc:" + m_npcTemplate;
        edit.position[0] = m_x;
        edit.position[1] = m_y;
        edit.position[2] = m_z;
        edit.propertyName  = "faction";
        edit.propertyValue = m_faction;
        m_store.Record(edit);
    }

    void Undo() override {
        if (m_world.IsAlive(m_entityID))
            m_world.DestroyEntity(m_entityID);
    }

    const char* Description() const override { return "Spawn NPC"; }

    atlas::ecs::EntityID CreatedID() const { return m_entityID; }

private:
    atlas::ecs::World& m_world;
    atlas::ecs::DeltaEditStore& m_store;
    std::string m_npcTemplate;
    std::string m_faction;
    float m_x, m_y, m_z;
    atlas::ecs::EntityID m_entityID = 0;
};

/**
 * Undoable command: despawn (remove) an existing NPC from the world.
 */
class DespawnNPCCommand : public IUndoableCommand {
public:
    DespawnNPCCommand(atlas::ecs::World& world,
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
            m_world.CreateEntity();
        }
    }

    const char* Description() const override { return "Despawn NPC"; }

private:
    atlas::ecs::World& m_world;
    atlas::ecs::DeltaEditStore& m_store;
    atlas::ecs::EntityID m_entityID;
    bool m_wasAlive = false;
};

/**
 * Undoable command: set a property on an existing NPC.
 */
class SetNPCPropertyCommand : public IUndoableCommand {
public:
    SetNPCPropertyCommand(atlas::ecs::DeltaEditStore& store,
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

    const char* Description() const override { return "Set NPC Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_propertyName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * NPCSpawnerTool — concrete ITool for NPC placement and management.
 *
 * Provides helpers that create undoable commands for spawning NPCs from
 * templates, despawning them, and modifying NPC properties.  All changes
 * are recorded in the DeltaEditStore for persistence on top of the PCG seed.
 */
class NPCSpawnerTool : public ITool {
public:
    NPCSpawnerTool(atlas::ecs::World& world,
                   UndoableCommandBus& bus,
                   atlas::ecs::DeltaEditStore& store)
        : m_world(world), m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "NPC Spawner"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void SpawnNPC(const std::string& npcTemplate,
                  const std::string& faction,
                  float x, float y, float z) {
        m_bus.PostCommand(std::make_unique<SpawnNPCCommand>(
            m_world, m_store, npcTemplate, faction, x, y, z));
    }

    void DespawnNPC(atlas::ecs::EntityID id) {
        m_bus.PostCommand(std::make_unique<DespawnNPCCommand>(
            m_world, m_store, id));
    }

    void SetNPCProperty(uint32_t entityID,
                        const std::string& propName,
                        const std::string& oldVal,
                        const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<SetNPCPropertyCommand>(
            m_store, entityID, propName, oldVal, newVal));
    }

private:
    atlas::ecs::World& m_world;
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
