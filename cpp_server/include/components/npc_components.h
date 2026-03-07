#ifndef NOVAFORGE_COMPONENTS_NPC_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_NPC_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

// ==================== Phase 9: Fleet AI Components ====================

class PlayerPresence : public ecs::Component {
public:
    float time_since_last_command = 0.0f;   // seconds since last player command
    float time_since_last_speech = 0.0f;    // seconds since last player chat

    COMPONENT_TYPE(PlayerPresence)
};

class FactionCulture : public ecs::Component {
public:
    std::string faction;
    float chatter_frequency_mod = 1.0f;     // multiplier on base chatter rate
    float formation_tightness_mod = 1.0f;   // how tight formations are kept
    float morale_sensitivity = 1.0f;        // how much events affect morale
    float risk_tolerance = 0.5f;            // willingness to stay in danger

    COMPONENT_TYPE(FactionCulture)
};

// ==================== Phase 2: Living Universe Components ====================

/**
 * @brief Per-system state vector for background simulation
 *
 * Tracks economic, security, and faction state of a star system.
 * Updated by AtlasBackgroundSimulationSystem each tick.
 */
class SimStarSystemState : public ecs::Component {
public:
    // Traffic & activity
    float traffic_level = 0.5f;         // 0.0 = empty, 1.0 = busy
    float mining_activity = 0.0f;       // 0.0-1.0 active mining rate
    float trade_volume = 0.0f;          // 0.0-1.0 trade activity

    // Economy
    float economic_index = 0.5f;        // 0.0 = depressed, 1.0 = booming
    float resource_availability = 1.0f; // 0.0 = depleted, 1.0 = abundant
    float price_modifier = 1.0f;        // multiplier on base prices

    // Security
    float security_level = 0.5f;        // 0.0 = lawless, 1.0 = secure
    float threat_level = 0.0f;          // 0.0 = safe, 1.0 = dangerous
    float pirate_activity = 0.0f;       // 0.0-1.0 pirate presence

    // Faction influence
    std::map<std::string, float> faction_influence;  // faction -> 0.0-1.0

    // Event flags
    bool pirate_surge = false;
    bool resource_shortage = false;
    bool lockdown = false;
    float event_timer = 0.0f;           // countdown for active events

    COMPONENT_TYPE(SimStarSystemState)
};

/**
 * @brief Intent-driven NPC AI component
 *
 * NPCs choose intents based on system state, personality, and needs.
 * The intent drives behavior tree execution each tick.
 */
class SimNPCIntent : public ecs::Component {
public:
    enum class Intent {
        Idle,
        Trade,
        Patrol,
        Hunt,
        Explore,
        Flee,
        Escort,
        Salvage,
        Mine,
        Haul,
        Dock
    };

    enum class Archetype {
        Trader,
        Pirate,
        Patrol,
        Miner,
        Hauler,
        Industrialist
    };

    Intent current_intent = Intent::Idle;
    Intent previous_intent = Intent::Idle;
    Archetype archetype = Archetype::Trader;

    // Intent scoring weights (personality-driven)
    float trade_weight = 0.5f;
    float patrol_weight = 0.5f;
    float hunt_weight = 0.5f;
    float explore_weight = 0.5f;
    float flee_weight = 0.5f;
    float escort_weight = 0.5f;
    float salvage_weight = 0.5f;
    float mine_weight = 0.5f;
    float haul_weight = 0.5f;

    // State tracking
    std::string target_system_id;       // destination system
    std::string target_entity_id;       // target entity (trade partner, escort target, etc.)
    float intent_duration = 0.0f;       // how long on current intent
    float intent_cooldown = 0.0f;       // cooldown before re-evaluation
    bool intent_complete = false;       // current intent fulfilled

    // Economic state
    double wallet = 10000.0;            // Credits balance
    float cargo_fill = 0.0f;           // 0.0-1.0 cargo space used
    float profit_target = 0.0f;         // target profit before docking

    COMPONENT_TYPE(SimNPCIntent)
};

// ==================== Living Universe: NPC Behavior Trees ====================

/**
 * @brief Per-NPC behavior tree state
 *
 * Tracks which phase of the archetype behavior tree the NPC is
 * currently executing, how long it has been in that phase, and
 * the full phase list derived from the current intent.
 */
