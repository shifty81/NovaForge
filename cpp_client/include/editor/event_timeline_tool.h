#pragma once
/**
 * @file event_timeline_tool.h
 * @brief Concrete ITool for creating and editing event timelines.
 *
 * EventTimelineTool lets the designer author sequences of timed events
 * (spawn → animation → physics → trigger) for cutscenes, tutorials,
 * scripted encounters, or test scenarios.  Every addition, removal, or
 * property change is posted to the UndoableCommandBus and recorded in
 * the DeltaEditStore so edits persist on top of the PCG seed.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/**
 * Undoable command: add a new event to the timeline at a given time offset.
 */
class AddTimelineEventCommand : public IUndoableCommand {
public:
    AddTimelineEventCommand(atlas::ecs::DeltaEditStore& store,
                            uint32_t timelineID,
                            const std::string& eventType,
                            float timeOffset)
        : m_store(store), m_timelineID(timelineID),
          m_eventType(eventType), m_timeOffset(timeOffset) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID      = m_timelineID;
        edit.objectType    = "timeline_event:" + m_eventType;
        edit.position[0]   = m_timeOffset;
        edit.position[1]   = 0.0f;
        edit.position[2]   = 0.0f;
        edit.propertyName  = "time_offset";
        edit.propertyValue = std::to_string(m_timeOffset);
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type     = atlas::ecs::DeltaEditType::RemoveObject;
        edit.entityID = m_timelineID;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Add Timeline Event"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_timelineID;
    std::string m_eventType;
    float       m_timeOffset;
};

/**
 * Undoable command: remove an event from the timeline.
 */
class RemoveTimelineEventCommand : public IUndoableCommand {
public:
    RemoveTimelineEventCommand(atlas::ecs::DeltaEditStore& store,
                               uint32_t timelineID,
                               const std::string& eventType,
                               float timeOffset)
        : m_store(store), m_timelineID(timelineID),
          m_eventType(eventType), m_timeOffset(timeOffset) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type     = atlas::ecs::DeltaEditType::RemoveObject;
        edit.entityID = m_timelineID;
        edit.propertyName  = "event_type";
        edit.propertyValue = m_eventType;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::AddObject;
        edit.entityID      = m_timelineID;
        edit.objectType    = "timeline_event:" + m_eventType;
        edit.position[0]   = m_timeOffset;
        edit.position[1]   = 0.0f;
        edit.position[2]   = 0.0f;
        edit.propertyName  = "time_offset";
        edit.propertyValue = std::to_string(m_timeOffset);
        m_store.Record(edit);
    }

    const char* Description() const override { return "Remove Timeline Event"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_timelineID;
    std::string m_eventType;
    float       m_timeOffset;
};

/**
 * Undoable command: set a property on a timeline event.
 */
class SetTimelineEventPropertyCommand : public IUndoableCommand {
public:
    SetTimelineEventPropertyCommand(atlas::ecs::DeltaEditStore& store,
                                    uint32_t timelineID,
                                    const std::string& propertyName,
                                    const std::string& oldValue,
                                    const std::string& newValue)
        : m_store(store), m_timelineID(timelineID),
          m_propertyName(propertyName),
          m_oldValue(oldValue), m_newValue(newValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_timelineID;
        edit.propertyName  = m_propertyName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_timelineID;
        edit.propertyName  = m_propertyName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Timeline Event Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_timelineID;
    std::string m_propertyName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * EventTimelineTool — concrete ITool for authoring event sequences.
 *
 * Provides helpers that create undoable commands for adding timeline
 * events (spawn, animation, physics, trigger), removing them, and
 * modifying their properties.  All changes are recorded in the
 * DeltaEditStore for persistence on top of the PCG seed.
 */
class EventTimelineTool : public ITool {
public:
    EventTimelineTool(UndoableCommandBus& bus,
                      atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Event Timeline"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void AddEvent(uint32_t timelineID,
                  const std::string& eventType,
                  float timeOffset) {
        m_bus.PostCommand(std::make_unique<AddTimelineEventCommand>(
            m_store, timelineID, eventType, timeOffset));
    }

    void RemoveEvent(uint32_t timelineID,
                     const std::string& eventType,
                     float timeOffset) {
        m_bus.PostCommand(std::make_unique<RemoveTimelineEventCommand>(
            m_store, timelineID, eventType, timeOffset));
    }

    void SetEventProperty(uint32_t timelineID,
                          const std::string& propName,
                          const std::string& oldVal,
                          const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<SetTimelineEventPropertyCommand>(
            m_store, timelineID, propName, oldVal, newVal));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
