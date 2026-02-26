#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::sim {

/**
 * Archetype identifiers for AI NPC profiles.
 */
enum class AIArchetype : uint8_t {
    Miner,
    Hauler,
    Trader,
    Pirate,
    Security,
    Industrialist
};

/**
 * A single AI behavior profile loaded from data.
 *
 * Every field has a sensible default so the system works without any JSON.
 * Mod authors override individual fields via ai_profiles.json.
 */
struct AIProfile {
    std::string  name;                      // human-readable label
    AIArchetype  archetype       = AIArchetype::Miner;

    // Combat / engagement
    float orbitRange             = 5000.0f; // preferred orbit distance (units)
    float retreatThreshold       = 0.25f;   // HP fraction to trigger flee
    bool  focusFire              = false;    // concentrate fire on single target

    // Economy / cargo
    float cargoCapacity          = 400.0f;  // m³
    float travelSpeed            = 100.0f;  // units/s
    float loadRate               = 2.0f;    // m³/s (mining yield or load speed)

    // Spawn / lifecycle
    float respawnDelay           = 60.0f;   // seconds
    float hitPoints              = 100.0f;  // max HP
    float startingCredits        = 5000.0f; // initial wallet
};

/**
 * @brief Data-driven AI profile configuration loader.
 *
 * Manages named AIProfile entries keyed by a unique string id.
 * Profiles can be installed programmatically via AddProfile() or loaded from a
 * simple key-value text representation via LoadFromText().
 *
 * The text format (one profile per block) is intentionally simple:
 *
 *   [profile_id]
 *   name = Asteroid Miner
 *   archetype = Miner
 *   orbitRange = 3000
 *   ...
 *
 * This gives mod authors a way to tweak AI behavior without recompilation.
 *
 * Usage:
 *   AIProfileLoader loader;
 *   loader.InstallDefaults();          // 6 built-in archetype profiles
 *   loader.LoadFromText(fileContents); // override / extend
 *   const AIProfile* p = loader.GetProfile("pirate_elite");
 */
class AIProfileLoader {
public:
    AIProfileLoader() = default;

    /** Install the 6 built-in default profiles (one per archetype). */
    void InstallDefaults();

    /** Add or replace a profile with the given id. */
    bool AddProfile(const std::string& id, const AIProfile& profile);

    /** Remove a profile by id. Returns false if not found. */
    bool RemoveProfile(const std::string& id);

    /** Get a profile by id. Returns nullptr if not found. */
    const AIProfile* GetProfile(const std::string& id) const;

    /** Get all profiles for a given archetype. */
    std::vector<const AIProfile*> GetByArchetype(AIArchetype archetype) const;

    /** Number of loaded profiles. */
    size_t ProfileCount() const { return m_profiles.size(); }

    /**
     * Load profiles from a simple text format.
     * Returns number of profiles parsed (0 if input is empty).
     */
    size_t LoadFromText(const std::string& text);

    /** Get all profile ids. */
    std::vector<std::string> ProfileIds() const;

private:
    std::unordered_map<std::string, AIProfile> m_profiles;
};

} // namespace atlas::sim
