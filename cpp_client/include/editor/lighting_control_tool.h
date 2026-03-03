#pragma once
/**
 * @file lighting_control_tool.h
 * @brief Concrete ITool for live lighting and material editing.
 *
 * LightingControlTool lets the designer adjust light source parameters
 * (color, intensity, range, type) and apply lighting presets live.
 * Every modification is posted to the UndoableCommandBus and recorded
 * in the DeltaEditStore so edits persist on top of the PCG seed.
 *
 * Lighting properties are stored as SetProperty edits with the property
 * name prefixed by "light:" to distinguish them from ordinary
 * properties.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/**
 * Undoable command: set a single lighting property (e.g. intensity, color).
 */
class SetLightPropertyCommand : public IUndoableCommand {
public:
    SetLightPropertyCommand(atlas::ecs::DeltaEditStore& store,
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
        edit.propertyName  = "light:" + m_propertyName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "light:" + m_propertyName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Light Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_propertyName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * Undoable command: apply a lighting preset, setting multiple properties at once.
 */
class ApplyLightingPresetCommand : public IUndoableCommand {
public:
    using PropertyList = std::vector<std::pair<std::string, std::string>>;

    ApplyLightingPresetCommand(atlas::ecs::DeltaEditStore& store,
                               uint32_t entityID,
                               const std::string& presetName,
                               const PropertyList& oldProps,
                               const PropertyList& newProps)
        : m_store(store), m_entityID(entityID),
          m_presetName(presetName),
          m_oldProps(oldProps), m_newProps(newProps) {}

    void Execute() override {
        for (const auto& [name, value] : m_newProps) {
            atlas::ecs::DeltaEdit edit{};
            edit.type          = atlas::ecs::DeltaEditType::SetProperty;
            edit.entityID      = m_entityID;
            edit.propertyName  = "light:" + name;
            edit.propertyValue = value;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        for (const auto& [name, value] : m_oldProps) {
            atlas::ecs::DeltaEdit edit{};
            edit.type          = atlas::ecs::DeltaEditType::SetProperty;
            edit.entityID      = m_entityID;
            edit.propertyName  = "light:" + name;
            edit.propertyValue = value;
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Apply Lighting Preset"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_presetName;
    PropertyList m_oldProps;
    PropertyList m_newProps;
};

/**
 * LightingControlTool — concrete ITool for live lighting & material editing.
 *
 * Provides helpers that create undoable commands for changing individual
 * light properties (color, intensity, range, type) and for applying
 * lighting presets (Hangar Ambient, Combat Red Alert, Docking Warm).
 */
class LightingControlTool : public ITool {
public:
    LightingControlTool(UndoableCommandBus& bus,
                        atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Lighting Control"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void SetProperty(uint32_t entityID,
                     const std::string& propName,
                     const std::string& oldVal,
                     const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<SetLightPropertyCommand>(
            m_store, entityID, propName, oldVal, newVal));
    }

    void ApplyPreset(uint32_t entityID,
                     const std::string& presetName,
                     const std::vector<std::pair<std::string, std::string>>& oldProps,
                     const std::vector<std::pair<std::string, std::string>>& newProps) {
        m_bus.PostCommand(std::make_unique<ApplyLightingPresetCommand>(
            m_store, entityID, presetName, oldProps, newProps));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
