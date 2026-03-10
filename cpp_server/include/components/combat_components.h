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

/**
 * @brief Tracks bounty collection state for a player entity
 *
 * When an NPC with a bounty is destroyed, the kill is recorded here.
 * The system processes pending kills each tick and awards credits.
 */
class BountyCollection : public ecs::Component {
public:
    struct BountyKill {
        std::string target_id;
        std::string target_type;
        double bounty_amount = 0.0;
        float timestamp = 0.0f;
        bool paid = false;
    };

    std::vector<BountyKill> pending_kills;
    double total_bounties_collected = 0.0;
    int total_kills_claimed = 0;
    int max_pending = 50;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(BountyCollection)
};

// ==================== Bounty Payout System ====================

/**
 * @brief Automatic bounty payout on NPC destruction
 *
 * When an NPC is destroyed, a payout entry is queued.  The system
 * processes the queue each tick, awards ISC to the killer's wallet,
 * and maintains running totals.  Supports payout multipliers from
 * security status or faction standing.
 */
class BountyPayout : public ecs::Component {
public:
    struct PayoutEntry {
        std::string killer_id;
        std::string victim_id;
        std::string victim_type;    // "pirate", "rogue_drone", "sleeper"
        double base_bounty = 0.0;
        double final_payout = 0.0;
        float timestamp = 0.0f;
        bool processed = false;
    };

    std::vector<PayoutEntry> pending_payouts;
    double total_isc_paid = 0.0;
    int total_payouts_processed = 0;
    float payout_multiplier = 1.0f;     // from security status, standings, etc.
    int max_pending = 100;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(BountyPayout)
};

// ==================== Module Overheat ====================

class ModuleOverheat : public ecs::Component {
public:
    struct ModuleSlot {
        std::string module_id;
        float heat_level = 0.0f;         // 0..100
        float heat_per_cycle = 5.0f;     // heat gained per activation cycle
        float dissipation_rate = 2.0f;   // heat lost per second when not overheating
        float damage_threshold = 80.0f;  // heat % at which module takes damage
        float damage_accumulated = 0.0f;
        bool overheating = false;
        bool burned_out = false;
    };

    std::vector<ModuleSlot> modules;
    int max_modules = 8;
    int total_burnouts = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ModuleOverheat)
};

// ==================== System Security Response ====================

/**
 * @brief Star-system security force response to criminal activity
 *
 * Tracks criminal offences reported within a star system.  Each offence
 * raises the response level.  When the response level exceeds a
 * threshold the security force spawns response ships.  Response level
 * decays over time once offences stop.
 */
class SystemSecurityResponse : public ecs::Component {
public:
    enum class SecurityLevel { HighSec, LowSec, NullSec };
    enum class ResponseState { Idle, Alerted, Responding, Engaged };

    struct Offence {
        std::string offender_id;
        std::string offence_type;        // "assault", "theft", "smuggling"
        float severity = 1.0f;           // 0.0–10.0
        float timestamp = 0.0f;
    };

    std::string system_id;
    SecurityLevel security_level = SecurityLevel::HighSec;
    ResponseState state = ResponseState::Idle;
    std::vector<Offence> offences;
    float response_level = 0.0f;          // 0.0–100.0
    float alert_threshold = 20.0f;        // triggers Alerted state
    float respond_threshold = 50.0f;      // triggers Responding state
    float decay_rate = 2.0f;              // per second when no new offences
    int max_offences = 50;
    int response_ships_dispatched = 0;
    int total_responses = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SystemSecurityResponse)
};

// ==================== Combat Loot Drop ====================

/**
 * @brief Loot drop table and pending drop tracking for combat kills
 *
 * Each entity with this component has a configurable drop table of items
 * with drop chances, quantity ranges, and rarity tiers.  When a kill
 * triggers a drop, a pending entry is created and resolved on the next
 * tick using a deterministic roll.
 */
class CombatLootDrop : public ecs::Component {
public:
    struct DropEntry {
        std::string item_id;
        float drop_chance = 0.5f;        // 0.0–1.0
        int min_qty = 1;
        int max_qty = 1;
        std::string rarity = "common";   // common, uncommon, rare, epic
    };

    struct PendingDrop {
        std::string source_id;           // entity that was destroyed
        bool resolved = false;
    };

    std::vector<DropEntry> drop_table;
    std::vector<PendingDrop> pending_drops;
    int max_drop_entries = 20;
    int total_drops_triggered = 0;
    int total_items_dropped = 0;
    std::string last_drop_source;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CombatLootDrop)
};

/**
 * @brief Ship damage resistance profile with type-specific mitigation and hardener stacking
 *
 * Base resistances per damage type (EM, Thermal, Kinetic, Explosive) are
 * augmented by active hardener modules that stack with diminishing returns.
 * Individual resistances are capped at 85%.  Active hardeners consume
 * charge each tick.
 */
