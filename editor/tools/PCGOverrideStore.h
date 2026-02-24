#pragma once
#include "../ui/EditorPanel.h"
#include "ViewportPanel.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace atlas::editor {

/**
 * A single manual override applied on top of procedural generation.
 *
 * The editor produces these when the user drags an object in the viewport
 * or tweaks a PCG parameter.  They are saved to a JSON file that the
 * client reads at startup — no project rebuild is needed.
 */
struct PCGOverride {
    uint32_t    objectId   = 0;
    std::string objectName;            ///< e.g. "Ship_Vanguard Turret 3"
    std::string objectType;            ///< "Ship", "Module", "Hardpoint", …
    std::string field;                 ///< "position", "rotation", "scale"
    float       values[3] = {};        ///< x, y, z
    uint64_t    seed       = 0;        ///< PCG seed the override applies to
    uint32_t    version    = 1;        ///< PCG version stamp
};

/**
 * Persistent store for PCG manual overrides.
 *
 * Workflow:
 *   1. Designer edits PCG content in the viewport (translate / rotate / scale).
 *   2. Designer clicks **Save** → overrides are written to a JSON file.
 *   3. Designer opens the game client → client loads the override file at
 *      startup and applies the tweaks on top of the PCG output.
 *
 * The file format is a simple JSON array so it is human-readable and
 * mergeable in version control.
 */
class PCGOverrideStore {
public:
    // ── Override management ───────────────────────────────────────

    /** Import all pending viewport changes as overrides. */
    void ImportFromViewport(const std::vector<ViewportChange>& changes,
                            const std::vector<ViewportObject>& objects,
                            uint64_t seed, uint32_t version);

    /** Add a single override directly. */
    void Add(const PCGOverride& ov);

    /** Remove all overrides for a given object ID. */
    void RemoveByObject(uint32_t objectId);

    /** Clear the entire store. */
    void Clear();

    /** Get all current overrides. */
    const std::vector<PCGOverride>& Overrides() const { return m_overrides; }

    /** True when there are unsaved overrides. */
    bool IsDirty() const { return m_dirty; }

    // ── Serialisation (JSON text) ────────────────────────────────

    /** Serialise all overrides to a JSON string. */
    std::string SerializeToJSON() const;

    /** Deserialise overrides from a JSON string, replacing current state. */
    bool DeserializeFromJSON(const std::string& json);

    // ── File I/O ─────────────────────────────────────────────────

    /** Save overrides to a file (creates parent directories if needed).
     *  Returns true on success. */
    bool SaveToFile(const std::string& path);

    /** Load overrides from a file.  Returns true on success. */
    bool LoadFromFile(const std::string& path);

    /** Status / log messages produced during save/load operations. */
    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<PCGOverride> m_overrides;
    std::vector<std::string> m_log;
    bool m_dirty = false;

    void log(const std::string& msg);
};

} // namespace atlas::editor
