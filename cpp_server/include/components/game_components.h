#ifndef NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H

// Aggregate header — includes all component domain headers for backwards
// compatibility.  New code should include the specific domain header it needs.

#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/fleet_components.h"
#include "components/economy_components.h"
#include "components/social_components.h"
#include "components/combat_components.h"
#include "components/mission_components.h"
#include "components/ship_components.h"
#include "components/fps_components.h"
#include "components/crew_components.h"
#include "components/exploration_components.h"
#include "components/narrative_components.h"
#include "components/npc_components.h"
#include "components/ui_components.h"
#include <map>

namespace atlas {
namespace components {

/**
 * @brief Player progression tracking — XP, levels, and milestones
 *
 * Tracks player experience across multiple categories (combat, mining,
 * exploration, industry, social), computes overall level, and records
 * milestone achievements.
 */
class PlayerProgression : public ecs::Component {
public:
    struct Milestone {
        std::string name;
        std::string category;
        float xp_required = 0.0f;
        bool achieved = false;
    };

    // XP tracking by category
    float combat_xp = 0.0f;
    float mining_xp = 0.0f;
    float exploration_xp = 0.0f;
    float industry_xp = 0.0f;
    float social_xp = 0.0f;
    float total_xp = 0.0f;

    // Level calculation
    int level = 1;
    float xp_to_next_level = 100.0f;  // XP needed for next level
    float level_progress = 0.0f;       // 0.0-1.0 progress to next level

    // Milestones
    std::vector<Milestone> milestones;
    int milestones_achieved = 0;

    // Prestige (for endgame)
    int prestige_level = 0;
    float prestige_multiplier = 1.0f;  // XP gain multiplier from prestige

    COMPONENT_TYPE(PlayerProgression)
};

// ==================== Server Performance Metrics ====================

class ServerPerformanceMetrics : public ecs::Component {
public:
    std::string monitor_id;
    std::string server_id;
    float tick_budget_ms = 50.0f;  // 50ms = 20Hz tick rate

    struct SystemTiming {
        std::string system_name;
        float last_time_ms = 0.0f;
        float avg_time_ms = 0.0f;
        float max_time_ms = 0.0f;
        int sample_count = 0;
    };

    std::vector<SystemTiming> system_timings;
    int entity_count = 0;
    int component_count = 0;
    float total_tick_time_ms = 0.0f;
    float avg_tick_time_ms = 0.0f;
    float max_tick_time_ms = 0.0f;
    int ticks_over_budget = 0;
    int total_ticks = 0;
    float budget_utilization = 0.0f;
    bool alert_active = false;
    float alert_threshold = 0.8f;
    int hot_path_count = 0;
    std::string slowest_system;

    SystemTiming* findTiming(const std::string& name) {
        for (auto& t : system_timings) {
            if (t.system_name == name) return &t;
        }
        return nullptr;
    }

    COMPONENT_TYPE(ServerPerformanceMetrics)
};

// ==================== Entity Stress Test ====================

/**
 * @brief Stress test metrics for 500+ entity performance validation
 *
 * Tracks entity creation timing, per-tick performance, query latency,
 * and whether the system stays within the 20Hz tick budget threshold.
 */
class EntityStressTest : public ecs::Component {
public:
    enum class StressPhase {
        Idle,
        Creating,
        Running,
        Complete
    };

    std::string test_id;
    std::string server_id;
    int entity_count = 0;
    int target_count = 500;

    std::vector<float> tick_times;
    float avg_tick_ms = 0.0f;
    float max_tick_ms = 0.0f;

    int queries_per_tick = 0;
    float avg_query_us = 0.0f;

    float entity_creation_time_ms = 0.0f;
    bool passed_threshold = false;
    float threshold_ms = 50.0f;  // 50ms = 20Hz tick budget

    StressPhase stress_phase = StressPhase::Idle;

    COMPONENT_TYPE(EntityStressTest)
};

// ==================== Client Prediction ====================

class ClientPrediction : public ecs::Component {
public:
    std::string client_id;
    float predicted_x = 0.0f, predicted_y = 0.0f, predicted_z = 0.0f;
    float server_x = 0.0f, server_y = 0.0f, server_z = 0.0f;
    float velocity_x = 0.0f, velocity_y = 0.0f, velocity_z = 0.0f;
    float correction_blend = 0.0f;
    float correction_speed = 5.0f;
    float prediction_error = 0.0f;
    float max_prediction_window = 0.2f;
    int prediction_frame = 0;
    int last_server_frame = 0;
    bool active = false;

    COMPONENT_TYPE(ClientPrediction)
};

// ==================== Ship Template Mod ====================

class ShipTemplateMod : public ecs::Component {
public:
    std::string template_id;
    std::string base_template_id;
    std::string mod_source;
    std::string ship_name;
    std::string ship_class;
    std::string faction;
    float hull_hp = 100.0f, shield_hp = 100.0f, armor_hp = 100.0f;
    float max_velocity = 300.0f, agility = 1.0f;
    int high_slots = 3, mid_slots = 3, low_slots = 3;
    int priority = 0;
    bool validated = false;
    bool is_override = false;
    bool needs_inheritance = true;

    COMPONENT_TYPE(ShipTemplateMod)
};

// ==================== Database Persistence ====================

class DatabasePersistence : public ecs::Component {
public:
    std::string db_name;
    std::map<std::string, std::string> store;
    float auto_save_interval = 60.0f;
    float save_timer = 0.0f;
    int total_writes = 0;
    int total_reads = 0;
    int save_count = 0;
    bool dirty = false;
    bool auto_save_enabled = true;

