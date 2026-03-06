#ifndef NOVAFORGE_COMPONENTS_COMBAT_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_COMBAT_COMPONENTS_H

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
 * @brief Tactical overlay state
 */
class TacticalOverlayState : public ecs::Component {
public:
    bool enabled = false;
    std::vector<float> ring_distances = {5.0f, 10.0f, 20.0f, 30.0f, 50.0f, 100.0f};
    float tool_range = 0.0f;
    std::string tool_type;

    // Shared filter support (Phase 10)
    std::vector<std::string> filter_categories = {"hostile", "friendly", "asteroid", "structure"};
    bool passive_display_only = true;   // no clickable elements, no dragging

    // Entity priority scaling (Phase 10)
    float entity_display_priority = 1.0f;  // higher = more visible at distance

    // Fleet extensions (Stage 4)
    float anchor_ring_radius = 0.0f;       // anchor ring distance (0 = disabled)
    std::string anchor_entity_id;          // entity at ring center
    bool  wing_bands_enabled = false;      // show wing-level band arcs
    std::vector<float> wing_band_offsets;  // per-wing radial offset from anchor

    COMPONENT_TYPE(TacticalOverlayState)
};

/**
 * @brief Damage event tracking for visual feedback
 * 
 * Records recent damage hits so the client can render appropriate
 * visual effects: shield ripple (blue), armor flash (yellow/orange),
 * hull pulse (red + screen shake).
 */
class DamageEvent : public ecs::Component {
public:
    struct HitRecord {
        float damage_amount = 0.0f;
        std::string damage_type;      // em, thermal, kinetic, explosive
        std::string layer_hit;        // shield, armor, hull
        float timestamp = 0.0f;
        bool shield_depleted = false;  // shield reached 0 on this hit
        bool armor_depleted = false;   // armor reached 0 on this hit
        bool hull_critical = false;    // hull below 25% after this hit
    };

    std::vector<HitRecord> recent_hits;
    float last_hit_time = 0.0f;
    float total_damage_taken = 0.0f;

    void addHit(float damage, const std::string& type, const std::string& layer,
                float time, bool shield_dep = false, bool armor_dep = false,
                bool hull_crit = false) {
        HitRecord hit;
        hit.damage_amount = damage;
        hit.damage_type = type;
        hit.layer_hit = layer;
        hit.timestamp = time;
        hit.shield_depleted = shield_dep;
        hit.armor_depleted = armor_dep;
        hit.hull_critical = hull_crit;
        recent_hits.push_back(hit);
        last_hit_time = time;
        total_damage_taken += damage;
    }

    void clearOldHits(float current_time, float max_age = 5.0f) {
        recent_hits.erase(
            std::remove_if(recent_hits.begin(), recent_hits.end(),
                [current_time, max_age](const HitRecord& h) {
                    return (current_time - h.timestamp) > max_age;
                }),
            recent_hits.end());
    }

    COMPONENT_TYPE(DamageEvent)
};
/**
 * @brief Server-side LOD priority hint for large battle optimisation
 *
 * Attached to entities in crowded scenes so the client can allocate
 * rendering budget wisely.  Higher priority values get higher LOD.
 */
class LODPriority : public ecs::Component {
public:
    float priority = 1.0f;        // 0.0 = lowest, 1.0 = normal, 2.0+ = critical
    bool force_visible = false;   // override culling (e.g. player's own ship)
    float impostor_distance = 500.0f; // distance at which to switch to impostor/billboard

    COMPONENT_TYPE(LODPriority)
};
// ==================== Phase 10: Tactical Overlay Components ====================

class TacticalProjection : public ecs::Component {
public:
    float projected_x = 0.0f;          // 2D projected position X
    float projected_y = 0.0f;          // 2D projected position Y
    float vertical_offset = 0.0f;      // height above/below tactical plane
    bool visible = true;                // whether entity appears on overlay

    COMPONENT_TYPE(TacticalProjection)
};
// ==================== Phase 15: Turret AI + Firing Arcs ====================

/**
 * @brief Per-turret state for automated targeting within arc constraints.
 *
 * Attached to ship/station entities with turrets. Each TurretAIState
 * represents one turret's targeting and firing state.
 */
class TurretAIState : public ecs::Component {
public:
    // Turret configuration (from TurretPlacement/TurretGenerator)
    uint32_t turret_index = 0;
    float arc_degrees = 90.0f;         // total firing arc width
    float direction_deg = 0.0f;        // turret facing direction (ship-relative)
    float tracking_speed = 1.0f;       // rad/s — how fast turret can track
    float base_damage = 10.0f;
    float rate_of_fire = 1.0f;         // shots per second
    float optimal_range = 100.0f;      // for future range-based damage falloff

    // Runtime state
    std::string target_entity_id;       // currently targeted entity
    float cooldown_remaining = 0.0f;   // seconds until next shot
    float angular_velocity = 0.0f;     // target's angular velocity (rad/s)
    bool engaged = false;               // currently firing
    int shots_fired = 0;
    float total_damage_dealt = 0.0f;

