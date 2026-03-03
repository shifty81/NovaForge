#pragma once
/**
 * @file animation_editor_tool.h
 * @brief Concrete ITool for live-editing skeletal animations.
 *
 * AnimationEditorTool lets the designer adjust keyframes, bone
 * transforms, and blending weights on character or rig animations in
 * real-time.  Every modification is posted to the UndoableCommandBus
 * and recorded in the DeltaEditStore so edits persist on top of the
 * PCG seed.
 *
 * Animation properties are stored as SetProperty edits with the
 * property name prefixed by "anim:" to distinguish them from ordinary
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
 * Undoable command: set a keyframe on a bone at a given time.
 */
class SetKeyframeCommand : public IUndoableCommand {
public:
    SetKeyframeCommand(atlas::ecs::DeltaEditStore& store,
                       uint32_t entityID,
                       const std::string& boneName,
                       float time,
                       const std::string& oldValue,
                       const std::string& newValue)
        : m_store(store), m_entityID(entityID),
          m_boneName(boneName), m_time(time),
          m_oldValue(oldValue), m_newValue(newValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "anim:" + m_boneName + "@" + std::to_string(m_time);
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "anim:" + m_boneName + "@" + std::to_string(m_time);
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Keyframe"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_boneName;
    float       m_time;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * Undoable command: remove a keyframe from a bone at a given time.
 */
class RemoveKeyframeCommand : public IUndoableCommand {
public:
    RemoveKeyframeCommand(atlas::ecs::DeltaEditStore& store,
                          uint32_t entityID,
                          const std::string& boneName,
                          float time,
                          const std::string& previousValue)
        : m_store(store), m_entityID(entityID),
          m_boneName(boneName), m_time(time),
          m_previousValue(previousValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "anim:" + m_boneName + "@" + std::to_string(m_time);
        edit.propertyValue = "";   // empty = removed
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "anim:" + m_boneName + "@" + std::to_string(m_time);
        edit.propertyValue = m_previousValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Remove Keyframe"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_boneName;
    float       m_time;
    std::string m_previousValue;
};

/**
 * Undoable command: set a blend weight between two animation layers.
 */
class SetBlendWeightCommand : public IUndoableCommand {
public:
    SetBlendWeightCommand(atlas::ecs::DeltaEditStore& store,
                          uint32_t entityID,
                          const std::string& layerName,
                          const std::string& oldWeight,
                          const std::string& newWeight)
        : m_store(store), m_entityID(entityID),
          m_layerName(layerName),
          m_oldWeight(oldWeight), m_newWeight(newWeight) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "anim:blend:" + m_layerName;
        edit.propertyValue = m_newWeight;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "anim:blend:" + m_layerName;
        edit.propertyValue = m_oldWeight;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Blend Weight"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_layerName;
    std::string m_oldWeight;
    std::string m_newWeight;
};

/**
 * AnimationEditorTool — concrete ITool for live skeletal animation editing.
 *
 * Provides helpers that create undoable commands for setting/removing
 * keyframes on bones and adjusting blend weights between animation
 * layers.  All changes are recorded in the DeltaEditStore for
 * persistence on top of the PCG seed.
 */
class AnimationEditorTool : public ITool {
public:
    AnimationEditorTool(UndoableCommandBus& bus,
                        atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Animation Editor"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void SetKeyframe(uint32_t entityID,
                     const std::string& boneName,
                     float time,
                     const std::string& newValue,
                     const std::string& oldValue = "") {
        m_bus.PostCommand(std::make_unique<SetKeyframeCommand>(
            m_store, entityID, boneName, time, oldValue, newValue));
    }

    void RemoveKeyframe(uint32_t entityID,
                        const std::string& boneName,
                        float time,
                        const std::string& previousValue) {
        m_bus.PostCommand(std::make_unique<RemoveKeyframeCommand>(
            m_store, entityID, boneName, time, previousValue));
    }

    void SetBlendWeight(uint32_t entityID,
                        const std::string& layerName,
                        const std::string& newWeight,
                        const std::string& oldWeight = "") {
        m_bus.PostCommand(std::make_unique<SetBlendWeightCommand>(
            m_store, entityID, layerName, oldWeight, newWeight));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