    COMPONENT_TYPE(DatabasePersistence)
};

// ==================== Task Scheduler ====================

class TaskScheduler : public ecs::Component {
public:
    enum TaskState { Queued = 0, Running = 1, Complete = 2, Failed = 3, Cancelled = 4 };
    enum Priority { Low = 0, Normal = 1, High = 2, Critical = 3 };

    struct TaskEntry {
        int id = 0;
        std::string name;
        int priority = Normal;
        int state = Queued;
        float progress = 0.0f;
        std::vector<int> dependencies;
    };

    std::vector<TaskEntry> tasks;
    int max_concurrent = 4;
    int total_completed = 0;
    int total_failed = 0;
    float throughput = 0.0f;
    float total_time = 0.0f;
    int next_task_id = 1;
    bool active = true;

    COMPONENT_TYPE(TaskScheduler)
};

// ==================== Mod Manager ====================

class ModManager : public ecs::Component {
public:
    struct ModEntry {
        std::string mod_id;
        std::string name;
        std::string version;
        std::string author;
        std::vector<std::string> dependencies;
        bool enabled = false;
        bool installed = false;
        int load_order = 0;
    };

    std::vector<ModEntry> mods;
    std::vector<std::string> conflicts;
    int max_mods = 50;
    int total_installed = 0;
    int total_enabled = 0;
    bool active = true;

    COMPONENT_TYPE(ModManager)
};

// ==================== Ship Designer ====================

class ShipDesigner : public ecs::Component {
public:
    struct SlotEntry {
        std::string module_name;
        int slot_type = 0;
        float cpu_cost = 0.0f;
        float power_cost = 0.0f;
    };

    std::string blueprint_name;
    std::string hull_type;
    std::string faction;
    std::vector<SlotEntry> fitted_modules;
    int high_slots = 4;
    int mid_slots = 4;
    int low_slots = 3;
    int rig_slots = 3;
    float total_cpu = 300.0f;
    float total_powergrid = 1000.0f;
    float used_cpu = 0.0f;
    float used_powergrid = 0.0f;
    float hull_hp = 1000.0f;
    float shield_hp = 500.0f;
    float armor_hp = 500.0f;
    float max_velocity = 200.0f;
    int design_count = 0;
    bool valid = false;
    bool active = true;

    COMPONENT_TYPE(ShipDesigner)
};

// ==================== Mission Editor ====================

class MissionEditor : public ecs::Component {
public:
    enum ObjectiveType { Kill = 0, Deliver = 1, Escort = 2, Scan = 3, Mine = 4, Salvage = 5, Defend = 6 };
    enum MissionType { Combat = 0, Mining = 1, Courier = 2, Exploration = 3, Rescue = 4 };

    struct Objective {
        int id = 0;
        std::string description;
        int type = Kill;
    };

    std::string mission_name;
    int mission_level = 1;
    int mission_type = Combat;
    std::vector<Objective> objectives;
    float reward_credits = 0.0f;
    float reward_standing = 0.0f;
    int next_objective_id = 1;
    int published_count = 0;
    std::string validation_error;
    bool active = true;

    COMPONENT_TYPE(MissionEditor)
};

// ==================== Content Validation ====================

class ContentValidation : public ecs::Component {
public:
    enum ContentType { Ship = 0, Module = 1, Mission = 2, Skill = 3 };
    enum ContentState { Pending = 0, Validating = 1, Approved = 2, Rejected = 3 };

    struct ContentEntry {
        std::string content_id;
        std::string name;
        int content_type = Ship;
        int state = Pending;
        std::string rejection_reason;
    };

    std::vector<ContentEntry> entries;
    int total_validations = 0;
    int approved_count = 0;
    int rejected_count = 0;
    bool active = true;

    ContentEntry* findEntry(const std::string& content_id) {
        for (auto& e : entries) {
            if (e.content_id == content_id) return &e;
        }
        return nullptr;
    }

    const ContentEntry* findEntry(const std::string& content_id) const {
        for (const auto& e : entries) {
            if (e.content_id == content_id) return &e;
        }
        return nullptr;
    }

    COMPONENT_TYPE(ContentValidation)
};

// ==================== Cloud Deployment Config ====================

class CloudDeploymentConfig : public ecs::Component {
public:
    enum Provider { AWS = 0, GCP = 1, Azure = 2 };

    int provider = AWS;
    std::string region;
    std::string instance_type;
    int max_players = 20;
    float health_check_interval = 30.0f;
    bool health_check_enabled = false;
    bool deployed = false;
    float uptime = 0.0f;
    int health_check_count = 0;
    float estimated_monthly_cost = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CloudDeploymentConfig)
};

// ==================== Interest Priority ====================

class InterestPriority : public ecs::Component {
public:
    std::string entity_id;
    int client_id = 0;
    int priority_tier = 2;
    float update_interval = 0.1f;
    float time_since_update = 0.0f;
    bool needs_update = true;
    float distance = 0.0f;
    float bandwidth_weight = 1.0f;
    int total_updates = 0;
    bool active = true;

    COMPONENT_TYPE(InterestPriority)
};

// ==================== Visual Feedback Queue ====================

class VisualFeedbackQueue : public ecs::Component {
public:
    struct FeedbackEffect {
        int id = 0;
        int category = 0;
        float intensity = 1.0f;
        float lifetime = 1.0f;
        float max_lifetime = 1.0f;
        int priority = 0;
        bool fading = false;
        std::string label;
    };

