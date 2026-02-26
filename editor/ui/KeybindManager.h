#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace atlas::editor {

/**
 * Modifier key flags (can be OR-combined).
 */
enum class KeyMod : uint8_t {
    None    = 0,
    Ctrl    = 1 << 0,
    Shift   = 1 << 1,
    Alt     = 1 << 2,
};

inline KeyMod operator|(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline KeyMod operator&(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
inline bool hasFlag(KeyMod flags, KeyMod flag) {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

/**
 * A single key binding: an action name mapped to a key combination.
 */
struct Keybind {
    std::string action;          ///< e.g. "Undo", "Save", "ToggleGrid"
    std::string category;        ///< e.g. "General", "Viewport", "Editor"
    int         key     = 0;     ///< Virtual key code (ASCII for letters)
    KeyMod      mods    = KeyMod::None;
    bool        enabled = true;
};

/**
 * @brief KeybindManager — editor keyboard shortcut management.
 *
 * Stores a set of action→key bindings and dispatches actions when keys
 * are pressed.  Supports:
 *   - Registering and removing bindings
 *   - Conflict detection (same key+mod assigned to multiple actions)
 *   - Querying bindings by action or by key
 *   - Triggering registered callbacks
 *   - Human-readable descriptions (e.g. "Ctrl+Z")
 *   - Enable/disable individual bindings
 */
class KeybindManager {
public:
    using ActionCallback = std::function<void()>;

    KeybindManager();
    ~KeybindManager() = default;

    // ── Binding management ───────────────────────────────────────

    /** Register a new key binding.  Returns true if added (false if duplicate action). */
    bool AddBinding(const Keybind& binding);

    /** Remove a binding by action name.  Returns true if removed. */
    bool RemoveBinding(const std::string& action);

    /** Update the key assignment for an existing action. */
    bool Rebind(const std::string& action, int key, KeyMod mods);

    /** Enable or disable a specific binding. */
    bool SetEnabled(const std::string& action, bool enabled);

    /** Get the number of registered bindings. */
    size_t BindingCount() const { return m_bindings.size(); }

    /** Find a binding by action name.  Returns nullptr if not found. */
    const Keybind* FindBinding(const std::string& action) const;

    /** Find all bindings assigned to a specific key+mods combination. */
    std::vector<const Keybind*> FindByKey(int key, KeyMod mods) const;

    /** Get all bindings in a category. */
    std::vector<const Keybind*> BindingsInCategory(const std::string& category) const;

    /** Read-only access to all bindings. */
    const std::vector<Keybind>& AllBindings() const { return m_bindings; }

    // ── Conflict detection ───────────────────────────────────────

    /** Check if a key+mods combo is already bound to another action. */
    bool HasConflict(int key, KeyMod mods, const std::string& excludeAction = "") const;

    // ── Action dispatch ──────────────────────────────────────────

    /** Register a callback for an action.  Overwrites any previous callback. */
    void RegisterCallback(const std::string& action, ActionCallback callback);

    /**
     * Handle a key press event.  Looks up the binding and fires
     * the registered callback if one exists and the binding is enabled.
     * Returns true if an action was triggered.
     */
    bool HandleKeyPress(int key, KeyMod mods);

    // ── Display helpers ──────────────────────────────────────────

    /** Human-readable string for a key binding (e.g. "Ctrl+Z"). */
    static std::string DescribeBinding(int key, KeyMod mods);

    /** Human-readable string for a specific action. */
    std::string DescribeAction(const std::string& action) const;

    // ── Default bindings ─────────────────────────────────────────

    /** Install a standard set of editor key bindings. */
    void InstallDefaults();

    /** Clear all bindings and callbacks. */
    void Clear();

    // ── Persistence ─────────────────────────────────────────────

    /** Save all bindings to a JSON file.  Returns true on success. */
    bool SaveToFile(const std::string& path) const;

    /** Load bindings from a JSON file (replaces current bindings,
     *  preserves registered callbacks).  Returns true on success. */
    bool LoadFromFile(const std::string& path);

    /** Serialise all bindings to a JSON string. */
    std::string SerializeToJSON() const;

    /** Deserialise bindings from a JSON string, replacing current bindings. */
    bool DeserializeFromJSON(const std::string& json);

private:
    Keybind* findMutable(const std::string& action);

    struct CallbackEntry {
        std::string action;
        ActionCallback callback;
    };

    std::vector<Keybind> m_bindings;
    std::vector<CallbackEntry> m_callbacks;
};

} // namespace atlas::editor