class DamageResistanceProfile : public ecs::Component {
public:
    struct Hardener {
        std::string hardener_id;
        std::string damage_type;  // em, thermal, kinetic, explosive
        float bonus = 0.0f;      // 0.0–0.5
        bool is_active = false;
    };

    float base_em = 0.0f;
    float base_thermal = 0.0f;
    float base_kinetic = 0.0f;
    float base_explosive = 0.0f;
    std::vector<Hardener> hardeners;
    int max_hardeners = 8;
    float total_damage_mitigated = 0.0f;
    float charge_consumed = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(DamageResistanceProfile)
};

/**
 * @brief Turret tracking state for rotation and target acquisition
 *
 * Manages turret rotation speed, tracking accuracy, and target
 * lock state. Small fast targets are harder to track, requiring
 * higher tracking speed to maintain accuracy.
 */
class TurretTrackingState : public ecs::Component {
public:
    std::string turret_id;
    std::string target_id;
    float tracking_speed = 1.0f;       // rad/s angular tracking speed
    float optimal_range = 10000.0f;    // meters
    float falloff_range = 5000.0f;     // meters beyond optimal
    float current_angle = 0.0f;        // radians, current turret angle
    float target_angle = 0.0f;         // radians, angle to target
    float angular_velocity = 0.0f;     // target angular velocity in rad/s
    float accuracy = 0.0f;             // 0.0-1.0, computed hit probability
    float damage_multiplier = 0.0f;    // based on tracking vs angular velocity
    int total_shots_fired = 0;
    int total_hits = 0;
    float elapsed = 0.0f;
    bool active = true;
    bool locked = false;               // true when turret is locked on target

    float hitRate() const {
        return total_shots_fired > 0
            ? static_cast<float>(total_hits) / static_cast<float>(total_shots_fired)
            : 0.0f;
    }

    COMPONENT_TYPE(TurretTrackingState)
};

/**
 * @brief Dynamic encounter balance state for difficulty and reward scaling
 *
 * Tracks player progression metrics and adjusts encounter difficulty multiplier
 * and reward scaling so the game stays challenging but fair for solo and co-op.
 * The system recalculates modifiers each tick based on fleet size, average ship
 * class, and total kill count.
 */
class EncounterBalanceState : public ecs::Component {
public:
    std::string encounter_id;
    float difficulty_multiplier = 1.0f;  // 0.5 (easy) – 3.0 (brutal)
    float reward_multiplier = 1.0f;      // scales ISC/loot payout
    int player_count = 1;                // fleet size for scaling
    int avg_ship_class = 1;              // 1=frigate … 5=capital
    int total_kills = 0;                 // cumulative kills for progression
    float base_difficulty = 1.0f;        // designer-authored baseline
    float base_reward = 100.0f;          // baseline ISC reward
    float elapsed = 0.0f;
    bool active = true;
    int recalc_count = 0;               // how many times balance was recalculated

    COMPONENT_TYPE(EncounterBalanceState)
};

/**
 * @brief Configurable weighted loot table for encounters, missions, and exploration
 *
 * Maintains a list of possible loot entries each with a weight, rarity tier,
 * and quantity range.  The system selects drops via weighted random and tracks
 * how many rolls / drops have occurred.
 */
class LootTableState : public ecs::Component {
public:
    struct LootEntry {
        std::string item_id;
        std::string rarity;      // "common","uncommon","rare","epic","legendary"
        float weight = 1.0f;     // relative drop weight
        int min_quantity = 1;
        int max_quantity = 1;
    };

    std::string table_id;
    std::vector<LootEntry> entries;
    int max_entries = 50;
    int total_rolls = 0;
    int total_drops = 0;
    float luck_modifier = 1.0f;  // multiplied into rare weights
    float elapsed = 0.0f;
    bool active = true;

    float totalWeight() const {
        float w = 0.0f;
        for (const auto& e : entries) w += e.weight;
        return w;
    }

    COMPONENT_TYPE(LootTableState)
};

// ==================== Gate Gun State ====================

/**
 * @brief Automated sentry gun placed at stargates in high/low-sec systems
 *
 * Gate guns automatically engage entities flagged as criminal or hostile
 * within their engagement range.  They cycle fire at a fixed interval
 * and deal configurable damage.  Damage falloff applies beyond optimal
 * range.
 */
class GateGunState : public ecs::Component {
public:
    struct Target {
        std::string entity_id;
        float threat_level = 0.0f;   // 0.0–10.0
        float time_engaged = 0.0f;
        bool is_criminal = false;
    };

    std::string gate_id;
    float engagement_range = 150.0f;   // km
    float optimal_range = 100.0f;      // km – full damage
    float falloff_range = 50.0f;       // km beyond optimal before 0 dmg
    float damage_per_cycle = 500.0f;
    float cycle_time = 3.0f;           // seconds per shot
    float cycle_progress = 0.0f;
    int max_targets = 3;
    std::vector<Target> targets;
    int total_shots_fired = 0;
    float total_damage_dealt = 0.0f;
    int total_kills = 0;
    float elapsed = 0.0f;
    bool active = true;
    bool online = true;               // can be disabled (e.g. during incursion)