    std::vector<FeedbackEffect> effects;
    int next_effect_id = 1;
    int max_effects = 20;
    int total_effects_queued = 0;
    int total_effects_expired = 0;
    bool active = true;

    COMPONENT_TYPE(VisualFeedbackQueue)
};

// ==================== Mod Doc Generator ====================

class ModDocGenerator : public ecs::Component {
public:
    struct DocEntry {
        std::string type_name;
        std::string category;
        std::string description;
        int field_count = 0;
        bool has_example = false;
        bool validated = false;
    };

    std::vector<DocEntry> entries;
    std::string title = "Nova Forge Modding Reference";
    std::string version = "1.0";
    int total_entries = 0;
    int total_validated = 0;
    bool generated = false;
    int generation_count = 0;
    int max_entries = 100;
    bool active = true;

    COMPONENT_TYPE(ModDocGenerator)
};

class CommunityContentRepo : public ecs::Component {
public:
    struct ContentEntry {
        std::string content_id;
        std::string type;
        std::string author;
        std::string title;
        std::string description;
        std::string state = "Draft";
        float average_rating = 0.0f;
        int total_rating = 0;
        int rating_count = 0;
        int downloads = 0;
    };

    std::vector<ContentEntry> contents;
    int max_content = 200;
    int total_submissions = 0;
    int total_downloads = 0;
    bool active = true;

    COMPONENT_TYPE(CommunityContentRepo)
};

class PvPState : public ecs::Component {
public:
    bool pvp_enabled = false;
    std::string safety_level = "HighSec";
    float engagement_timer = 300.0f;
    float aggression_timer = 0.0f;
    std::string last_target;
    int kill_count = 0;
    float bounty = 0.0f;
    float security_status = 5.0f;
    bool active = true;

    COMPONENT_TYPE(PvPState)
};

class DynamicEvent : public ecs::Component {
public:
    struct EventEntry {
        std::string event_id;
        std::string type;
        std::string state = "Pending";
        float duration = 0.0f;
        float elapsed_time = 0.0f;
        float intensity = 0.5f;
        float reward_pool = 0.0f;
        float start_delay = 5.0f;
        std::vector<std::string> participants;
    };

    std::vector<EventEntry> events;
    int max_concurrent_events = 5;
    int total_completed = 0;
    bool active = true;

    COMPONENT_TYPE(DynamicEvent)
};

/**
 * @brief Tracks long-term consequences of player actions with magnitude and decay
 */
class PersistenceDelta : public ecs::Component {
public:
    std::string tracker_id;
    struct DeltaEntry {
        std::string action_id;
        std::string category; // Combat, Trade, Diplomacy, Exploration, Crime
        float magnitude = 0.0f;
        float timestamp = 0.0f;
        float decay_rate = 0.0f;
        bool permanent = false;
    };
    std::vector<DeltaEntry> entries;
    int max_entries = 100;
    float total_positive_impact = 0.0f;
    float total_negative_impact = 0.0f;
    int actions_recorded = 0;
    float consequence_threshold = 10.0f;
    bool consequence_triggered = false;
    bool active = true;

    COMPONENT_TYPE(PersistenceDelta)
};

// ==================== Snapshot Replication System ====================

class SnapshotReplication : public ecs::Component {
public:
    struct EntitySnapshot {
        std::string entity_id;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float health = 100.0f;
        float shield = 100.0f;
        float velocity = 0.0f;
        uint32_t frame_number = 0;
        bool dirty = false;
    };

    struct SnapshotFrame {
        uint32_t frame_number = 0;
        float timestamp = 0.0f;
        std::vector<EntitySnapshot> entities;
    };

    std::string server_id;
    std::vector<SnapshotFrame> history;
    int max_history = 60;          // keep last 60 frames
    uint32_t current_frame = 0;
    float snapshot_interval = 0.05f; // 20 Hz
    float time_accumulator = 0.0f;
    int total_snapshots_sent = 0;
    int total_deltas_sent = 0;

    struct ClientAck {
        std::string client_id;
        uint32_t last_acked_frame = 0;
    };
    std::vector<ClientAck> client_acks;
    int max_clients = 20;
    bool active = true;

    COMPONENT_TYPE(SnapshotReplication)
};

/**
 * @brief Star system manager for vertical slice orchestration
 *
 * Manages a complete star system: celestial bodies, stations, gates, belts,
 * and NPC presence. Enables the full fly/fight/mine/trade/dock loop.
 */
class StarSystemState : public ecs::Component {
public:
    struct CelestialBody {
        std::string body_id;
        std::string name;
        std::string type;  // "Star", "Planet", "Moon", "AsteroidBelt"
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float radius = 1000.0f;
        bool active = true;
    };

    struct StationInfo {
        std::string station_id;
        std::string name;
        std::string owner_faction;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        int docked_count = 0;
        int max_docking = 50;
        bool has_market = true;
        bool has_repair = true;
        bool online = true;
    };

    struct GateLink {
        std::string gate_id;
        std::string destination_system;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool online = true;
        int total_jumps = 0;
    };

    struct NPCPresence {
        std::string faction;
        int ship_count = 0;
        float threat_level = 0.0f;  // 0.0 = none, 1.0 = max
        bool hostile = false;
    };

