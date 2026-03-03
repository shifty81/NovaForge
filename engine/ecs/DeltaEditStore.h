#pragma once
/**
 * @file DeltaEditStore.h
 * @brief Records and replays structured world edits on top of a PCG seed.
 *
 * The DeltaEditStore captures every add/move/remove/set-property action
 * performed via the ToolingLayer so that edits layer persistently on top
 * of procedural generation.  Edits serialize to/from JSON for save files
 * and repo commits, and can be replayed against a fresh World to
 * reproduce the designer's modifications.
 *
 * Typical flow:
 *   1. PCGEngine generates World from seed.
 *   2. Designer opens ToolingLayer, makes edits.
 *   3. Edits recorded in DeltaEditStore.
 *   4. Save: Serialize(seed + edits) → JSON.
 *   5. Load: Generate(seed) + ApplyAll(edits).
 */

#include <cstdint>
#include <string>
#include <vector>

namespace atlas::ecs {

/** Types of structured edits the store can record. */
enum class DeltaEditType : uint8_t {
    AddObject,       ///< Place a new object into the world
    RemoveObject,    ///< Delete an existing object
    MoveObject,      ///< Change an object's position
    SetProperty      ///< Change an arbitrary named property on an object
};

/** Returns a human-readable string for a DeltaEditType. */
const char* DeltaEditTypeName(DeltaEditType type);

/**
 * A single recorded edit.
 *
 * All fields are plain-old-data so the struct can be serialized trivially.
 * Position uses float[3] so it is independent of any math library.
 */
struct DeltaEdit {
    DeltaEditType type;
    uint32_t      entityID   = 0;      ///< Target entity (0 = none / auto)
    std::string   objectType;          ///< "planet", "station", "ship", etc.
    float         position[3] = {};    ///< World position  (AddObject / MoveObject)
    std::string   propertyName;        ///< For SetProperty
    std::string   propertyValue;       ///< For SetProperty (string-encoded)
};

/**
 * Persistent record of all world edits layered on top of a PCG seed.
 *
 * Thread safety: not thread-safe; assumes single-threaded editor access.
 */
class DeltaEditStore {
public:
    /** Construct with an optional seed (default 0). */
    explicit DeltaEditStore(uint64_t seed = 0) : m_seed(seed) {}

    // ── Seed ────────────────────────────────────────────────────────
    uint64_t Seed() const { return m_seed; }
    void     SetSeed(uint64_t seed) { m_seed = seed; }

    // ── Recording ──────────────────────────────────────────────────
    /** Append an edit to the store. */
    void Record(const DeltaEdit& edit);

    /** Number of recorded edits. */
    size_t Count() const { return m_edits.size(); }

    /** Read-only access to all edits (in recording order). */
    const std::vector<DeltaEdit>& Edits() const { return m_edits; }

    /** Remove all recorded edits (seed is preserved). */
    void Clear();

    // ── Serialization ──────────────────────────────────────────────
    /** Serialize the seed + all edits to a JSON string. */
    std::string SerializeToJSON() const;

    /** Deserialize from a JSON string, replacing current contents. */
    bool DeserializeFromJSON(const std::string& json);

private:
    uint64_t               m_seed = 0;
    std::vector<DeltaEdit> m_edits;
};

} // namespace atlas::ecs