    COMPONENT_TYPE(GateGunState)
};

/**
 * @brief NPC aggression switching state
 *
 * Tracks threat values for each attacker and determines which target
 * the NPC should prioritise.  Threat is accumulated from DPS, EWAR,
 * and proximity bonuses.  The highest-threat target becomes the
 * primary, with optional hysteresis to avoid constant switching.
 */
class AggressionSwitchingState : public ecs::Component {
public:
    struct ThreatEntry {
        std::string entity_id;
        float accumulated_threat = 0.0f;   // running total
        float dps_contribution = 0.0f;     // recent DPS from this source
        float ewar_contribution = 0.0f;    // electronic warfare threat
        float proximity_bonus = 0.0f;      // close-range bonus
        float time_on_list = 0.0f;
    };

    std::string current_target_id;
    std::vector<ThreatEntry> threat_table;
    float switch_threshold = 1.2f;         // new target must exceed current by 20%
    float threat_decay_rate = 0.1f;        // per-second decay on all entries
    float evaluation_interval = 2.0f;      // seconds between re-evaluations
    float evaluation_timer = 0.0f;
    int total_switches = 0;
    float elapsed = 0.0f;
    bool active = true;
    bool locked = false;                   // if true, ignore re-evaluation

    COMPONENT_TYPE(AggressionSwitchingState)
};

// ==================== AEGIS NPC Spawn ====================

/**
 * @brief AEGIS security force spawning state
 *
 * Controls the spawning of AEGIS NPC response fleets in high-security
 * space when criminal activity is detected.  Response time scales with
 * the system security level: 1.0 is near-instant, 0.5 is ~20 seconds.
 * AEGIS forces despawn after the threat is neutralised or expires.
 */
class AegisSpawnState : public ecs::Component {
public:
    enum class SpawnPhase { Idle, Dispatching, Warping, Engaged, Withdrawing };

    struct DispatchedSquad {
        std::string squad_id;
        std::string target_id;         // criminal being pursued
        int ship_count = 3;
        float dps_per_ship = 200.0f;
        float warp_eta = 0.0f;         // seconds until arrival
        float engagement_time = 0.0f;  // time on-grid
        float max_engagement = 120.0f; // withdraw after this
        SpawnPhase phase = SpawnPhase::Dispatching;
    };

    std::string system_id;
    float security_level = 1.0f;           // 0.0–1.0
    float base_response_time = 6.0f;       // seconds at sec 1.0
    float response_time_scale = 30.0f;     // added seconds at sec 0.5
    float dispatch_timer = 0.0f;
    float dispatch_interval = 2.0f;        // re-check interval
    std::vector<DispatchedSquad> squads;
    int max_squads = 5;
    int total_dispatched = 0;
    int total_kills = 0;
    float elapsed = 0.0f;
    bool active = true;

    float responseTimeForSecurity() const {
        float t = base_response_time + response_time_scale * (1.0f - security_level);
        return (std::max)(t, base_response_time);
    }

    COMPONENT_TYPE(AegisSpawnState)
};

// ==================== Drifter AI ====================

/**
 * @brief Drifter / Triglavian NPC AI state
 *
 * Drifters use advanced beam weapons that ramp up damage over time
 * when attacking the same target.  They can deploy area denial fields
 * and call reinforcements from a superweapon carrier when provoked
 * beyond a configurable threshold.
 */
class DrifterAIState : public ecs::Component {
public:
    enum class DrifterRole { Cruiser, Battleship, Carrier, Response };
    enum class ThreatLevel { Passive, Aggressive, Berserk };

    struct DrifterUnit {
        std::string unit_id;
        DrifterRole role = DrifterRole::Cruiser;
        float hp = 2000.0f;
        float max_hp = 2000.0f;
        float base_dps = 200.0f;
        float ramp_multiplier = 1.0f;       // grows while firing at same target
        float ramp_rate = 0.1f;              // multiplier increase per second
        float max_ramp = 3.0f;              // cap on ramp_multiplier
        std::string current_target;
        bool alive = true;
    };

    std::string site_id;
    ThreatLevel threat_level = ThreatLevel::Passive;
    std::vector<DrifterUnit> units;
    int max_units = 8;
    float provocation_threshold = 3000.0f;   // total damage to trigger Berserk
    float damage_taken = 0.0f;
    float area_denial_radius = 15000.0f;     // meters
    bool area_denial_active = false;
    float area_denial_dps = 50.0f;           // damage per second to ships in field
    float area_denial_timer = 0.0f;
    float area_denial_duration = 30.0f;      // seconds
    int reinforcement_wave = 0;
    int max_reinforcement_waves = 2;
    float reinforcement_cooldown = 45.0f;
    float reinforcement_timer = 0.0f;
    int total_kills = 0;
    int total_losses = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(DrifterAIState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_COMBAT_COMPONENTS_H