    COMPONENT_TYPE(TurretAIState)
};


// ==================== Combat Log ====================

/**
 * @brief Combat event recording and engagement analytics
 *
 * Records combat events for damage type analysis, DPS calculation,
 * and engagement outcome tracking. Supports balance tuning by
 * providing per-engagement statistics and damage breakdowns.
 */
class CombatLog : public ecs::Component {
public:
    enum class DamageType { EM, Thermal, Kinetic, Explosive };
    enum class EngagementOutcome { Ongoing, Victory, Defeat, Draw };

    struct CombatEntry {
        std::string attacker_id;
        std::string defender_id;
        DamageType damage_type = DamageType::Kinetic;
        float damage_amount = 0.0f;
        std::string weapon_type;
        bool hit = true;
        float timestamp = 0.0f;
    };

    struct EngagementSummary {
        std::string engagement_id;
        float start_time = 0.0f;
        float duration = 0.0f;
        float total_damage_dealt = 0.0f;
        float total_damage_received = 0.0f;
        int kills = 0;
        int losses = 0;
        EngagementOutcome outcome = EngagementOutcome::Ongoing;
    };

    std::vector<CombatEntry> entries;
    std::vector<EngagementSummary> engagements;
    int max_entries = 100;
    int max_engagements = 20;
    int total_entries_recorded = 0;
    float total_damage_dealt = 0.0f;
    float total_damage_received = 0.0f;
    int total_kills = 0;
    int total_losses = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CombatLog)
};

// ==================== Damage Notification ====================

/**
 * @brief Tracks and queues damage notifications for HUD display
 *
 * Records incoming/outgoing damage events with type, weapon, and crit info.
 * Entries expire after configurable lifetime. Provides DPS calculations.
 */
class DamageNotification : public ecs::Component {
public:
    enum DamageType { EM = 0, Thermal = 1, Kinetic = 2, Explosive = 3 };

    struct DamageEntry {
        std::string source_id;
        float amount = 0.0f;
        int damage_type = 0;
        std::string weapon_name;
        bool is_critical = false;
        float timestamp = 0.0f;
    };

    std::vector<DamageEntry> incoming;
    std::vector<DamageEntry> outgoing;
    float total_damage_taken = 0.0f;
    float total_damage_dealt = 0.0f;
    int hits_taken = 0;
    int hits_dealt = 0;
    int crits_taken = 0;
    int crits_dealt = 0;
    int max_entries = 50;
    float entry_lifetime = 10.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(DamageNotification)
};

// ==================== Damage Application ====================

/**
 * @brief Per-entity damage resistance profile and pending damage queue
 *
 * Stores EM/Thermal/Kinetic/Explosive resistance values per layer
 * (shield/armor/hull) and queues incoming damage for application
 * by DamageApplicationSystem each tick.
 */
class DamageApplication : public ecs::Component {
public:
    enum class DamageType { EM = 0, Thermal = 1, Kinetic = 2, Explosive = 3 };

    struct PendingDamage {
        std::string source_id;
        float raw_amount = 0.0f;
        DamageType type = DamageType::Kinetic;
        float timestamp = 0.0f;
    };

    std::vector<PendingDamage> pending;
    float total_applied = 0.0f;
    float total_mitigated = 0.0f;
    int hits_processed = 0;
    int max_pending = 50;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(DamageApplication)
};

// ==================== Combat Engagement ====================

/**
 * @brief Tracks combat engagement state lifecycle per entity
 *
 * Entities transition: Safe -> Engaging -> InCombat -> Disengaging -> Safe
 * Used to gate warp, docking, and logoff while under fire.
 */
class CombatEngagement : public ecs::Component {
public:
    enum class State { Safe, Engaging, InCombat, Disengaging };

    State state = State::Safe;
    std::string primary_target_id;
    std::vector<std::string> attackers;
    float time_in_state = 0.0f;
    float disengage_timer = 0.0f;
    float disengage_duration = 15.0f;  // seconds before safe after last hit
    float engage_range = 150000.0f;    // meters; auto-engage within this range
    int engagement_count = 0;
    float total_combat_time = 0.0f;
    bool warp_blocked = false;
    bool dock_blocked = false;
    bool active = true;

    COMPONENT_TYPE(CombatEngagement)
};

/**
 * @brief Tracks accumulated combat rewards for an entity
 *
 * When NPCs are destroyed, reward XP and loot credits accumulate here.
 * The system periodically flushes pending rewards to the player's wallet
 * and progression components.
 */
class CombatReward : public ecs::Component {
public:
    struct KillReward {
        std::string target_id;
        std::string target_name;
        float xp_awarded = 0.0f;
        double credits_awarded = 0.0;
        std::string loot_table;
        float timestamp = 0.0f;
        bool flushed = false;
    };

    std::vector<KillReward> pending_rewards;
    double total_credits_awarded = 0.0;
    float total_xp_awarded = 0.0f;
    int total_kills = 0;
    int pending_count = 0;
    float flush_interval = 2.0f;    // seconds between flushes
    float time_since_flush = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CombatReward)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_COMBAT_COMPONENTS_H