    std::string system_id;
    std::string system_name;
    float security_status = 1.0f;  // 0.0 = null-sec, 1.0 = high-sec
    std::vector<CelestialBody> celestials;
    std::vector<StationInfo> stations;
    std::vector<GateLink> gates;
    std::vector<NPCPresence> npc_presence;
    int max_celestials = 30;
    int max_stations = 10;
    int max_gates = 5;
    int max_npc_factions = 8;
    int total_dockings = 0;
    int total_jumps = 0;
    int total_npc_spawns = 0;
    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(StarSystemState)
};

/**
 * @brief Grid-based spatial partition for efficient entity queries
 *
 * Divides 3D space into grid cells for O(1) neighbor lookups.
 * Supports the 500+ entity performance target for client polish.
 */
class SpatialPartition : public ecs::Component {
public:
    struct GridEntry {
        std::string entity_id;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        int cell_x = 0;
        int cell_y = 0;
        int cell_z = 0;
        bool active = true;
    };

    struct CellStats {
        int cell_x = 0;
        int cell_y = 0;
        int cell_z = 0;
        int entity_count = 0;
    };

    std::vector<GridEntry> entries;
    float cell_size = 1000.0f;  // Size of each grid cell in world units
    int max_entries = 1000;
    int total_inserts = 0;
    int total_removals = 0;
    int total_queries = 0;
    int total_rebuilds = 0;
    float rebuild_interval = 1.0f;  // seconds between full rebuilds
    float rebuild_timer = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SpatialPartition)
};

// ==================== Player Session State ====================

/**
 * @brief Player session state for connect/disconnect/character management
 *
 * Tracks a player's connection lifecycle, selected character, session
 * duration, and reconnection history.  Enables the vertical-slice
 * game loop where a player connects, selects a character, plays, and
 * can reconnect after a disconnect.
 */
class PlayerSession : public ecs::Component {
public:
    enum class SessionState {
        Disconnected = 0,
        Connecting,
        CharacterSelect,
        Loading,
        InGame,
        Reconnecting
    };

    struct CharacterSlot {
        std::string character_id;
        std::string character_name;
        std::string ship_type;
        std::string location;       // star system or station
        float play_time = 0.0f;     // total play time in seconds
        bool active = false;
    };

    std::string player_id;
    std::string display_name;
    SessionState state = SessionState::Disconnected;
    std::string selected_character_id;
    std::vector<CharacterSlot> character_slots;
    int max_character_slots = 3;

    // Session tracking
    float session_start_time = 0.0f;
    float session_duration = 0.0f;
    float total_play_time = 0.0f;
    int login_count = 0;
    int disconnect_count = 0;
    int reconnect_count = 0;
    float last_heartbeat = 0.0f;
    float heartbeat_timeout = 30.0f;  // seconds before considered disconnected

    bool active = true;

    COMPONENT_TYPE(PlayerSession)
};

// ==================== Save Game State ====================

/**
 * @brief Save game state for persisting player progress
 *
 * Manages save slots with metadata (timestamp, location, playtime).
 * Tracks what needs to be serialized: ship loadout, wallet balance,
 * skill state, standing, cargo, and mission progress.
 */
class SaveGameState : public ecs::Component {
public:
    enum class SaveStatus {
        Idle = 0,
        Saving,
        Loading,
        Error
    };

    struct SaveSlot {
        std::string slot_id;
        std::string character_name;
        std::string location;
        std::string ship_type;
        double wallet_balance = 0.0;
        int skill_points = 0;
        float play_time = 0.0f;
        float save_timestamp = 0.0f;
        bool occupied = false;
        bool corrupted = false;
    };

    std::string owner_id;       // player who owns this save data
    std::vector<SaveSlot> slots;
    int max_slots = 5;
    SaveStatus status = SaveStatus::Idle;

    // Dirty flags — what changed since last save
    bool ship_dirty = false;
    bool wallet_dirty = false;
    bool skills_dirty = false;
    bool standings_dirty = false;
    bool cargo_dirty = false;
    bool missions_dirty = false;

    // Stats
    int total_saves = 0;
    int total_loads = 0;
    int save_errors = 0;
    float last_save_time = 0.0f;
    float auto_save_interval = 300.0f;  // 5 minutes
    float auto_save_timer = 0.0f;
    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SaveGameState)
};

// ==================== Onboarding State ====================

/**
 * @brief Onboarding / tutorial state for new-player experience
 *
 * Tracks tutorial progress through ordered objectives, hints shown,
 * and completion milestones.  Designed for the vertical-slice first
 * play-through: undock → fly → mine → sell → fit → fight → warp.
 */
class OnboardingState : public ecs::Component {
public:
    enum class TutorialPhase {
        NotStarted = 0,
        Welcome,
        Undocking,
        BasicFlight,
        Mining,
        Trading,
        ShipFitting,
        Combat,
        Warping,
        Completed
    };

    struct Objective {
        std::string objective_id;
        std::string description;
        TutorialPhase phase = TutorialPhase::NotStarted;
        bool completed = false;
        float completed_at = 0.0f;
    };

    struct HintEntry {
        std::string hint_id;
        std::string text;
        bool shown = false;
        float shown_at = 0.0f;
    };

    std::string player_id;
    TutorialPhase current_phase = TutorialPhase::NotStarted;
    std::vector<Objective> objectives;
    std::vector<HintEntry> hints;
    int max_objectives = 50;
    int max_hints = 30;

    // Progress
    int objectives_completed = 0;
    int hints_shown = 0;
    float start_time = 0.0f;
    float completion_time = 0.0f;
    bool tutorial_complete = false;
    bool tutorial_skipped = false;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(OnboardingState)
};

