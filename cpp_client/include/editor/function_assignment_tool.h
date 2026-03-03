#pragma once
/**
 * @file function_assignment_tool.h
 * @brief Concrete ITool for assigning triggers and functions to entities.
 *
 * FunctionAssignmentTool lets the designer attach behaviour triggers to
 * scene objects — for example "door_open", "turret_fire",
 * "lights_flicker", or "ai_spawn".  Each assignment (and removal) is
 * posted to the UndoableCommandBus and recorded in the DeltaEditStore
 * so edits persist on top of the PCG seed.
 *
 * Functions are stored as SetProperty edits with the property name
 * prefixed by "fn:" to distinguish them from ordinary properties.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/**
 * Undoable command: assign a function/trigger to an entity.
 */
class AssignFunctionCommand : public IUndoableCommand {
public:
    AssignFunctionCommand(atlas::ecs::DeltaEditStore& store,
                          uint32_t entityID,
                          const std::string& functionName,
                          const std::string& functionConfig,
                          const std::string& previousConfig)
        : m_store(store), m_entityID(entityID),
          m_functionName(functionName),
          m_functionConfig(functionConfig),
          m_previousConfig(previousConfig) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "fn:" + m_functionName;
        edit.propertyValue = m_functionConfig;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "fn:" + m_functionName;
        edit.propertyValue = m_previousConfig;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Assign Function"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_functionName;
    std::string m_functionConfig;
    std::string m_previousConfig;
};

/**
 * Undoable command: remove a function/trigger from an entity.
 */
class RemoveFunctionCommand : public IUndoableCommand {
public:
    RemoveFunctionCommand(atlas::ecs::DeltaEditStore& store,
                          uint32_t entityID,
                          const std::string& functionName,
                          const std::string& previousConfig)
        : m_store(store), m_entityID(entityID),
          m_functionName(functionName),
          m_previousConfig(previousConfig) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "fn:" + m_functionName;
        edit.propertyValue = "";   // empty = removed
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "fn:" + m_functionName;
        edit.propertyValue = m_previousConfig;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Remove Function"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_functionName;
    std::string m_previousConfig;
};

/**
 * FunctionAssignmentTool — concrete ITool for assigning triggers/functions.
 *
 * Provides helpers that create undoable commands for attaching behaviour
 * triggers (door_open, turret_fire, lights_flicker, ai_spawn) to scene
 * entities.  All changes are recorded in the DeltaEditStore for
 * persistence on top of the PCG seed.
 */
class FunctionAssignmentTool : public ITool {
public:
    FunctionAssignmentTool(UndoableCommandBus& bus,
                           atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Function Assignment"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void AssignFunction(uint32_t entityID,
                        const std::string& functionName,
                        const std::string& config,
                        const std::string& previousConfig = "") {
        m_bus.PostCommand(std::make_unique<AssignFunctionCommand>(
            m_store, entityID, functionName, config, previousConfig));
    }

    void RemoveFunction(uint32_t entityID,
                        const std::string& functionName,
                        const std::string& previousConfig) {
        m_bus.PostCommand(std::make_unique<RemoveFunctionCommand>(
            m_store, entityID, functionName, previousConfig));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
