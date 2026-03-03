#pragma once
/**
 * @file prefab_library.h
 * @brief Reusable prefab template library for the editor.
 *
 * PrefabLibrary stores named prefab entries that designers create from
 * existing scene objects.  Each prefab captures an object type, a human-
 * readable name, and template data (encoded as key-value properties).
 * Prefabs can be filtered by type, looked up by name, and cleared.
 *
 * This is editor-side state — it does not directly modify the
 * DeltaEditStore, but prefab placement is expected to flow through
 * the UndoableCommandBus when placed into the scene.
 */

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::editor {

/**
 * A single prefab template entry.
 */
struct PrefabEntry {
    std::string name;         ///< Unique human-readable name
    std::string type;         ///< Category: "ship", "prop", "station", etc.
    std::string description;  ///< Optional short description
    std::unordered_map<std::string, std::string> properties; ///< Template data
};

/**
 * PrefabLibrary — manages reusable prefab templates.
 *
 * Usage:
 *   PrefabLibrary lib;
 *   PrefabEntry entry;
 *   entry.name = "PatrolFrigate";
 *   entry.type = "ship";
 *   entry.properties["hull"] = "frigate_mk2";
 *   lib.Add(entry);
 *   auto ships = lib.FilterByType("ship");
 */
class PrefabLibrary {
public:
    /**
     * Add a prefab entry.  Returns false if a prefab with the same
     * name already exists.
     */
    bool Add(const PrefabEntry& entry) {
        if (entry.name.empty()) return false;
        for (const auto& p : m_entries)
            if (p.name == entry.name) return false;
        m_entries.push_back(entry);
        return true;
    }

    /**
     * Remove a prefab by name.  Returns false if not found.
     */
    bool Remove(const std::string& name) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const PrefabEntry& p) { return p.name == name; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it);
        return true;
    }

    /**
     * Look up a prefab by name.  Returns nullptr if not found.
     */
    const PrefabEntry* Get(const std::string& name) const {
        for (const auto& p : m_entries)
            if (p.name == name) return &p;
        return nullptr;
    }

    /** Number of stored prefabs. */
    size_t Count() const { return m_entries.size(); }

    /**
     * Rename a prefab.  Returns false if old name not found or new
     * name already taken.
     */
    bool Rename(const std::string& oldName, const std::string& newName) {
        if (newName.empty()) return false;
        // Check new name is not already used
        for (const auto& p : m_entries)
            if (p.name == newName) return false;
        for (auto& p : m_entries) {
            if (p.name == oldName) {
                p.name = newName;
                return true;
            }
        }
        return false;
    }

    /** Filter prefabs by type (e.g. "ship", "prop"). */
    std::vector<const PrefabEntry*> FilterByType(const std::string& type) const {
        std::vector<const PrefabEntry*> result;
        for (const auto& p : m_entries)
            if (p.type == type)
                result.push_back(&p);
        return result;
    }

    /** Collect unique types present in the library. */
    std::vector<std::string> UniqueTypes() const {
        std::unordered_map<std::string, bool> seen;
        for (const auto& p : m_entries)
            seen[p.type] = true;
        std::vector<std::string> types;
        types.reserve(seen.size());
        for (const auto& [t, _] : seen)
            types.push_back(t);
        return types;
    }

    /** Read-only access to all entries. */
    const std::vector<PrefabEntry>& Entries() const { return m_entries; }

    /** Clear all prefabs. */
    void Clear() { m_entries.clear(); }

private:
    std::vector<PrefabEntry> m_entries;
};

} // namespace atlas::editor