/**
 * @brief Data for star system content population (vertical-slice seeding)
 */
class StarSystemPopulation : public ecs::Component {
public:
    struct StationSeed {
        std::string station_id;
        std::string station_name;
        std::string station_type; // "Trade", "Industrial", "Military"
        bool spawned = false;
    };

    struct AsteroidBeltSeed {
        std::string belt_id;
        std::string ore_type; // "Veldspar", "Scordite", "Pyroxeres"
        int richness = 1;     // 1–5 scale
        bool spawned = false;
    };

    struct NPCFactionSeed {
        std::string faction_id;
        std::string faction_name;
        int spawn_count = 0;
        bool spawned = false;
    };

    struct JumpGateSeed {
        std::string gate_id;
        std::string destination_system;
        bool spawned = false;
    };

    struct PointOfInterestSeed {
        std::string poi_id;
        std::string poi_type; // "Anomaly", "Wreck", "Beacon", "Landmark"
        std::string description;
        bool spawned = false;
    };

    std::string system_id;
    std::string system_name;
    float security_status = 1.0f;

    std::vector<StationSeed> stations;
    std::vector<AsteroidBeltSeed> asteroid_belts;
    std::vector<NPCFactionSeed> npc_factions;
    std::vector<JumpGateSeed> jump_gates;
    std::vector<PointOfInterestSeed> points_of_interest;

    int max_stations = 10;
    int max_asteroid_belts = 15;
    int max_npc_factions = 8;
    int max_jump_gates = 5;
    int max_pois = 20;

    bool populated = false;
    float population_time = 0.0f;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(StarSystemPopulation)
};

/**
 * @brief Tracks cumulative player progress and session statistics
 */
class SessionProgression : public ecs::Component {
public:
    struct Milestone {
        std::string milestone_id;
        std::string description;
        float timestamp = 0.0f;
        bool reached = false;
    };

    struct Statistic {
        std::string stat_key;
        double value = 0.0;
    };

    struct ActivityEntry {
        std::string activity_type;
        std::string detail;
        float timestamp = 0.0f;
    };

    std::string player_id;

    std::vector<Milestone> milestones;
    std::vector<Statistic> statistics;
    std::vector<ActivityEntry> activities;

    int max_milestones = 50;
    int max_activities = 200;

    float session_start_time = 0.0f;
    float session_end_time = 0.0f;
    bool session_finalized = false;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SessionProgression)
};

/**
 * @brief Manages dynamic encounter lifecycle during vertical-slice gameplay
 */
class EncounterState : public ecs::Component {
public:
    enum class Status {
        Pending = 0,
        Active,
        Completed,
        Failed,
        Expired
    };

    struct Encounter {
        std::string encounter_id;
        std::string encounter_type; // "PirateAmbush", "TradeEscort", "CombatChallenge", "WarpInterdiction"
        int difficulty = 1;         // 1–5 scale
        float duration = 0.0f;     // max duration in seconds
        float started_at = 0.0f;
        float ended_at = 0.0f;
        Status status = Status::Pending;
        double isc_reward = 0.0;
        int loot_count = 0;
    };

    std::string system_id;

    std::vector<Encounter> encounters;
    int max_encounters = 20;

    int active_count = 0;
    int completed_count = 0;
    int failed_count = 0;
    double total_rewards_earned = 0.0;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(EncounterState)
};

/**
 * @brief Real-time objective tracking with waypoints and completion for HUD
 */
class ObjectiveTrackerState : public ecs::Component {
public:
    struct TrackedObjective {
        std::string objective_id;
        std::string description;
        std::string category; // "Mission", "Tutorial", "Exploration", "Combat"
        float target_x = 0.0f;
        float target_y = 0.0f;
        float progress = 0.0f; // 0.0–1.0
        bool completed = false;
        float completed_at = 0.0f;
    };

    std::string player_id;

    std::vector<TrackedObjective> objectives;
    int max_objectives = 30;
    std::string active_objective_id;

    float player_x = 0.0f;
    float player_y = 0.0f;

    int completed_count = 0;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ObjectiveTrackerState)
};

/**
 * @brief Centralized notification feed for player-facing gameplay events
 */
class EventNotificationFeed : public ecs::Component {
public:
    struct Notification {
        std::string event_id;
        std::string category; // "Combat", "Trade", "Mining", "Docking", "Mission", "System"
        std::string message;
        int priority = 1;     // 1 (low) – 5 (critical)
        float lifetime = 0.0f; // 0 = permanent until dismissed
        float age = 0.0f;
        bool read = false;
    };

    std::string player_id;

    std::vector<Notification> notifications;
    int max_notifications = 50;

    int total_pushed = 0;
    int total_expired = 0;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(EventNotificationFeed)
};

/**
 * @brief End-of-session summary with stats, grades, and derived metrics
 */
class SessionSummaryState : public ecs::Component {
public:
    struct SummaryStat {
        std::string stat_key;
        double value = 0.0;
    };

    struct CategoryStats {
        std::string category; // "Combat", "Economy", "Exploration", "Mission"
        std::vector<SummaryStat> entries;
    };

    std::string player_id;

    std::vector<SummaryStat> stats;
    std::vector<CategoryStats> categories;

    int max_stats = 50;
    int max_categories = 10;
    int max_category_entries = 20;

    float session_start_time = 0.0f;
    float session_end_time = 0.0f;
    bool finalized = false;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SessionSummaryState)
};

/**
 * @brief Market order browser state for trade hub interactions
 */