class NPCBehaviorState : public ecs::Component {
public:
    std::vector<std::string> phases;           // ordered phase names
    int current_phase_index = 0;               // index into phases[]
    float phase_elapsed = 0.0f;                // seconds in current phase
    float phase_duration = 10.0f;              // default time per phase
    bool tree_complete = false;                // all phases finished
    SimNPCIntent::Intent bound_intent = SimNPCIntent::Intent::Idle; // intent this tree was built for

    COMPONENT_TYPE(NPCBehaviorState)
};

// ==================== Living Universe: Security Response ====================

/**
 * @brief Per-system security response timer (AEGIS-style)
 *
 * Tracks whether a delayed security response is pending or active
 * for a star system.
 */
class SecurityResponseState : public ecs::Component {
public:
    float response_timer = 0.0f;        // countdown (or remaining duration)
    bool  responding = false;            // true when response is active
    float response_strength = 0.0f;     // proportional to security_level

    COMPONENT_TYPE(SecurityResponseState)
};

// ==================== Living Universe: Ambient Traffic ====================

/**
 * @brief Per-system ambient NPC traffic state
 *
 * Manages the spawn timer and pending spawn list for background
 * NPC traffic driven by the system's SimStarSystemState.
 */
class AmbientTrafficState : public ecs::Component {
public:
    float spawn_timer = 60.0f;          // countdown to next spawn evaluation
    int   active_traffic_count = 0;     // current NPC traffic count
    std::vector<std::string> pending_spawns;  // types awaiting spawn ("trader", "miner", …)

    COMPONENT_TYPE(AmbientTrafficState)
};
// ==================== Living Universe: NPC Rerouting, Reputation, News, Wrecks ====================

class NPCRouteState : public ecs::Component {
public:
    std::string current_system_id;
    std::string destination_system_id;
    std::vector<std::string> planned_route;
    bool rerouting = false;
    float danger_threshold = 0.6f;
    float reroute_cooldown = 0.0f;

    COMPONENT_TYPE(NPCRouteState)
};

class LocalReputation : public ecs::Component {
public:
    std::map<std::string, float> player_reputation;
    std::string system_id;
    float reputation_decay_rate = 0.01f;

    COMPONENT_TYPE(LocalReputation)
};

struct StationNewsEntry {
    std::string headline;
    std::string body;
    float timestamp = 0.0f;
    std::string category;
};

class StationNewsFeed : public ecs::Component {
public:
    std::vector<StationNewsEntry> entries;
    int max_entries = 20;

    void addEntry(const std::string& headline, const std::string& body,
                  float ts, const std::string& category) {
        StationNewsEntry entry;
        entry.headline = headline;
        entry.body = body;
        entry.timestamp = ts;
        entry.category = category;
        entries.push_back(entry);
        while (static_cast<int>(entries.size()) > max_entries) {
            entries.erase(entries.begin());
        }
    }

    COMPONENT_TYPE(StationNewsFeed)
};

class WreckPersistence : public ecs::Component {
public:
    float lifetime = 7200.0f;
    float elapsed = 0.0f;
    bool salvage_npc_assigned = false;
    std::string assigned_npc_id;

    COMPONENT_TYPE(WreckPersistence)
};

// ==================== Imperfect Information ====================

/**
 * @brief Tracks scan-derived intelligence about other entities
 *
 * Entity information has accuracy based on scan resolution, distance,
 * and time since last scan. Intel decays over time.
 */
class EntityIntel : public ecs::Component {
public:
    struct IntelEntry {
        std::string target_id;
        float confidence = 0.0f;       // 0.0 (no data) to 1.0 (perfect)
        float scan_quality = 0.0f;     // quality of the scan that produced this
        float age = 0.0f;              // seconds since recorded
        float decay_rate = 0.01f;      // confidence lost per second
        float distance_at_scan = 0.0f; // distance when scanned
        bool is_ghost = false;         // became a sensor ghost
    };

    std::vector<IntelEntry> entries;
    float sensor_strength = 1.0f;   // observer's sensor multiplier
    int max_entries = 50;
    float ghost_threshold = 0.1f;   // below this confidence, mark as ghost
    int total_scans = 0;

    IntelEntry* getEntry(const std::string& target_id) {
        for (auto& e : entries) {
            if (e.target_id == target_id) return &e;
        }
        return nullptr;
    }

