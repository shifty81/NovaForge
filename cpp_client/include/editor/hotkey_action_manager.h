#pragma once
/**
 * @file hotkey_action_manager.h
 * @brief Concrete ITool for managing editor hotkey-to-action bindings.
 *
 * HotkeyActionManager lets the designer bind keyboard shortcuts to
 * common editor operations — for example "Ctrl+Z" → Undo, "G" → Grab
 * (translate), "R" → Rotate, "S" → Scale.  Each binding change is
 * posted to the UndoableCommandBus and recorded in the DeltaEditStore
 * so custom keybinds persist on top of the PCG seed.
 *
 * Hotkey bindings are stored as SetProperty edits with the property
 * name prefixed by "hotkey:" to distinguish them from ordinary
 * properties.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace atlas::editor {

/** A single hotkey → action binding. */
struct HotkeyBinding {
    std::string key;      // e.g. "Ctrl+Z", "G", "Shift+D"
    std::string action;   // e.g. "undo", "grab", "duplicate"
};

/**
 * Undoable command: bind a hotkey to an action.
 */
class BindHotkeyCommand : public IUndoableCommand {
public:
    BindHotkeyCommand(atlas::ecs::DeltaEditStore& store,
                      uint32_t entityID,
                      const std::string& key,
                      const std::string& oldAction,
                      const std::string& newAction)
        : m_store(store), m_entityID(entityID),
          m_key(key), m_oldAction(oldAction),
          m_newAction(newAction) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "hotkey:" + m_key;
        edit.propertyValue = m_newAction;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "hotkey:" + m_key;
        edit.propertyValue = m_oldAction;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Bind Hotkey"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_key;
    std::string m_oldAction;
    std::string m_newAction;
};

/**
 * Undoable command: unbind a hotkey (remove its action).
 */
class UnbindHotkeyCommand : public IUndoableCommand {
public:
    UnbindHotkeyCommand(atlas::ecs::DeltaEditStore& store,
                        uint32_t entityID,
                        const std::string& key,
                        const std::string& previousAction)
        : m_store(store), m_entityID(entityID),
          m_key(key), m_previousAction(previousAction) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "hotkey:" + m_key;
        edit.propertyValue = "";   // empty = unbound
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "hotkey:" + m_key;
        edit.propertyValue = m_previousAction;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Unbind Hotkey"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_key;
    std::string m_previousAction;
};

/**
 * HotkeyActionManager — concrete ITool for managing editor hotkey bindings.
 *
 * Maintains a local binding map and provides helpers that create
 * undoable commands for binding/unbinding keyboard shortcuts to editor
 * actions.  All changes are recorded in the DeltaEditStore for
 * persistence on top of the PCG seed.
 */
class HotkeyActionManager : public ITool {
public:
    HotkeyActionManager(UndoableCommandBus& bus,
                        atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Hotkey Action Manager"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Local binding map ───────────────────────────────────────────

    /** Return the number of active bindings. */
    size_t BindingCount() const { return m_bindings.size(); }

    /** Look up the action for a key. Returns empty string if unbound. */
    std::string ActionForKey(const std::string& key) const {
        auto it = m_bindings.find(key);
        return (it != m_bindings.end()) ? it->second : "";
    }

    /** Return all current bindings as a vector. */
    std::vector<HotkeyBinding> AllBindings() const {
        std::vector<HotkeyBinding> result;
        result.reserve(m_bindings.size());
        for (const auto& [k, a] : m_bindings) {
            result.push_back({k, a});
        }
        return result;
    }

    // ── Helpers that create and post commands ────────────────────────

    /** Bind a hotkey to an action (posts undoable command + updates local map). */
    void Bind(uint32_t entityID,
              const std::string& key,
              const std::string& action) {
        std::string oldAction = ActionForKey(key);
        m_bindings[key] = action;
        m_bus.PostCommand(std::make_unique<BindHotkeyCommand>(
            m_store, entityID, key, oldAction, action));
    }

    /** Unbind a hotkey (posts undoable command + removes from local map). */
    void Unbind(uint32_t entityID,
                const std::string& key) {
        std::string prev = ActionForKey(key);
        m_bindings.erase(key);
        m_bus.PostCommand(std::make_unique<UnbindHotkeyCommand>(
            m_store, entityID, key, prev));
    }

    /** Remove all bindings from the local map (does not post commands). */
    void ClearBindings() { m_bindings.clear(); }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
    std::unordered_map<std::string, std::string> m_bindings;
};

} // namespace atlas::editor