class MarketBrowserState : public ecs::Component {
public:
    struct MarketOrder {
        std::string order_id;
        std::string item_name;
        bool is_buy_order = false;
        double price = 0.0;
        int quantity = 0;
    };

    struct Transaction {
        std::string item_name;
        bool is_buy = false;
        double price = 0.0;
        int quantity = 0;
        float timestamp = 0.0f;
    };

    std::string player_id;

    std::vector<MarketOrder> orders;
    int max_orders = 200;

    std::string current_filter;

    std::vector<std::string> favorites;
    int max_favorites = 50;

    std::vector<Transaction> transactions;
    int max_transactions = 100;

    double total_spent = 0.0;
    double total_earned = 0.0;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(MarketBrowserState)
};

/**
 * @brief Skill training queue with time-based progression
 */
class SkillQueueState : public ecs::Component {
public:
    struct QueuedSkill {
        std::string skill_id;
        int target_level = 1;
        float train_duration = 0.0f;
        float trained_time = 0.0f;
    };

    struct CompletedSkill {
        std::string skill_id;
        int level = 0;
        float completed_at = 0.0f;
    };

    std::string player_id;

    std::vector<QueuedSkill> queue;
    int max_queue_size = 50;

    std::vector<CompletedSkill> completed_skills;

    bool paused = false;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SkillQueueState)
};

/**
 * @brief Ship fitting validation state for CPU, power grid, and slot constraints
 */
class ShipFittingValidationState : public ecs::Component {
public:
    struct FittedModule {
        std::string module_id;
        std::string slot_type; // "high", "mid", "low"
        float cpu_usage = 0.0f;
        float power_grid_usage = 0.0f;
    };

    float max_cpu = 0.0f;
    float max_power_grid = 0.0f;
    int high_slots = 0;
    int mid_slots = 0;
    int low_slots = 0;

    std::vector<FittedModule> fitted_modules;

    float cpu_used = 0.0f;
    float power_grid_used = 0.0f;
    int validation_errors = 0;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ShipFittingValidationState)
};

/**
 * @brief Combat after-action report state with per-engagement tracking
 */
class CombatAfterActionState : public ecs::Component {
public:
    struct Engagement {
        std::string engagement_id;
        std::string target_name;
        double damage_dealt = 0.0;
        double damage_received = 0.0;
        float duration = 0.0f;
        double isc_destroyed = 0.0;
        bool finalized = false;
    };

    struct Casualty {
        std::string ship_name;
        double isc_value = 0.0;
    };

    std::string player_id;

    std::vector<Engagement> engagements;
    int max_engagements = 100;

    std::vector<Casualty> casualties;
    int max_casualties = 50;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CombatAfterActionState)
};

/**
 * @brief Multi-leg transit route with waypoints, travel times, and fuel costs
 */
class TransitPlannerState : public ecs::Component {
public:
    struct Waypoint {
        std::string waypoint_id;
        std::string waypoint_name;
        float travel_time = 0.0f;
        float fuel_cost = 0.0f;
    };

    std::string player_id;

    std::vector<Waypoint> waypoints;
    int max_waypoints = 50;

    int current_waypoint_index = 0;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(TransitPlannerState)
};

/**
 * @brief Fleet readiness assessment with member stats and supply tracking
 */
class FleetReadinessState : public ecs::Component {
public:
    struct FleetMember {
        std::string member_id;
        std::string ship_name;
        float dps = 0.0f;
        float ehp = 0.0f;
        float capacitor = 0.0f;
        bool ready = false;
    };

    struct SupplyStatus {
        std::string supply_type;
        float level = 0.0f;
    };

    std::string fleet_id;

    std::vector<FleetMember> members;
    int max_members = 25;

    std::vector<SupplyStatus> supplies;
    int max_supplies = 20;

    float elapsed_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FleetReadinessState)
};

/**
 * @brief Player progression tracking data
 *
 * Stores milestones, ISC totals, combat stats, and gameplay metrics
 * to prove the vertical-slice loop is playable end-to-end.
 */
class PlayerProgressionTracking : public ecs::Component {
public:
    struct Milestone {
        std::string milestone_id;
        std::string description;
        float timestamp = 0.0f;
    };

    std::vector<Milestone> milestones;
    int max_milestones = 50;
    float isc_earned = 0.0f;
    float isc_spent = 0.0f;
    int kills = 0;
    int deaths = 0;
    int docks = 0;
    int jumps = 0;
    float mining_yield = 0.0f;
    float play_time = 0.0f;
    bool active = true;

    COMPONENT_TYPE(PlayerProgressionTracking)
};

/**
 * @brief Server tick performance metrics
 *
 * Tracks per-tick timing, entity counts, and performance budgets to
 * help maintain the 20 Hz target tick rate.  Records min/max/avg tick
 * durations, overtime warnings, and per-system phase timings.
 */
class ServerTickMetrics : public ecs::Component {
public:
    struct PhaseTimer {
        std::string phase_name;
        float duration_ms = 0.0f;
    };

    float target_tick_rate = 20.0f;     // Hz
    float tick_budget_ms = 50.0f;       // 1000/20
    float last_tick_ms = 0.0f;
    float avg_tick_ms = 0.0f;
    float min_tick_ms = 999.0f;
    float max_tick_ms = 0.0f;
    int total_ticks = 0;
    int overtime_ticks = 0;             // ticks exceeding budget
    int entity_count = 0;
    int peak_entity_count = 0;
    float elapsed = 0.0f;
    bool active = true;

