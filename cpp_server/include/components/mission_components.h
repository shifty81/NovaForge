#ifndef NOVAFORGE_COMPONENTS_MISSION_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_MISSION_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

/**
 * @brief Active mission tracking for a player entity
 * 
 * Tracks missions the player has accepted, their objectives,
 * and progress. Supports multiple concurrent missions.
 */
class MissionTracker : public ecs::Component {
public:
    struct Objective {
        std::string type;          // "destroy", "mine", "deliver", "reach"
        std::string target;        // entity type or item name
        int required = 1;
        int completed = 0;
        bool done() const { return completed >= required; }
    };

    struct ActiveMission {
        std::string mission_id;
        std::string name;
        int level = 1;
        std::string type;          // "combat", "mining", "courier"
        std::string agent_faction;
        std::vector<Objective> objectives;
        double isk_reward = 0.0;
        double lp_reward = 0.0;
        float standing_reward = 0.0f;
        float time_remaining = -1.0f;  // seconds, -1 = no limit
        bool completed = false;
        bool failed = false;

        bool allObjectivesDone() const {
            for (const auto& obj : objectives)
                if (!obj.done()) return false;
            return !objectives.empty();
        }
    };

    std::vector<ActiveMission> active_missions;
    std::vector<std::string> completed_mission_ids;

    COMPONENT_TYPE(MissionTracker)
};
/**
 * @brief An in-space anomaly (combat site, mining site, data site, etc.)
 *
 * Generated procedurally from a solar system seed.  Players discover
 * anomalies via the ScannerSystem and warp to them for content.
 */
class Anomaly : public ecs::Component {
public:
    enum class Type { Combat, Mining, Data, Relic, Gas, Wormhole };
    enum class Difficulty { Trivial, Easy, Medium, Hard, Deadly };

    std::string anomaly_id;
    std::string anomaly_name;
    std::string system_id;                  // owning solar system entity
    Type type = Type::Combat;
    Difficulty difficulty = Difficulty::Medium;
    float signature_strength = 0.5f;        // 0.0–1.0, affects scan difficulty
    float x = 0.0f;                         // position in system
    float y = 0.0f;
    float z = 0.0f;
    bool discovered = false;                // has anyone scanned this down?
    bool completed = false;                 // has content been cleared?
    float despawn_timer = 3600.0f;          // seconds until natural despawn
    int npc_count = 0;                      // NPCs to spawn on warp-in
    float loot_multiplier = 1.0f;           // scales drop quality

    COMPONENT_TYPE(Anomaly)
};

/**
 * @brief Visual distortion cue rendered near an anomaly
 *
 * Provides client-side rendering hints so anomalies have visible
 * spatial distortion effects — shimmering for wormholes, particle
 * clouds for gas sites, etc.  Intensity fades with distance.
 */
class AnomalyVisualCue : public ecs::Component {
public:
    enum class CueType { Shimmer, ParticleCloud, EnergyPulse, GravityLens, ElectricArc, None };

    std::string anomaly_id;                // linked anomaly entity
    CueType cue_type = CueType::None;
    float intensity = 1.0f;               // 0.0–1.0 base intensity
    float radius = 500.0f;                // metres — visible range
    float pulse_frequency = 0.5f;         // Hz — animation speed
    float r = 1.0f, g = 1.0f, b = 1.0f;  // tint colour
    float distortion_strength = 0.0f;     // 0.0–1.0 spatial warp amount
    bool active = true;

    COMPONENT_TYPE(AnomalyVisualCue)
};
/**
 * @brief Probe scanner — attached to ships that can scan for anomalies
 *
 * Players deploy probes to discover hidden anomalies in a solar system.
 * Scan strength and deviation improve with skill and probe count.
 */