    const IntelEntry* getEntry(const std::string& target_id) const {
        for (const auto& e : entries) {
            if (e.target_id == target_id) return &e;
        }
        return nullptr;
    }

    COMPONENT_TYPE(EntityIntel)
};

/**
 * @brief AI faction response escalation to galactic threats
 *
 * Tracks threat level, response tiers (0-4), reinforcement dispatches,
 * and trade rerouting for a faction responding to escalating threats.
 */
class GalacticResponseCurve : public ecs::Component {
public:
    std::string faction_id;
    float threat_level = 0.0f;       // accumulated threat 0.0+
    float escalation_rate = 1.0f;    // multiplier for threat accumulation
    float decay_rate = 0.1f;         // threat decay per second
    int response_tier = 0;           // 0=None, 1=Alert, 2=Mobilize, 3=Reinforce, 4=FullMobilization
    int reinforcements_dispatched = 0;
    std::vector<std::string> rerouted_systems;
    int max_rerouted = 20;
    bool active = true;

    // Tier thresholds
    static constexpr float TIER_1_THRESHOLD = 10.0f;  // Alert
    static constexpr float TIER_2_THRESHOLD = 25.0f;  // Mobilize
    static constexpr float TIER_3_THRESHOLD = 50.0f;  // Reinforce
    static constexpr float TIER_4_THRESHOLD = 80.0f;  // Full Mobilization

    COMPONENT_TYPE(GalacticResponseCurve)
};

/**
 * @brief NPC daily activity schedule for creating visible economic cycles
 *
 * Tracks per-NPC activity schedules with time-of-day driven transitions
 * between activities (mining, hauling, trading, patrolling, resting).
 * Enables observable AI economic patterns.
 */
class NPCSchedule : public ecs::Component {
public:
    enum class Activity { Idle, Mining, Hauling, Trading, Patrolling, Resting, Docking };

    struct ScheduleEntry {
        Activity activity = Activity::Idle;
        float start_hour = 0.0f;      // 0-24 hour of day
        float end_hour = 0.0f;
        std::string location;
        int priority = 1;              // 1 = lowest, 5 = highest
    };

    std::vector<ScheduleEntry> schedule;
    Activity current_activity = Activity::Idle;
    float current_hour = 0.0f;         // 0-24, wraps
    float day_length = 86400.0f;       // seconds per game day (default 24h real-time)
    float elapsed_day_time = 0.0f;
    int max_entries = 24;
    int transitions = 0;               // total activity transitions
    int days_completed = 0;
    float adherence_score = 1.0f;      // 0-1, how well NPC follows schedule
    bool active = true;

    COMPONENT_TYPE(NPCSchedule)
};

/**
 * @brief Per-entity aggro table for NPC AI targeting decisions
 *
 * Tracks accumulated threat from each attacker.  Each tick the system decays
 * old entries and exposes the highest-threat attacker for AI targeting.
 */
class AggroTable : public ecs::Component {
public:
    struct AggroEntry {
        std::string attacker_id;
        float threat = 0.0f;
        float last_hit_time = 0.0f;
    };

    std::vector<AggroEntry> entries;
    float decay_rate = 2.0f;        // threat units per second
    float decay_delay = 5.0f;       // seconds after last hit before decay starts
    int max_entries = 20;
    int total_threat_events = 0;
    float total_threat_accumulated = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(AggroTable)
};

/**
 * @brief NPC spawn schedule for asteroid belts, gate camps, and mission pockets
 *
 * Manages a cyclic wave table with a population cap and respawn interval.
 * When the live count drops below cap, the next wave spawns after the
 * timer elapses.  Supports pausing and tracks total spawned/killed.
 */
class NpcSpawnSchedule : public ecs::Component {
public:
    struct WaveEntry {
        std::string npc_type;
        int count = 1;
    };

    std::vector<WaveEntry> wave_entries;
    int max_wave_entries = 10;
    int population_cap = 10;
    float respawn_interval = 30.0f;  // seconds
    float respawn_timer = 0.0f;
    int current_wave_index = 0;
    int live_count = 0;
    int total_spawned = 0;
    int total_killed = 0;
    bool paused = false;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(NpcSpawnSchedule)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_NPC_COMPONENTS_H