    std::vector<PhaseTimer> phase_timers;
    int max_phases = 20;

    COMPONENT_TYPE(ServerTickMetrics)
};

/**
 * @brief Shield harmonics state for frequency tuning and resonance
 *
 * Tracks shield frequency, harmonic profiles, and damage resistance
 * modifiers.  Tuning the frequency toward an incoming damage type
 * increases resistance but leaves other types more vulnerable.
 */
class ShieldHarmonicsState : public ecs::Component {
public:
    struct HarmonicProfile {
        std::string damage_type;      // em, thermal, kinetic, explosive
        float base_resistance = 0.0f; // 0.0-1.0
        float tuned_bonus = 0.0f;     // bonus from frequency alignment
        float effective_resistance = 0.0f;
    };

    float frequency = 50.0f;          // Current shield frequency (0-100)
    float optimal_frequency = 50.0f;  // Optimal frequency for current threat
    float tuning_speed = 5.0f;        // Frequency change per second
    float resonance_strength = 0.0f;  // 0.0-1.0, how well-tuned the shield is
    float max_bonus = 0.3f;           // Maximum resistance bonus from tuning

    std::vector<HarmonicProfile> profiles;
    int max_profiles = 4;

    int total_retunings = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ShieldHarmonicsState)
};

/**
 * @brief Trade route optimization state for multi-hop profit calculation
 *
 * Stores station market snapshots and computes optimal buy/sell routes
 * across multiple hops to maximize profit per unit of cargo volume.
 */
class TradeRouteOptimizerState : public ecs::Component {
public:
    struct MarketEntry {
        std::string station_id;
        std::string commodity;
        float buy_price = 0.0f;   // price to buy at this station
        float sell_price = 0.0f;  // price to sell at this station
        int supply = 0;
        int demand = 0;
    };

    struct TradeHop {
        std::string from_station;
        std::string to_station;
        std::string commodity;
        float buy_price = 0.0f;
        float sell_price = 0.0f;
        float profit_per_unit = 0.0f;
        float travel_time = 0.0f;
    };

    std::vector<MarketEntry> market_data;
    int max_market_entries = 200;

    std::vector<TradeHop> optimized_route;
    int max_hops = 10;

    float total_estimated_profit = 0.0f;
    float total_travel_time = 0.0f;
    int cargo_capacity = 100;
    int routes_calculated = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(TradeRouteOptimizerState)
};

/**
 * @brief Scan probe deployment state for anomaly discovery
 *
 * Manages deployed scan probes that gradually resolve signatures
 * in a star system.  Each probe has a scan radius and strength;
 * overlapping probes increase resolution speed.
 */
class ScanProbeDeploymentState : public ecs::Component {
public:
    struct ScanProbe {
        std::string probe_id;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float scan_radius = 4.0f;  // AU
        float scan_strength = 1.0f;
        float lifetime = 300.0f;   // seconds before auto-recall
        float age = 0.0f;
        bool recalled = false;
    };

    struct Signature {
        std::string sig_id;
        std::string sig_type;       // "anomaly", "wormhole", "data", "relic", "gas"
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float scan_progress = 0.0f; // 0.0-1.0
        bool resolved = false;
    };

    std::vector<ScanProbe> probes;
    int max_probes = 8;

    std::vector<Signature> signatures;
    int max_signatures = 30;

    int total_probes_launched = 0;
    int total_signatures_resolved = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ScanProbeDeploymentState)
};

/**
 * @brief Docking bay allocation state for station capacity management
 *
 * Tracks per-station docking bays, assignments, wait queues, and
 * service timers.  Ships request bays, get assigned when available,
 * and release bays on undock.
 */
class DockingBayAllocationState : public ecs::Component {
public:
    struct DockingBay {
        std::string bay_id;
        std::string assigned_ship;
        std::string bay_size;      // "small", "medium", "large"
        float service_timer = 0.0f;
        bool occupied = false;
    };

    struct QueueEntry {
        std::string ship_id;
        std::string required_size;
        float wait_time = 0.0f;
        int priority = 0;          // higher = more urgent
    };

    std::string station_id;

    std::vector<DockingBay> bays;
    int max_bays = 20;

    std::vector<QueueEntry> wait_queue;
    int max_queue = 50;

    int total_dockings = 0;
    int total_undockings = 0;
    int total_queue_timeouts = 0;
    float avg_wait_time = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(DockingBayAllocationState)
};

/**
 * @brief Fleet warp coordination state
 *
 * Tracks fleet member alignment, readiness, and synchronized warp
 * initiation.  The fleet commander initiates a warp; each member
 * must align and reach readiness before the fleet warps together.
 */
class FleetWarpCoordinatorState : public ecs::Component {
public:
    struct FleetMember {
        std::string ship_id;
        float align_progress = 0.0f;   // 0.0-1.0
        float align_time = 5.0f;       // seconds to align
        bool ready = false;
        bool warping = false;
    };

    std::string fleet_id;
    std::string commander_id;
    std::string destination;

    std::vector<FleetMember> members;
    int max_members = 50;

    bool warp_initiated = false;
    bool warp_active = false;
    float warp_countdown = 0.0f;
    float warp_countdown_duration = 3.0f;
    int total_fleet_warps = 0;
    int total_members_warped = 0;

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FleetWarpCoordinatorState)
};

/**
 * @brief Mining laser state for ore extraction
 *
 * Tracks mining laser activation, extraction rate, target asteroid,
 * cumulative yield, and cycle timing.  Each cycle extracts ore based
 * on laser strength and asteroid composition.
 */