class Scanner : public ecs::Component {
public:
    float scan_strength = 50.0f;         // base scan strength (affected by skills/modules)
    float scan_deviation = 4.0f;         // positional error in AU (decreases with better scans)
    int probe_count = 8;                 // number of probes deployed
    float scan_duration = 10.0f;         // seconds per scan cycle
    float scan_progress = 0.0f;          // current scan cycle progress
    bool scanning = false;               // currently scanning?
    std::string target_system_id;        // system being scanned
    
    struct ScanResult {
        std::string anomaly_id;
        float signal_strength = 0.0f;    // 0.0–1.0 (1.0 = fully scanned)
        float deviation = 0.0f;          // positional error remaining
        bool warpable = false;           // true when signal >= 1.0
    };
    
    std::vector<ScanResult> results;

    COMPONENT_TYPE(Scanner)
};

/**
 * @brief Per-system difficulty modifier based on security status
 *
 * Attached to solar system entities.  Scales NPC stats, spawn rates,
 * and loot quality based on the zone's security level.
 */
class DifficultyZone : public ecs::Component {
public:
    float security_status = 0.5f;        // 1.0 highsec → 0.0 nullsec
    float npc_hp_multiplier = 1.0f;      // applied to NPC health pools
    float npc_damage_multiplier = 1.0f;  // applied to NPC weapon damage
    float spawn_rate_multiplier = 1.0f;  // controls how often NPCs respawn
    float loot_quality_multiplier = 1.0f; // scales loot drop quality
    float ore_richness_multiplier = 1.0f; // scales mining yield
    int max_npc_tier = 1;                // highest NPC tier that can spawn (1-5)

    COMPONENT_TYPE(DifficultyZone)
};

/**
 * @brief A template for procedurally generating missions
 *
 * Stored as entities in a template library.  MissionTemplateSystem uses
 * these to produce concrete ActiveMission instances with deterministic
 * objective counts and scaled rewards.
 */
class MissionTemplate : public ecs::Component {
public:
    struct ObjectiveTemplate {
        std::string type;          // "destroy", "mine", "deliver", "reach"
        std::string target;        // target type (e.g., "pirate_frigate", "Ferrite", "Trade Goods")
        int count_min = 1;
        int count_max = 5;
    };

    std::string template_id;
    std::string name_pattern;      // e.g., "Pirate Clearance: {system}"
    std::string type;              // "combat", "mining", "courier", "trade", "exploration"
    int level = 1;                 // 1-5
    std::string required_faction;  // faction that offers this mission ("" = any)
    float min_standing = 0.0f;     // minimum faction standing required

    std::vector<ObjectiveTemplate> objective_templates;

    // Reward scaling
    double base_isk = 100000.0;
    double isk_per_level = 50000.0;
    float base_standing_reward = 0.1f;
    float standing_per_level = 0.05f;
    float base_time_limit = 3600.0f; // seconds, -1 = no limit

    COMPONENT_TYPE(MissionTemplate)
};

// ==================== Mission Consequence ====================

class MissionConsequence : public ecs::Component {
public:
    std::string consequence_id;
    std::string mission_id;
    std::string system_id;

    enum class ConsequenceType {
        StandingChange,
        SecurityShift,
        PriceImpact,
        SpawnChange,
        ReputationBoost,
        TerritoryShift,
        ResourceDepletion
    };

    ConsequenceType type = ConsequenceType::StandingChange;
    float magnitude = 0.0f;
    float duration = 300.0f;
    float elapsed = 0.0f;
    std::string target_faction;
    std::string target_item;
    bool is_active = false;
    bool is_permanent = false;
    bool is_expired = false;
    int times_triggered = 0;
    float decay_rate = 0.01f;

    struct ConsequenceEntry {
        std::string id;
        std::string mission_id;
        ConsequenceType type = ConsequenceType::StandingChange;
        float magnitude = 0.0f;
        float remaining_time = 0.0f;
        bool permanent = false;
    };

    std::vector<ConsequenceEntry> active_consequences;

    COMPONENT_TYPE(MissionConsequence)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_MISSION_COMPONENTS_H
