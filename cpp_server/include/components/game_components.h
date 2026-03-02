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

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H
