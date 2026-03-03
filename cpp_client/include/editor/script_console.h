#pragma once
/**
 * @file script_console.h
 * @brief Concrete ITool for a real-time logging and command console.
 *
 * ScriptConsole provides a live debug / preview console that shows
 * real-time feedback for physics, collisions, DeltaEdits, PCG spawn
 * events, and arbitrary log messages.  Commands can be submitted to
 * manipulate the scene or query state.  All command executions that
 * modify scene state are posted to the UndoableCommandBus.
 *
 * Console entries are stored as SetProperty edits with the property
 * name prefixed by "console:" to distinguish them from ordinary
 * properties.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <deque>
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/** A single console log entry with severity level. */
struct ConsoleEntry {
    enum class Level { Info, Warning, Error };
    Level       level   = Level::Info;
    std::string message;
};

/**
 * Undoable command: execute a console command that sets a scene property.
 */
class ConsoleSetPropertyCommand : public IUndoableCommand {
public:
    ConsoleSetPropertyCommand(atlas::ecs::DeltaEditStore& store,
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
        edit.propertyName  = "console:" + m_propertyName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "console:" + m_propertyName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Console Set Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_propertyName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * ScriptConsole — concrete ITool providing a live logging & command console.
 *
 * Features:
 *   - Log messages at Info / Warning / Error levels
 *   - Fixed-capacity ring buffer (oldest entries evicted)
 *   - Execute commands that set scene properties (undoable)
 *   - Filter entries by severity level
 *   - Clear all entries
 */
class ScriptConsole : public ITool {
public:
    ScriptConsole(UndoableCommandBus& bus,
                  atlas::ecs::DeltaEditStore& store,
                  size_t maxEntries = 1000)
        : m_bus(bus), m_store(store), m_maxEntries(maxEntries) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Script Console"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Logging ─────────────────────────────────────────────────────

    void Log(const std::string& message) {
        AddEntry(ConsoleEntry::Level::Info, message);
    }

    void Warn(const std::string& message) {
        AddEntry(ConsoleEntry::Level::Warning, message);
    }

    void Error(const std::string& message) {
        AddEntry(ConsoleEntry::Level::Error, message);
    }

    // ── Query ───────────────────────────────────────────────────────

    size_t EntryCount() const { return m_entries.size(); }

    const std::deque<ConsoleEntry>& Entries() const { return m_entries; }

    std::vector<const ConsoleEntry*> Filter(ConsoleEntry::Level level) const {
        std::vector<const ConsoleEntry*> result;
        for (const auto& e : m_entries) {
            if (e.level == level) {
                result.push_back(&e);
            }
        }
        return result;
    }

    void Clear() { m_entries.clear(); }

    // ── Command execution (undoable) ────────────────────────────────

    void ExecuteSetProperty(uint32_t entityID,
                            const std::string& propName,
                            const std::string& oldVal,
                            const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<ConsoleSetPropertyCommand>(
            m_store, entityID, propName, oldVal, newVal));
    }

private:
    void AddEntry(ConsoleEntry::Level level, const std::string& message) {
        if (m_entries.size() >= m_maxEntries) {
            m_entries.pop_front();
        }
        m_entries.push_back({level, message});
    }

    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    size_t m_maxEntries;
    bool m_active = false;
    std::deque<ConsoleEntry> m_entries;
};

} // namespace atlas::editor