class MiningLaserState : public ecs::Component {
public:
    struct OreYield {
        std::string ore_type;
        float amount = 0.0f;
    };

    std::string target_asteroid;
    std::string laser_type;        // "strip", "deep_core", "ice"
    float mining_strength = 1.0f;  // multiplier
    float cycle_duration = 10.0f;  // seconds per cycle
    float cycle_progress = 0.0f;   // 0.0-1.0
    float range = 15.0f;           // km
    float optimal_range = 10.0f;   // km
    bool mining_active = false;

    std::vector<OreYield> cumulative_yield;
    int max_ore_types = 10;

    float total_ore_mined = 0.0f;
    int total_cycles = 0;
    int failed_cycles = 0;
    float asteroid_remaining = 100.0f;  // percentage

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(MiningLaserState)
};

/**
 * @brief NPC behavior scheduler state
 *
 * Manages NPC daily schedule with patrol, trade, idle, dock, and
 * combat behavior states.  NPCs transition between states based on
 * time-of-day, threat level, and market conditions.
 */
class NPCBehaviorSchedulerState : public ecs::Component {
public:
    enum class Behavior {
        Idle = 0,
        Patrol,
        Trade,
        Mine,
        Dock,
        Combat,
        Flee
    };

    struct ScheduleEntry {
        std::string label;
        int behavior = static_cast<int>(Behavior::Idle);
        float start_hour = 0.0f;    // 0-24 game hours
        float duration_hours = 1.0f;
    };

    std::string npc_id;
    std::string faction;
    int current_behavior = static_cast<int>(Behavior::Idle);
    int previous_behavior = static_cast<int>(Behavior::Idle);

    std::vector<ScheduleEntry> schedule;
    int max_schedule_entries = 24;

    float current_game_hour = 0.0f;  // 0-24
    float threat_level = 0.0f;       // 0-1, triggers combat/flee
    float threat_threshold = 0.5f;   // above this → combat

    int total_transitions = 0;
    int total_combat_entries = 0;
    int total_trade_trips = 0;

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(NPCBehaviorSchedulerState)
};

/**
 * @brief Capacitor warfare state for energy neutralizer/nosferatu
 *
 * Tracks energy drain/transfer between ships.  Neutralizers drain
 * both target and self; nosferatu drain target and transfer to self.
 * Includes drain resistance calculations and warfare statistics.
 */
class CapacitorWarfareState : public ecs::Component {
public:
    struct WarfareModule {
        std::string module_id;
        std::string module_type;     // "neutralizer", "nosferatu"
        std::string target_id;
        float drain_rate = 10.0f;    // GJ/s
        float optimal_range = 10.0f; // km
        float cycle_time = 12.0f;    // seconds
        float cycle_progress = 0.0f;
        bool active_cycling = false;
    };

    std::vector<WarfareModule> modules;
    int max_modules = 8;

    float drain_resistance = 0.0f;    // 0-1, reduces incoming drain
    float total_energy_drained = 0.0f;
    float total_energy_received = 0.0f;
    float total_energy_lost = 0.0f;   // from enemy warfare
    int total_cycles_completed = 0;
    int total_targets_capped = 0;     // targets emptied

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CapacitorWarfareState)
};

class SignatureAnalysisState : public ecs::Component {
public:
    struct SignatureReading {
        std::string sig_id;
        std::string sig_type;
        float signal_strength = 0.0f;
        float scan_resolution = 0.0f;
        bool identified = false;
    };

    std::vector<SignatureReading> readings;
    int max_readings = 50;

    float scan_power = 1.0f;
    float analysis_speed = 1.0f;
    int total_identified = 0;
    int total_scans = 0;

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SignatureAnalysisState)
};

class OverheatManagementState : public ecs::Component {
public:
    struct ModuleHeat {
        std::string module_id;
        float heat_level = 0.0f;
        float heat_generation = 5.0f;
        float max_heat = 100.0f;
        float damage_threshold = 80.0f;
        bool overheated = false;
        bool burned_out = false;
    };

    std::vector<ModuleHeat> modules;
    int max_modules = 16;

    float heat_dissipation_rate = 2.0f;
    float global_heat = 0.0f;
    float max_global_heat = 100.0f;
    int total_overheats = 0;
    int total_burnouts = 0;

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(OverheatManagementState)
};

class SalvageProcessingState : public ecs::Component {
public:
    struct SalvageJob {
        std::string wreck_id;
        std::string material_type;
        float processing_time = 30.0f;
        float progress = 0.0f;
        float yield_amount = 0.0f;
        float success_chance = 0.75f;
        bool completed = false;
        bool successful = false;
    };

    std::vector<SalvageJob> jobs;
    int max_jobs = 10;

    float processing_speed = 1.0f;
    float skill_bonus = 0.0f;
    float total_materials_salvaged = 0.0f;
    int total_jobs_completed = 0;
    int total_jobs_failed = 0;

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SalvageProcessingState)
};

class JumpFatigueTrackerState : public ecs::Component {
public:
    float blue_timer = 0.0f;
    float orange_timer = 0.0f;
    float max_blue_timer = 600.0f;
    float max_orange_timer = 36000.0f;
    float fatigue_multiplier = 1.0f;
    float decay_rate = 1.0f;
    int total_jumps = 0;
    int total_fatigue_penalties = 0;
    bool jump_restricted = false;

    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(JumpFatigueTrackerState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H
