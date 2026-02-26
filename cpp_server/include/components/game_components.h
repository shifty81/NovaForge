#ifndef NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H

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
 * @brief Position and orientation in 3D space
 */
class Position : public ecs::Component {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float rotation = 0.0f;  // radians
    
    COMPONENT_TYPE(Position)
};

/**
 * @brief Velocity and movement
 */
class Velocity : public ecs::Component {
public:
    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
    float angular_velocity = 0.0f;
    float max_speed = 100.0f;
    
    COMPONENT_TYPE(Velocity)
};

/**
 * @brief Health pools (shield, armor, hull) like Astralis ONLINE
 */
class Health : public ecs::Component {
public:
    // Health pools
    float hull_hp = 100.0f;
    float hull_max = 100.0f;
    float armor_hp = 100.0f;
    float armor_max = 100.0f;
    float shield_hp = 100.0f;
    float shield_max = 100.0f;
    float shield_recharge_rate = 1.0f;  // HP per second
    
    // Hull resistances (0.0 = no resist, 0.5 = 50% resist)
    float hull_em_resist = 0.0f;
    float hull_thermal_resist = 0.0f;
    float hull_kinetic_resist = 0.0f;
    float hull_explosive_resist = 0.0f;
    
    // Armor resistances
    float armor_em_resist = 0.0f;
    float armor_thermal_resist = 0.0f;
    float armor_kinetic_resist = 0.0f;
    float armor_explosive_resist = 0.0f;
    
    // Shield resistances
    float shield_em_resist = 0.0f;
    float shield_thermal_resist = 0.0f;
    float shield_kinetic_resist = 0.0f;
    float shield_explosive_resist = 0.0f;
    
    bool isAlive() const {
        return hull_hp > 0.0f;
    }
    
    COMPONENT_TYPE(Health)
};

/**
 * @brief Energy capacitor like Astralis ONLINE
 */
class Capacitor : public ecs::Component {
public:
    float capacitor = 100.0f;
    float capacitor_max = 100.0f;
    float recharge_rate = 2.0f;  // GJ per second
    
    COMPONENT_TYPE(Capacitor)
};

/**
 * @brief Ship-specific data
 */
class Ship : public ecs::Component {
public:
    std::string ship_type = "Frigate";
    std::string ship_class = "Frigate";
    std::string ship_name = "Fang";
    std::string race = "Keldari";
    
    // Fitting resources
    float cpu = 0.0f;
    float cpu_max = 100.0f;
    float powergrid = 0.0f;
    float powergrid_max = 50.0f;
    
    // Signature and targeting
    float signature_radius = 35.0f;  // meters
    float scan_resolution = 400.0f;  // mm
    int max_locked_targets = 3;
    float max_targeting_range = 20000.0f;  // meters
    
    // Warp parameters (per ship class, from warp_mechanics.json)
    float warp_speed_au = 5.0f;      // AU/s (frigate default)
    float align_time = 2.5f;          // seconds to align for warp (frigate default)
    int warp_strength = 1;             // warp core strength (points needed to disrupt)
    
    COMPONENT_TYPE(Ship)
};

/**
 * @brief Targeting information
 */
class Target : public ecs::Component {
public:
    std::vector<std::string> locked_targets;  // entity IDs
    std::map<std::string, float> locking_targets;  // entity_id: progress (0-1)
    
    COMPONENT_TYPE(Target)
};

/**
 * @brief Weapon system
 */
class Weapon : public ecs::Component {
public:
    std::string weapon_type = "Projectile";  // Projectile, Energy, Missile, Hybrid
    std::string damage_type = "kinetic";  // em, thermal, kinetic, explosive
    float damage = 10.0f;
    float optimal_range = 5000.0f;  // meters
    float falloff_range = 2500.0f;  // meters
    float tracking_speed = 0.5f;  // radians per second
    float rate_of_fire = 3.0f;  // seconds between shots
    float cooldown = 0.0f;  // current cooldown timer
    float capacitor_cost = 5.0f;  // GJ per shot
    std::string ammo_type = "EMP";
    int ammo_count = 100;
    
    COMPONENT_TYPE(Weapon)
};

/**
 * @brief AI behavior for NPCs
 */
class AI : public ecs::Component {
public:
    enum class Behavior {
        Aggressive,
        Defensive,
        Passive,
        Flee
    };
    
    enum class State {
        Idle,
        Approaching,
        Orbiting,
        Fleeing,
        Attacking,
        Mining
    };
    
    /**
     * @brief Target selection strategy for AI combat
     */
    enum class TargetSelection {
        Closest,     // Target nearest entity (default)
        LowestHP,    // Target entity with lowest total HP fraction
        HighestThreat // Target entity dealing the most damage to us
    };
    
    Behavior behavior = Behavior::Aggressive;
    State state = State::Idle;
    std::string target_entity_id;
    float orbit_distance = 1000.0f;  // preferred orbit distance (0 = auto from ship class)
    float awareness_range = 50000.0f;  // meters
    float flee_threshold = 0.25f;  // flee when total HP (shield+armor+hull) below this fraction of max
    TargetSelection target_selection = TargetSelection::Closest;  // how to pick targets
    bool use_dynamic_orbit = false;  // if true, orbit_distance set from ship class
    float engagement_range = 0.0f;  // 0 = derive from weapon optimal+falloff
    
    COMPONENT_TYPE(AI)
};

/**
 * @brief Player-controlled entity
 */
class Player : public ecs::Component {
public:
    std::string player_id;
    std::string character_name = "Pilot";
    double credits = 1000000.0;  // Starting Credits
    std::string corporation = "NPC Corp";
    
    COMPONENT_TYPE(Player)
};

/**
 * @brief Faction affiliation
 */
class Faction : public ecs::Component {
public:
    std::string faction_name = "Neutral";  // Veyren, Aurelian, Solari, Keldari, Venom Syndicate, etc.
    std::map<std::string, float> standings;  // faction_name: standing (-10 to +10)
    
    COMPONENT_TYPE(Faction)
};

/**
 * @brief Personal standings with entities, corporations, and factions
 * 
 * Tracks relationships on a -10 to +10 scale:
 * - Personal standings: Individual player/NPC relationships
 * - Corporation standings: Corporation-level relationships
 * - Faction standings: Faction-wide relationships
 * 
 * Standings affect:
 * - Agent access (requires positive corp/faction standing)
 * - NPC aggression (negative standings cause attacks)
 * - Market taxes and broker fees
 * - Mission rewards and LP gains
 */
class Standings : public ecs::Component {
public:
    // Personal standings with individual entities (player_id or npc_id)
    std::map<std::string, float> personal_standings;
    
    // Corporation standings (corporation_name: standing)
    std::map<std::string, float> corporation_standings;
    
    // Faction standings (faction_name: standing) 
    // Duplicated from Faction component for convenience
    std::map<std::string, float> faction_standings;
    
    /**
     * @brief Get standing with an entity
     * Checks personal, then corporation, then faction standings in order
     * @return Standing value from -10 to +10, or 0 if no standing exists
     */
    float getStandingWith(const std::string& entity_id, 
                         const std::string& entity_corp = "",
                         const std::string& entity_faction = "") const {
        // Check personal standing first (highest priority)
        auto personal_it = personal_standings.find(entity_id);
        if (personal_it != personal_standings.end()) {
            return personal_it->second;
        }
        
        // Check corporation standing
        if (!entity_corp.empty()) {
            auto corp_it = corporation_standings.find(entity_corp);
            if (corp_it != corporation_standings.end()) {
                return corp_it->second;
            }
        }
        
        // Check faction standing (lowest priority)
        if (!entity_faction.empty()) {
            auto faction_it = faction_standings.find(entity_faction);
            if (faction_it != faction_standings.end()) {
                return faction_it->second;
            }
        }
        
        return 0.0f;  // Neutral if no standing found
    }
    
    /**
     * @brief Modify standing with clamping to valid range
     * @param standing_map The map to modify (personal, corp, or faction)
     * @param key The entity/corp/faction identifier
     * @param change Amount to change (can be negative)
     */
    static void modifyStanding(std::map<std::string, float>& standing_map,
                              const std::string& key,
                              float change) {
        float current = 0.0f;
        auto it = standing_map.find(key);
        if (it != standing_map.end()) {
            current = it->second;
        }
        
        // Apply change and clamp to -10 to +10
        float new_standing = current + change;
        new_standing = (std::max)(-10.0f, (std::min)(10.0f, new_standing));
        standing_map[key] = new_standing;
    }
    
    COMPONENT_TYPE(Standings)
};

/**
 * @brief Solar system properties for wormhole space
 *
 * Tracks the wormhole class (C1-C6), active system-wide effects,
 * and whether dormant NPCs have already been spawned.
 */
class SolarSystem : public ecs::Component {
public:
    std::string system_id;
    std::string system_name;
    int wormhole_class = 0;               // 0 = k-space, 1-6 = wormhole class
    std::string effect_name;              // e.g. "magnetar", "pulsar", "" for none
    bool dormants_spawned = false;
    
    COMPONENT_TYPE(SolarSystem)
};

/**
 * @brief A wormhole connection between two systems
 *
 * Models mass limits, remaining stability, and lifetime so that
 * the WormholeSystem can decay and eventually collapse connections.
 */
class WormholeConnection : public ecs::Component {
public:
    std::string wormhole_id;
    std::string source_system;            // system entity id
    std::string destination_system;       // system entity id
    double max_mass = 500000000.0;        // kg total mass allowed
    double remaining_mass = 500000000.0;  // kg remaining before collapse
    double max_jump_mass = 20000000.0;    // kg max single-ship mass
    float max_lifetime_hours = 24.0f;     // hours until natural collapse
    float elapsed_hours = 0.0f;           // hours elapsed since spawn
    bool collapsed = false;
    
    bool isStable() const {
        return !collapsed && elapsed_hours < max_lifetime_hours && remaining_mass > 0.0;
    }
    
    COMPONENT_TYPE(WormholeConnection)
};

/**
 * @brief Fleet membership for an entity (attached to each fleet member)
 *
 * Tracks which fleet a player belongs to, their role, and any
 * active fleet bonuses being applied.
 */
class FleetMembership : public ecs::Component {
public:
    std::string fleet_id;
    std::string role = "Member";  // "FleetCommander", "WingCommander", "SquadCommander", "Member"
    std::string squad_id;
    std::string wing_id;
    std::map<std::string, float> active_bonuses;  // e.g. "armor_hp_bonus" -> 0.10
    
    COMPONENT_TYPE(FleetMembership)
};

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
 * @brief Skill training and bonuses for a player entity
 *
 * Tracks trained skills, current training queue, and provides
 * methods to compute skill bonuses on ship stats.
 */
class SkillSet : public ecs::Component {
public:
    struct TrainedSkill {
        std::string skill_id;
        std::string name;
        int level = 0;           // 0-5
        int max_level = 5;
        float training_multiplier = 1.0f;
    };

    struct QueueEntry {
        std::string skill_id;
        int target_level = 1;
        float time_remaining = 0.0f;  // seconds remaining
    };

    // All trained skills indexed by skill_id
    std::map<std::string, TrainedSkill> skills;

    // Training queue (FIFO)
    std::vector<QueueEntry> training_queue;

    // Total skill points
    double total_sp = 0.0;

    int getSkillLevel(const std::string& skill_id) const {
        auto it = skills.find(skill_id);
        return (it != skills.end()) ? it->second.level : 0;
    }

    COMPONENT_TYPE(SkillSet)
};

/**
 * @brief Module activation state for fitted modules on a ship
 *
 * Tracks which modules are fitted, their activation state,
 * and cycling timers. Separate from Weapon component which
 * handles NPC auto-fire; this handles player-initiated module use.
 */
class ModuleRack : public ecs::Component {
public:
    struct FittedModule {
        std::string module_id;
        std::string name;
        std::string slot_type;     // "high", "mid", "low"
        int slot_index = 0;
        bool active = false;       // currently cycling
        float cycle_time = 5.0f;   // seconds per cycle
        float cycle_progress = 0.0f; // 0-1 progress through current cycle
        float capacitor_cost = 5.0f;
        float cpu_usage = 10.0f;
        float powergrid_usage = 5.0f;

        // Effects applied while active (key: stat_name, value: modifier)
        std::map<std::string, float> effects;
    };

    std::vector<FittedModule> high_slots;
    std::vector<FittedModule> mid_slots;
    std::vector<FittedModule> low_slots;

    COMPONENT_TYPE(ModuleRack)
};

/**
 * @brief Cargo inventory for ships, wrecks, containers
 */
class Inventory : public ecs::Component {
public:
    struct Item {
        std::string item_id;
        std::string name;
        std::string type;        // "weapon", "module", "ammo", "ore", "salvage", "commodity"
        int quantity = 1;
        float volume = 1.0f;     // m3 per unit
    };

    std::vector<Item> items;
    float max_capacity = 400.0f;  // m3 cargo hold

    float usedCapacity() const {
        float total = 0.0f;
        for (const auto& item : items)
            total += item.volume * item.quantity;
        return total;
    }

    float freeCapacity() const {
        return max_capacity - usedCapacity();
    }

    COMPONENT_TYPE(Inventory)
};

/**
 * @brief Loot drop table attached to NPCs
 */
class LootTable : public ecs::Component {
public:
    struct LootEntry {
        std::string item_id;
        std::string name;
        std::string type;
        float drop_chance = 1.0f;  // 0.0-1.0
        int min_quantity = 1;
        int max_quantity = 1;
        float volume = 1.0f;
    };

    std::vector<LootEntry> entries;
    double isk_drop = 0.0;     // Credits bounty

    COMPONENT_TYPE(LootTable)
};

/**
 * @brief Drone bay and deployed drone management
 *
 * Tracks which drones are stored in the drone bay and which are
 * currently deployed in space.  Enforces bandwidth and bay capacity.
 */
class DroneBay : public ecs::Component {
public:
    struct DroneInfo {
        std::string drone_id;
        std::string name;
        std::string type;          // "light_combat_drone", "medium_combat_drone", "mining_drone", "salvage_drone", etc.
        std::string damage_type;   // "em", "thermal", "kinetic", "explosive"
        float damage = 0.0f;
        float rate_of_fire = 3.0f; // seconds between shots
        float cooldown = 0.0f;     // current cooldown timer
        float optimal_range = 5000.0f;
        float hitpoints = 45.0f;
        float current_hp = 45.0f;
        int bandwidth_use = 5;
        float volume = 5.0f;       // m3 per drone
        float mining_yield = 0.0f;  // units of ore per cycle (mining drones)
        float salvage_chance = 0.0f; // probability of successful salvage per cycle (salvage drones)
    };

    std::vector<DroneInfo> stored_drones;    // drones in bay (not deployed)
    std::vector<DroneInfo> deployed_drones;  // drones in space

    float bay_capacity = 25.0f;     // m3 total bay capacity
    int max_bandwidth = 25;         // Mbit/s bandwidth limit

    std::string mining_target_id;   // entity id of deposit for mining drones
    std::string salvage_target_id;  // entity id of wreck for salvage drones

    int usedBandwidth() const {
        int total = 0;
        for (const auto& d : deployed_drones)
            total += d.bandwidth_use;
        return total;
    }

    float usedBayVolume() const {
        float total = 0.0f;
        for (const auto& d : stored_drones)
            total += d.volume;
        for (const auto& d : deployed_drones)
            total += d.volume;
        return total;
    }

    COMPONENT_TYPE(DroneBay)
};

/**
 * @brief Insurance policy on a ship
 */
class InsurancePolicy : public ecs::Component {
public:
    std::string policy_id;
    std::string ship_type;
    std::string tier = "basic";    // "basic", "standard", "platinum"
    float coverage_fraction = 0.5f; // fraction of ship value paid out
    double premium_paid = 0.0;     // Credits paid for policy
    double payout_value = 0.0;     // Credits paid out on loss
    float duration_remaining = -1.0f; // seconds, -1 = permanent
    bool active = true;
    bool claimed = false;

    COMPONENT_TYPE(InsurancePolicy)
};

/**
 * @brief Tracks bounty rewards earned by a player
 */
class BountyLedger : public ecs::Component {
public:
    double total_bounty_earned = 0.0;
    int total_kills = 0;
    
    struct BountyRecord {
        std::string target_id;
        std::string target_name;
        double bounty_amount = 0.0;
        std::string faction;
    };
    
    std::vector<BountyRecord> recent_kills;  // last N kills
    static constexpr int MAX_RECENT = 50;
    
    COMPONENT_TYPE(BountyLedger)
};

/**
 * @brief Market order tracking for stations
 */
class MarketHub : public ecs::Component {
public:
    struct Order {
        std::string order_id;
        std::string item_id;
        std::string item_name;
        std::string owner_id;       // entity that placed the order
        bool is_buy_order = false;   // true = buy, false = sell
        double price_per_unit = 0.0;
        int quantity = 1;
        int quantity_remaining = 1;
        float duration_remaining = -1.0f;  // seconds, -1 = permanent
        bool fulfilled = false;
    };

    std::string station_id;
    std::vector<Order> orders;
    double broker_fee_rate = 0.02;  // 2% broker fee
    double sales_tax_rate = 0.04;   // 4% sales tax

    COMPONENT_TYPE(MarketHub)
};

class Corporation : public ecs::Component {
public:
    std::string corp_id;
    std::string corp_name;
    std::string ticker;
    std::string ceo_id;
    float tax_rate = 0.05f;
    std::vector<std::string> member_ids;
    double corp_wallet = 0.0;

    struct CorpHangarItem {
        std::string item_id;
        std::string name;
        std::string type;
        int quantity = 1;
        float volume = 1.0f;
    };

    std::vector<CorpHangarItem> hangar_items;

    COMPONENT_TYPE(Corporation)
};

class ContractBoard : public ecs::Component {
public:
    struct ContractItem {
        std::string item_id;
        std::string name;
        int quantity = 1;
        float volume = 1.0f;
    };

    struct Contract {
        std::string contract_id;
        std::string issuer_id;
        std::string assignee_id;
        std::string type;            // "item_exchange", "courier", "auction"
        std::string status;          // "outstanding", "in_progress", "completed", "expired", "failed"
        std::vector<ContractItem> items_offered;
        std::vector<ContractItem> items_requested;
        double isk_reward = 0.0;
        double isk_collateral = 0.0;
        float duration_remaining = -1.0f;
        float days_to_complete = 3.0f;
    };

    std::vector<Contract> contracts;

    COMPONENT_TYPE(ContractBoard)
};

/**
 * @brief Planetary Operations colony on a planet
 *
 * Tracks extractors, processors, and storage for PI resources.
 * Each colony has a CPU and powergrid budget from the planet type.
 */
class PlanetaryColony : public ecs::Component {
public:
    std::string colony_id;
    std::string owner_id;       // player entity id
    std::string planet_type;    // "barren", "temperate", "oceanic", "lava", "gas", "ice", "storm", "plasma"
    std::string system_id;

    struct Extractor {
        std::string extractor_id;
        std::string resource_type;  // e.g. "base_metals", "aqueous_liquids"
        float cycle_time = 3600.0f; // seconds per extraction cycle
        float cycle_progress = 0.0f;
        int quantity_per_cycle = 100;
        bool active = true;
        float cpu_usage = 45.0f;
        float powergrid_usage = 550.0f;
    };

    struct Processor {
        std::string processor_id;
        std::string input_type;
        std::string output_type;
        int input_quantity = 40;     // units consumed per cycle
        int output_quantity = 5;     // units produced per cycle
        float cycle_time = 1800.0f;  // seconds per processing cycle
        float cycle_progress = 0.0f;
        bool active = true;
        float cpu_usage = 200.0f;
        float powergrid_usage = 800.0f;
    };

    struct StoredResource {
        std::string resource_type;
        int quantity = 0;
    };

    std::vector<Extractor> extractors;
    std::vector<Processor> processors;
    std::vector<StoredResource> storage;
    float storage_capacity = 10000.0f;  // units

    float cpu_max = 1675.0f;
    float powergrid_max = 6000.0f;

    float usedCpu() const {
        float total = 0.0f;
        for (const auto& e : extractors) total += e.cpu_usage;
        for (const auto& p : processors) total += p.cpu_usage;
        return total;
    }

    float usedPowergrid() const {
        float total = 0.0f;
        for (const auto& e : extractors) total += e.powergrid_usage;
        for (const auto& p : processors) total += p.powergrid_usage;
        return total;
    }

    int totalStored() const {
        int total = 0;
        for (const auto& s : storage) total += s.quantity;
        return total;
    }

    COMPONENT_TYPE(PlanetaryColony)
};

/**
 * @brief Manufacturing facility for blueprint-based production
 *
 * Tracks manufacturing jobs: blueprint, materials, output,
 * time remaining, and job status.
 */
class ManufacturingFacility : public ecs::Component {
public:
    std::string facility_id;
    std::string station_id;
    int max_jobs = 1;  // concurrent job slots

    struct MaterialRequirement {
        std::string material_id;
        int quantity = 0;
    };

    struct ManufacturingJob {
        std::string job_id;
        std::string blueprint_id;
        std::string owner_id;
        std::string output_item_id;
        std::string output_item_name;
        int output_quantity = 1;
        int runs = 1;
        int runs_completed = 0;
        float time_per_run = 3600.0f;   // seconds per run
        float time_remaining = 3600.0f; // current run time remaining
        std::string status;             // "pending", "active", "completed", "cancelled"
        double install_cost = 0.0;
        std::vector<MaterialRequirement> materials;
        float material_efficiency = 0.0f;  // 0-10, reduces material cost
        float time_efficiency = 0.0f;      // 0-20, reduces time
    };

    std::vector<ManufacturingJob> jobs;

    int activeJobCount() const {
        int count = 0;
        for (const auto& j : jobs)
            if (j.status == "active") ++count;
        return count;
    }

    COMPONENT_TYPE(ManufacturingFacility)
};

/**
 * @brief Research laboratory for invention and blueprint research
 *
 * Supports ME/TE research on blueprints and Tech II invention.
 */
class ResearchLab : public ecs::Component {
public:
    std::string lab_id;
    std::string station_id;
    int max_jobs = 1;

    struct ResearchJob {
        std::string job_id;
        std::string blueprint_id;
        std::string owner_id;
        std::string research_type;       // "material_efficiency", "time_efficiency", "invention"
        float time_remaining = 3600.0f;
        float total_time = 3600.0f;
        std::string status;              // "active", "completed", "failed"
        // ME/TE research
        int target_level = 1;            // target ME or TE level
        // Invention
        std::string output_blueprint_id; // T2 blueprint on success
        float success_chance = 0.5f;     // 0.0-1.0
        std::string datacore_1;
        std::string datacore_2;
        double install_cost = 0.0;
    };

    std::vector<ResearchJob> jobs;

    int activeJobCount() const {
        int count = 0;
        for (const auto& j : jobs)
            if (j.status == "active") ++count;
        return count;
    }

    COMPONENT_TYPE(ResearchLab)
};

/**
 * @brief Chat channel for persistent messaging
 *
 * Supports multiple channel types (local, corp, fleet, private)
 * with message history, moderation, and member tracking.
 */
class ChatChannel : public ecs::Component {
public:
    std::string channel_id;
    std::string channel_name;
    std::string channel_type = "local";  // "local", "corp", "fleet", "alliance", "private"
    std::string owner_id;                // channel creator/owner
    std::string motd;                    // message of the day
    int max_members = 0;                 // 0 = unlimited
    bool is_moderated = false;

    struct ChatMessage {
        std::string message_id;
        std::string sender_id;
        std::string sender_name;
        std::string content;
        std::string timestamp;
        bool is_system_message = false;
    };

    struct ChannelMember {
        std::string player_id;
        std::string player_name;
        std::string role = "member";     // "member", "moderator", "operator", "owner"
        bool is_muted = false;
    };

    std::vector<ChatMessage> messages;
    std::vector<ChannelMember> members;
    int max_history = 200;               // max messages to keep

    int memberCount() const { return static_cast<int>(members.size()); }

    COMPONENT_TYPE(ChatChannel)
};

/**
 * @brief Character sheet for player identity and attributes
 *
 * Tracks race, bloodline, ancestry, clone, implants, and
 * character attributes for creation and progression.
 */
class CharacterSheet : public ecs::Component {
public:
    std::string character_id;
    std::string character_name;
    std::string race = "Caldari";           // "Caldari", "Amarr", "Gallente", "Minmatar"
    std::string bloodline;                   // race-specific bloodline
    std::string ancestry;                    // background/origin
    std::string gender = "male";             // "male", "female"
    float date_of_birth = 0.0f;              // simulation time of creation

    // Attributes (base values, modified by implants)
    int intelligence = 20;
    int perception = 20;
    int charisma = 19;
    int willpower = 20;
    int memory = 20;

    // Clone
    std::string clone_grade = "foundry";       // "foundry", "apex"
    std::string clone_location;              // station ID for medical clone
    int clone_jump_cooldown = 0;             // seconds remaining

    // Implants (slots 1-10)
    struct Implant {
        std::string implant_id;
        std::string implant_name;
        int slot = 0;                        // 1-10
        std::string attribute_bonus;         // attribute boosted
        int bonus_amount = 0;
    };
    std::vector<Implant> implants;

    // Security status
    float security_status = 0.0f;            // -10.0 to 10.0

    // Employment history
    struct EmploymentRecord {
        std::string corp_id;
        std::string corp_name;
        float join_date = 0.0f;
        float leave_date = 0.0f;
    };
    std::vector<EmploymentRecord> employment_history;

    int getEffectiveAttribute(const std::string& attr) const {
        int base = 0;
        if (attr == "intelligence") base = intelligence;
        else if (attr == "perception") base = perception;
        else if (attr == "charisma") base = charisma;
        else if (attr == "willpower") base = willpower;
        else if (attr == "memory") base = memory;

        for (const auto& imp : implants) {
            if (imp.attribute_bonus == attr) base += imp.bonus_amount;
        }
        return base;
    }

    COMPONENT_TYPE(CharacterSheet)
};

/**
 * @brief Tournament bracket for competitive PvE events
 *
 * Tracks tournament lifecycle: registration, active rounds,
 * participant scoring, and final results with rewards.
 */
class Tournament : public ecs::Component {
public:
    std::string tournament_id;
    std::string name;
    std::string status = "registration";  // "registration", "active", "completed", "cancelled"

    int max_participants = 16;
    int current_round = 0;
    int total_rounds = 0;
    float round_duration = 600.0f;        // seconds per round
    float round_timer = 0.0f;             // countdown for current round

    double entry_fee = 0.0;
    double prize_pool = 0.0;

    struct Participant {
        std::string player_id;
        std::string player_name;
        int score = 0;
        int kills = 0;
        bool eliminated = false;
    };

    std::vector<Participant> participants;

    struct RoundResult {
        int round_number = 0;
        std::string winner_id;
        int winner_score = 0;
        int participant_count = 0;
    };

    std::vector<RoundResult> round_results;

    COMPONENT_TYPE(Tournament)
};

/**
 * @brief Leaderboard for tracking player rankings and achievements
 *
 * Aggregates player stats across categories (kills, Credits earned,
 * missions completed, etc.) and tracks unlocked achievements.
 */
class Leaderboard : public ecs::Component {
public:
    struct PlayerEntry {
        std::string player_id;
        std::string player_name;
        int total_kills = 0;
        double total_isk_earned = 0.0;
        int missions_completed = 0;
        int tournaments_won = 0;
        double total_bounty = 0.0;
        int ships_destroyed = 0;
        int ships_lost = 0;
        double total_damage_dealt = 0.0;
    };

    struct Achievement {
        std::string achievement_id;
        std::string name;
        std::string description;
        std::string category;             // "combat", "industry", "exploration", "social"
        int requirement = 1;              // threshold to unlock
        std::string stat_key;             // which stat to check against requirement
    };

    struct UnlockedAchievement {
        std::string achievement_id;
        std::string player_id;
        float unlock_time = 0.0f;
    };

    std::string board_id;
    std::vector<PlayerEntry> entries;
    std::vector<Achievement> achievements;
    std::vector<UnlockedAchievement> unlocked;

    COMPONENT_TYPE(Leaderboard)
};

/**
 * @brief Station entity — represents a dockable station in space
 */
class Station : public ecs::Component {
public:
    std::string station_name;
    float docking_range = 2500.0f;       // metres
    float repair_cost_per_hp = 1.0f;     // Credits per HP repaired
    int docked_count = 0;                // number of ships currently docked

    COMPONENT_TYPE(Station)
};

/**
 * @brief Docked state — attached to entities that are inside a station
 */
class Docked : public ecs::Component {
public:
    std::string station_id;              // entity id of the station

    COMPONENT_TYPE(Docked)
};

/**
 * @brief Wreck entity — remains of a destroyed ship
 */
class Wreck : public ecs::Component {
public:
    std::string source_entity_id;        // entity that was destroyed
    float lifetime_remaining = 1800.0f;  // seconds before despawn (default 30 min)
    bool salvaged = false;               // true once a player has salvaged it

    COMPONENT_TYPE(Wreck)
};

/**
 * @brief Personality axes for AI fleet captains
 *
 * Eight axes capturing both behavioural style and deeper psychology.
 * The original four (aggression, sociability, optimism, professionalism)
 * describe observable behaviour; the new four (loyalty, paranoia, ambition,
 * adaptability) drive long-term decision making and relationship dynamics.
 */
class CaptainPersonality : public ecs::Component {
public:
    // Behavioural axes
    float aggression = 0.5f;        // 0=cautious, 1=bold
    float sociability = 0.5f;       // 0=quiet, 1=talkative
    float optimism = 0.5f;          // 0=grim, 1=hopeful
    float professionalism = 0.5f;   // 0=casual, 1=formal

    // Psychological axes (Phase 1)
    float loyalty = 0.5f;           // 0=self-serving, 1=devoted to fleet
    float paranoia = 0.5f;          // 0=trusting, 1=suspicious/cautious
    float ambition = 0.5f;          // 0=content, 1=driven/power-seeking
    float adaptability = 0.5f;      // 0=rigid, 1=flexible

    std::string captain_name;
    std::string faction;            // Solari, Veyren, Aurelian, Keldari

    COMPONENT_TYPE(CaptainPersonality)
};

/**
 * @brief Tracks fleet morale for an entity
 */
class FleetMorale : public ecs::Component {
public:
    float morale_score = 0.0f;              // clamped -100 to +100
    int wins = 0;
    int losses = 0;
    int ships_lost = 0;
    int times_saved_by_player = 0;
    int times_player_saved = 0;
    int missions_together = 0;
    std::string morale_state = "Steady";    // Inspired/Steady/Doubtful/Disengaged

    void updateMoraleScore() {
        morale_score = static_cast<float>(wins) * 1.0f
                     - static_cast<float>(losses) * 1.5f
                     - static_cast<float>(ships_lost) * 2.0f
                     + static_cast<float>(times_saved_by_player) * 1.2f;
        morale_score = std::clamp(morale_score, -100.0f, 100.0f);
        if (morale_score >= 50.0f) {
            morale_state = "Inspired";
        } else if (morale_score >= 0.0f) {
            morale_state = "Steady";
        } else if (morale_score >= -50.0f) {
            morale_state = "Doubtful";
        } else {
            morale_state = "Disengaged";
        }
    }

    COMPONENT_TYPE(FleetMorale)
};

/**
 * @brief Social graph for fleet captains
 */
class CaptainRelationship : public ecs::Component {
public:
    struct Relationship {
        std::string other_captain_id;
        float affinity = 0.0f;  // -100 to +100
    };

    std::vector<Relationship> relationships;

    float getAffinityWith(const std::string& id) const {
        for (const auto& r : relationships) {
            if (r.other_captain_id == id) {
                return r.affinity;
            }
        }
        return 0.0f;
    }

    void modifyAffinity(const std::string& id, float change) {
        for (auto& r : relationships) {
            if (r.other_captain_id == id) {
                r.affinity = std::clamp(r.affinity + change, -100.0f, 100.0f);
                return;
            }
        }
        Relationship rel;
        rel.other_captain_id = id;
        rel.affinity = std::clamp(change, -100.0f, 100.0f);
        relationships.push_back(rel);
    }

    COMPONENT_TYPE(CaptainRelationship)
};

/**
 * @brief Long-term emotional arcs
 */
class EmotionalState : public ecs::Component {
public:
    float confidence = 50.0f;       // 0-100
    float trust_in_player = 50.0f;  // 0-100
    float fatigue = 0.0f;           // 0-100
    float hope = 50.0f;             // 0-100

    COMPONENT_TYPE(EmotionalState)
};

/**
 * @brief Warp phase tracking (for warp anomaly system)
 */
class WarpState : public ecs::Component {
public:
    enum class WarpPhase { None, Align, Entry, Cruise, Event, Exit };

    WarpPhase phase = WarpPhase::None;
    float warp_time = 0.0f;
    float distance_remaining = 0.0f;
    float warp_speed = 3.0f;    // AU/s (initialized from Ship component)
    float mass_norm = 0.0f;     // 0=frigate, 1=capital
    float intensity = 0.0f;     // computed from time + mass
    int warp_disrupt_strength = 0;  // total disruption applied to this entity

    COMPONENT_TYPE(WarpState)
};

/**
 * @brief Chatter state for an entity
 */
class FleetChatterState : public ecs::Component {
public:
    float chatter_cooldown = 0.0f;
    bool is_speaking = false;
    float priority = 0.0f;
    std::string current_activity = "Idle";  // Warp/Mining/Combat/Travel/Idle
    std::string last_line_spoken;
    int lines_spoken_total = 0;

    // Interruptible chatter support (Phase 9)
    float speaking_priority = 0.0f;   // priority of the line currently being spoken
    bool was_interrupted = false;     // true if last speech was cut off

    COMPONENT_TYPE(FleetChatterState)
};

/**
 * @brief Rumors heard/witnessed by a captain
 */
class RumorLog : public ecs::Component {
public:
    struct Rumor {
        std::string rumor_id;
        std::string text;
        float belief_strength = 0.5f;
        bool personally_witnessed = false;
        int times_heard = 0;
    };

    std::vector<Rumor> rumors;

    bool hasRumor(const std::string& id) const {
        for (const auto& r : rumors) {
            if (r.rumor_id == id) {
                return true;
            }
        }
        return false;
    }

    void addRumor(const std::string& id, const std::string& text, bool witnessed) {
        for (auto& r : rumors) {
            if (r.rumor_id == id) {
                r.times_heard++;
                return;
            }
        }
        Rumor rumor;
        rumor.rumor_id = id;
        rumor.text = text;
        rumor.belief_strength = 0.5f;
        rumor.personally_witnessed = witnessed;
        rumor.times_heard = 1;
        rumors.push_back(rumor);
    }

    COMPONENT_TYPE(RumorLog)
};

/**
 * @brief Aggregated fleet cargo
 */
class FleetCargoPool : public ecs::Component {
public:
    uint64_t total_capacity = 0;
    uint64_t used_capacity = 0;
    std::map<std::string, uint64_t> pooled_items;   // item_type -> quantity
    std::vector<std::string> contributor_ship_ids;

    COMPONENT_TYPE(FleetCargoPool)
};

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
 * @brief Mineral deposit — an asteroid or ore site containing minable resources
 *
 * Attached to asteroid belt entities.  Each deposit has a mineral type,
 * a remaining quantity (units), and a yield rate that controls how much
 * ore is extracted per mining cycle.
 */
class MineralDeposit : public ecs::Component {
public:
    std::string mineral_type = "Ferrite";   // ore name
    float quantity_remaining = 10000.0f;     // units of ore left
    float max_quantity = 10000.0f;           // original total
    float yield_rate = 1.0f;                 // multiplier on mining yield
    float volume_per_unit = 0.1f;            // m3 per unit of ore

    bool isDepleted() const { return quantity_remaining <= 0.0f; }

    COMPONENT_TYPE(MineralDeposit)
};

/**
 * @brief Mining laser module — attached to ships that can mine
 *
 * Tracks the mining cycle timer and yield per cycle.  When the cycle
 * completes the MiningSystem transfers ore from the targeted deposit
 * into the ship's Inventory.
 */
class MiningLaser : public ecs::Component {
public:
    float yield_per_cycle = 100.0f;          // base units mined per cycle
    float cycle_time = 60.0f;                // seconds per mining cycle
    float cycle_progress = 0.0f;             // seconds elapsed in current cycle
    bool active = false;                     // currently mining?
    std::string target_deposit_id;           // entity id of the deposit being mined

    COMPONENT_TYPE(MiningLaser)
};

/**
 * @brief Per–solar-system resource tracking
 *
 * Attached to the solar system entity to record total and remaining
 * resources so the server can balance spawn rates and depletion.
 */
class SystemResources : public ecs::Component {
public:
    struct ResourceEntry {
        std::string mineral_type;
        float total_quantity = 0.0f;
        float remaining_quantity = 0.0f;
    };

    std::vector<ResourceEntry> resources;

    float totalRemaining() const {
        float sum = 0.0f;
        for (const auto& r : resources) sum += r.remaining_quantity;
        return sum;
    }

    COMPONENT_TYPE(SystemResources)
};

/**
 * @brief Fleet formation assignment for an entity
 *
 * Stores the formation type the fleet is using and this member's
 * computed offset relative to the fleet commander.
 */
class FleetFormation : public ecs::Component {
public:
    enum class FormationType { None, Arrow, Line, Wedge, Spread, Diamond };

    FormationType formation = FormationType::Arrow;
    int slot_index = 0;           // position within the formation (0 = leader)
    float offset_x = 0.0f;       // metres relative to commander
    float offset_y = 0.0f;
    float offset_z = 0.0f;

    // Relationship-driven spacing modifier (Phase 9)
    // 1.0 = normal spacing, <1.0 = closer (friends), >1.0 = wider (grudges)
    float spacing_modifier = 1.0f;

    COMPONENT_TYPE(FleetFormation)
};

/**
 * @brief Persistent memory for an AI fleet captain
 *
 * Records significant events so the captain can reference them in chatter
 * and factor them into personality-driven decisions.
 */
class CaptainMemory : public ecs::Component {
public:
    struct MemoryEntry {
        std::string event_type;     // "combat_win", "combat_loss", "ship_lost", "saved_by_player", "warp_anomaly"
        std::string context;        // free-form detail (e.g. enemy name)
        float timestamp = 0.0f;     // in-game seconds since session start
        float emotional_weight = 0.0f; // -1=traumatic, +1=uplifting
    };

    std::vector<MemoryEntry> memories;
    int max_memories = 50;          // cap to prevent unbounded growth

    void addMemory(const std::string& event, const std::string& ctx,
                   float time, float weight) {
        if (static_cast<int>(memories.size()) >= max_memories) {
            memories.erase(memories.begin()); // drop oldest
        }
        memories.push_back({event, ctx, time, weight});
    }

    int countByType(const std::string& event_type) const {
        int n = 0;
        for (const auto& m : memories) {
            if (m.event_type == event_type) ++n;
        }
        return n;
    }

    float averageWeight() const {
        if (memories.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& m : memories) sum += m.emotional_weight;
        return sum / static_cast<float>(memories.size());
    }

    COMPONENT_TYPE(CaptainMemory)
};

/**
 * @brief Warp tunnel visual layer configuration (cinematic warp system)
 *
 * Stores per-entity shader layer intensities computed by WarpCinematicSystem.
 * Client reads these to drive the multi-layer warp tunnel overlay.
 */
class WarpTunnelConfig : public ecs::Component {
public:
    // Shader layer intensities (0.0–1.0)
    float radial_distortion = 0.0f;    // Radial distortion layer
    float starfield_bloom   = 0.0f;    // Starfield velocity bloom
    float tunnel_skin       = 0.0f;    // Tunnel skin/noise layer
    float vignette          = 0.0f;    // Edge vignette darkening

    // Composite intensity derived from ship mass + phase
    float composite_intensity = 0.0f;

    COMPONENT_TYPE(WarpTunnelConfig)
};

/**
 * @brief Warp audio profile for adaptive warp sounds
 *
 * Drives three audio channels during warp: engine core (sub-bass),
 * warp field harmonics, and environmental shimmer.
 */
class WarpAudioProfile : public ecs::Component {
public:
    float engine_core_volume  = 0.0f;   // Sub-bass engine drone (0.0–1.0)
    float harmonics_volume    = 0.0f;   // Warp field harmonics (0.0–1.0)
    float shimmer_volume      = 0.0f;   // Environmental shimmer (0.0–1.0)
    float engine_core_pitch   = 1.0f;   // Pitch multiplier for engine core
    float harmonics_pitch     = 1.0f;   // Pitch multiplier for harmonics

    COMPONENT_TYPE(WarpAudioProfile)
};

/**
 * @brief Accessibility settings for warp visual/audio effects
 *
 * Allows players to reduce motion, bass, and blur intensity
 * to accommodate different sensitivities.
 */
class WarpAccessibility : public ecs::Component {
public:
    float motion_intensity = 1.0f;   // Motion effect scale (0.0–1.0, 0=off)
    float bass_intensity   = 1.0f;   // Sub-bass volume scale (0.0–1.0)
    float blur_intensity   = 1.0f;   // Blur/distortion scale (0.0–1.0)
    bool  tunnel_geometry_enabled = true;  // false = star streaks only, no full warp skin

    COMPONENT_TYPE(WarpAccessibility)
};

/**
 * @brief HUD travel mode during warp — softens edges, desaturates, adds padding
 *
 * During warp cruise the HUD transitions into a softer, less intrusive mode:
 * - Edges soften, bright colors desaturate, tactical warnings muted
 * - Safe-area padding pushes HUD inward (32–48 px)
 * - Optional UI flair: animated brackets, glow synced to engine bass, parallax
 */
class WarpHUDTravelMode : public ecs::Component {
public:
    // Soft edge treatment (0=normal, 1=fully softened)
    float edge_softness = 0.0f;

    // Color desaturation (0=normal, 1=fully desaturated)
    float color_desaturation = 0.0f;

    // Warning muting (0=normal, 1=fully muted tactical warnings)
    float warning_mute = 0.0f;

    // Safe-area padding in pixels (0=normal, target 32–48px during cruise)
    float safe_area_padding = 0.0f;

    // HUD scale factor (1.0=normal, 0.95=scaled inward during warp)
    float hud_scale = 1.0f;

    // Optional UI flair (player toggle)
    bool  ui_flair_enabled = false;
    float bracket_animation = 0.0f;     // animated bracket offset (0–1)
    float ui_glow_intensity = 0.0f;     // glow synced to engine bass (0–1)
    float hud_parallax_offset = 0.0f;   // subtle parallax shift (pixels)

    COMPONENT_TYPE(WarpHUDTravelMode)
};

/**
 * @brief Auto-comfort rules for warp visual effects
 *
 * Automatically reduces warp visual intensity when performance drops
 * or ultrawide aspect ratios are detected to prevent discomfort.
 */
class WarpAutoComfort : public ecs::Component {
public:
    float target_fps = 60.0f;              // Desired frame rate
    float current_fps = 60.0f;             // Measured frame rate
    float comfort_reduction = 0.0f;        // Auto-applied reduction (0=full effects, 1=minimum)
    bool  ultrawide_detected = false;      // True if aspect ratio > 2.2
    float max_distortion_ultrawide = 0.5f; // Clamp distortion on ultrawide displays

    COMPONENT_TYPE(WarpAutoComfort)
};

/**
 * @brief Refining facility — converts raw ore into refined minerals
 *
 * Attached to station entities that offer refining services.
 * Efficiency determines the yield: at 1.0 (100%) all ore converts
 * to minerals; at 0.5 only half does.
 *
 * Each ore type maps to one or more mineral outputs via refine_ratios.
 */
class RefiningFacility : public ecs::Component {
public:
    struct RefineRecipe {
        std::string ore_type;           // input ore name (e.g. "Ferrite")
        int ore_units_required = 100;   // units consumed per batch
        struct MineralOutput {
            std::string mineral_type;   // output mineral name (e.g. "Stellium")
            int base_quantity = 333;    // base output per batch at 100% efficiency
        };
        std::vector<MineralOutput> outputs;
    };

    float efficiency = 0.5f;            // 0.0–1.0, refining yield multiplier
    float tax_rate = 0.05f;             // fraction taken as tax
    std::vector<RefineRecipe> recipes;

    COMPONENT_TYPE(RefiningFacility)
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

// ==================== Phase 8: Warp Cinematic Components ====================

class WarpProfile : public ecs::Component {
public:
    float warp_speed = 3.0f;        // AU/s
    float mass_norm = 0.0f;         // 0=frigate, 1=capital
    float intensity = 0.0f;         // composite visual/audio intensity
    float comfort_scale = 1.0f;     // accessibility scaling (0-1)

    COMPONENT_TYPE(WarpProfile)
};

class WarpVisual : public ecs::Component {
public:
    float distortion_strength = 0.0f;   // radial distortion amount
    float tunnel_noise_scale = 1.0f;    // procedural noise skin scale
    float vignette_amount = 0.0f;       // peripheral darkening
    float bloom_strength = 0.0f;        // velocity bloom intensity
    float starfield_speed = 1.0f;       // starfield streak multiplier

    COMPONENT_TYPE(WarpVisual)
};

class WarpEvent : public ecs::Component {
public:
    std::string current_event;          // active anomaly event id (empty = none)
    float event_timer = 0.0f;           // remaining duration of event
    int severity = 0;                   // 0=none, 1=visual, 2=sensory, 3=shear, 4=legendary

    COMPONENT_TYPE(WarpEvent)
};

// ==================== Phase 8: Warp Audio Enhancement Components ====================

class WarpMeditationLayer : public ecs::Component {
public:
    bool active = false;
    float fade_timer = 0.0f;
    float fade_duration = 5.0f;
    float volume = 0.0f;
    float activation_delay = 15.0f;
    float warp_cruise_time = 0.0f;

    COMPONENT_TYPE(WarpMeditationLayer)
};

class WarpAudioProgression : public ecs::Component {
public:
    enum class Phase { Tension, Stabilize, Bloom, Meditative };

    Phase current_phase = Phase::Tension;
    float phase_timer = 0.0f;
    float tension_duration = 3.0f;
    float stabilize_duration = 5.0f;
    float bloom_duration = 4.0f;
    float blend_factor = 0.0f;

    float computeOverallProgression() const {
        float total = tension_duration + stabilize_duration + bloom_duration;
        float elapsed = phase_timer;
        if (elapsed >= total) return 1.0f;
        if (elapsed <= 0.0f) return 0.0f;
        return elapsed / total;
    }

    COMPONENT_TYPE(WarpAudioProgression)
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

// ==================== Procedural Content Generation ====================

/**
 * @brief Procedural capital ship interior data
 * Carries generated deck/room/elevator layout for a capital ship entity.
 */
class ProceduralInterior : public ecs::Component {
public:
    int shipClass = 0;
    int deckCount = 0;
    int roomCount = 0;
    int elevatorCount = 0;
    uint64_t pcgSeed = 0;
    COMPONENT_TYPE(ProceduralInterior)
};

/**
 * @brief Procedural station data
 * Carries module layout and power grid info for a station entity.
 */
class ProceduralStation : public ecs::Component {
public:
    int moduleCount = 0;
    int totalPowerConsumption = 0;
    int totalPowerProduction = 0;
    uint64_t pcgSeed = 0;
    COMPONENT_TYPE(ProceduralStation)
};

/**
 * @brief Salvage field data
 * Marks an entity as a salvageable wreck/debris field with loot nodes.
 */
class SalvageFieldComponent : public ecs::Component {
public:
    int totalNodes = 0;
    int hiddenNodes = 0;
    float totalValue = 0.0f;
    uint64_t pcgSeed = 0;
    COMPONENT_TYPE(SalvageFieldComponent)
};

/**
 * @brief Sector grid data
 * Marks an entity as a procedural sector with asteroid/debris grid.
 */
class SectorGrid : public ecs::Component {
public:
    int gridWidth = 0;
    int gridHeight = 0;
    int gridDepth = 0;
    float cellSize = 0.0f;
    int occupiedCells = 0;
    uint64_t pcgSeed = 0;
    COMPONENT_TYPE(SectorGrid)
};

// ==================== Phase 9: Rumor-to-Questline, Departure, Transfer ====================

class RumorQuestline : public ecs::Component {
public:
    std::string rumor_id;
    std::string questline_id;
    int required_confirmations = 3;
    bool graduated = false;
    std::string quest_description;

    COMPONENT_TYPE(RumorQuestline)
};

class CaptainDepartureState : public ecs::Component {
public:
    enum class DeparturePhase { None, Grumbling, FormalRequest, Departing };

    DeparturePhase phase = DeparturePhase::None;
    float disagreement_score = 0.0f;
    float grumble_threshold = 5.0f;
    float request_threshold = 10.0f;
    float departure_timer = 0.0f;
    float departure_delay = 120.0f;
    bool player_acknowledged = false;

    COMPONENT_TYPE(CaptainDepartureState)
};

class CaptainTransferRequest : public ecs::Component {
public:
    enum class TransferType { BiggerShip, EscortOnly, RoleChange };

    TransferType request_type = TransferType::BiggerShip;
    bool request_pending = false;
    std::string requested_ship_class;
    std::string requested_role;
    float morale_at_request = 0.0f;

    COMPONENT_TYPE(CaptainTransferRequest)
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

// ==================== Phase 11: Fleet History ====================

struct FleetHistoryEntry {
    std::string event_type;
    std::string description;
    float timestamp = 0.0f;
    std::string involved_entity_id;
};

class FleetHistory : public ecs::Component {
public:
    std::vector<FleetHistoryEntry> events;
    int max_events = 100;

    void addEvent(const std::string& type, const std::string& desc,
                  float ts, const std::string& entity_id) {
        FleetHistoryEntry entry;
        entry.event_type = type;
        entry.description = desc;
        entry.timestamp = ts;
        entry.involved_entity_id = entity_id;
        events.push_back(entry);
        while (static_cast<int>(events.size()) > max_events) {
            events.erase(events.begin());
        }
    }

    COMPONENT_TYPE(FleetHistory)
};

// ==================== Phase 11: Fleet-as-Civilization ====================

/**
 * @brief Fleet progression stage tracking
 *
 * Tracks fleet growth through 3 stages:
 *   Early (max 5 ships)  — Player + 4 captains, basic personalities + chatter
 *   Mid   (max 15 ships) — 3 wings × 5, wing commanders, role specialization
 *   End   (max 25 ships) — 5 wings × 5, full doctrine (mining, salvage, logistics, escort, construction)
 */
class FleetProgression : public ecs::Component {
public:
    enum class Stage { Early, Mid, End };

    Stage stage = Stage::Early;
    int max_ships = 5;
    int max_wings = 1;
    int current_ship_count = 1;       // starts with player ship
    int ships_per_wing = 5;

    // Stage thresholds (measured by cumulative fleet experience)
    float fleet_experience = 0.0f;    // accumulated from missions, combat, etc.
    float mid_threshold = 100.0f;     // XP required to unlock Mid stage
    float end_threshold = 500.0f;     // XP required to unlock End stage

    // Role specialization unlocks (Mid stage+)
    bool mining_wing_unlocked = false;
    bool combat_wing_unlocked = false;
    bool logistics_wing_unlocked = false;
    bool salvage_wing_unlocked = false;   // End stage
    bool construction_wing_unlocked = false; // End stage

    void updateStage() {
        if (fleet_experience >= end_threshold) {
            stage = Stage::End;
            max_ships = 25;
            max_wings = 5;
        } else if (fleet_experience >= mid_threshold) {
            stage = Stage::Mid;
            max_ships = 15;
            max_wings = 3;
        } else {
            stage = Stage::Early;
            max_ships = 5;
            max_wings = 1;
        }
    }

    COMPONENT_TYPE(FleetProgression)
};

/**
 * @brief Station deployment component
 *
 * Attached to ships capable of deploying into permanent stations.
 * Tracks deployment state, attached modules, and system upgrade effects.
 */
class StationDeployment : public ecs::Component {
public:
    enum class DeployState { Mobile, Deploying, Deployed };

    DeployState deploy_state = DeployState::Mobile;
    float deploy_timer = 0.0f;          // seconds remaining in deployment
    float deploy_duration = 300.0f;     // 5 minutes to deploy

    // Location (set on deployment)
    std::string system_id;
    float deploy_x = 0.0f;
    float deploy_y = 0.0f;
    float deploy_z = 0.0f;

    // Attached station modules (module_type -> count)
    std::map<std::string, int> attached_modules;
    int max_module_slots = 3;

    // System upgrading effects
    float security_bonus = 0.0f;        // +security to the system
    float economy_bonus = 0.0f;         // +economy to the system
    float resource_bonus = 0.0f;        // +resource availability

    int getTotalAttachedModules() const {
        int total = 0;
        for (const auto& pair : attached_modules)
            total += pair.second;
        return total;
    }

    bool canAttachModule() const {
        return getTotalAttachedModules() < max_module_slots;
    }

    COMPONENT_TYPE(StationDeployment)
};

/**
 * @brief Fleet warp formation state
 *
 * Extends FleetFormation with warp-specific behavior:
 * - Formation type selection based on ship class
 * - Breathing oscillation for organic feel (0.02–0.05 Hz)
 * - Visual interaction data (distortion bending, wake ripples)
 */
class FleetWarpState : public ecs::Component {
public:
    enum class WarpFormationType { TightEchelon, LooseDiamond, WideCapital };

    WarpFormationType warp_formation = WarpFormationType::TightEchelon;
    bool in_fleet_warp = false;

    // Breathing oscillation (organic feel)
    float breathing_frequency = 0.03f;  // Hz (0.02–0.05)
    float breathing_amplitude = 0.1f;   // fraction of spacing
    float breathing_phase = 0.0f;       // current phase (radians)

    // Visual interaction
    float distortion_bend = 0.0f;       // warp distortion bending factor (mass-based)
    float wake_ripple = 0.0f;           // wake ripple intensity

    // Ship class for formation selection
    std::string ship_class = "Frigate";

    float computeBreathingOffset(float elapsed_time) const {
        static constexpr float kTwoPi = 6.2831853f;
        float phase = breathing_phase + elapsed_time * breathing_frequency * kTwoPi;
        return breathing_amplitude * std::sin(phase);
    }

    COMPONENT_TYPE(FleetWarpState)
};

/**
 * @brief Fleet civilization-scale tracking
 *
 * Tracks the metrics required for a fleet to reach civilization status:
 * titan threshold, fleet stability, and persistent history.
 */
class FleetCivilization : public ecs::Component {
public:
    // Titan threshold requirements
    bool has_stable_logistics = false;     // sustained supply chain
    bool has_loyal_captains = false;       // average morale >= "Steady"
    bool has_fleet_history = false;        // >= 20 missions completed together
    bool has_fleet_industry = false;       // fleet-scale manufacturing active

    // Civilization metrics
    int total_missions_completed = 0;
    int total_stations_deployed = 0;
    float fleet_stability_score = 0.0f;   // 0-100 stability index
    float economic_output = 0.0f;         // cumulative fleet economic value

    // Persistent history
    int major_events_count = 0;
    int ships_ever_owned = 0;
    int captains_ever_served = 0;

    bool isCivilizationThresholdMet() const {
        return has_stable_logistics && has_loyal_captains &&
               has_fleet_history && has_fleet_industry;
    }

    COMPONENT_TYPE(FleetCivilization)
};

// ==================== Rig & Equipment System ====================

class RigModule : public ecs::Component {
public:
    enum class ModuleType {
        LifeSupport,
        PowerCore,
        JetpackTank,
        Sensor,
        Shield,
        EnvironFilter,
        ToolMount,
        WeaponMount,
        DroneController,
        ScannerSuite,
        CargoPod,
        BatteryPack,
        SolarPanel
    };

    ModuleType type = ModuleType::LifeSupport;
    int tier = 1;
    float efficiency = 1.0f;
    float durability = 100.0f;
    float max_durability = 100.0f;
    std::string module_name;

    COMPONENT_TYPE(RigModule)
};

class RigLoadout : public ecs::Component {
public:
    int rack_width = 2;
    int rack_height = 2;
    int max_slots() const { return rack_width * rack_height; }
    std::vector<std::string> installed_module_ids;

    float total_oxygen = 0.0f;
    float total_power = 0.0f;
    float total_cargo = 0.0f;
    float total_shield = 0.0f;
    float jetpack_fuel = 0.0f;

    bool canInstallModule() const {
        return static_cast<int>(installed_module_ids.size()) < max_slots();
    }

    COMPONENT_TYPE(RigLoadout)
};

// ==================== Myth & Legend System ====================

class PlayerLegend : public ecs::Component {
public:
    struct LegendEntry {
        std::string event_type;
        std::string description;
        float timestamp;
        std::string system_id;
        int magnitude;
    };

    std::vector<LegendEntry> entries;
    int legend_score = 0;
    std::string title;
    int max_entries = 50;

    void addEntry(const std::string& type, const std::string& desc, float ts,
                  const std::string& sys_id, int mag) {
        LegendEntry entry{type, desc, ts, sys_id, mag};
        entries.push_back(entry);
        legend_score += mag;
        if (static_cast<int>(entries.size()) > max_entries) {
            entries.erase(entries.begin());
        }
    }

    COMPONENT_TYPE(PlayerLegend)
};

class AncientTechModule : public ecs::Component {
public:
    enum class TechState { Broken, Repairing, Repaired, Upgraded };

    TechState state = TechState::Broken;
    std::string tech_type;
    float repair_progress = 0.0f;
    float repair_cost = 100.0f;
    float power_multiplier = 1.5f;
    bool reverse_engineered = false;
    std::string blueprint_id;

    bool isUsable() const { return state == TechState::Repaired || state == TechState::Upgraded; }

    COMPONENT_TYPE(AncientTechModule)
};

class DockingPort : public ecs::Component {
public:
    enum class PortType { Airlock, DockingRing, HangarBay, RoverBay };

    PortType type = PortType::Airlock;
    bool is_extended = false;
    bool is_pressurized = true;
    std::string docked_entity_id;
    float max_ship_mass = 0.0f;

    bool isOccupied() const { return !docked_entity_id.empty(); }

    COMPONENT_TYPE(DockingPort)
};

// ==================== Survival Module ====================

class SurvivalNeeds : public ecs::Component {
public:
    float oxygen = 100.0f;
    float hunger = 0.0f;
    float fatigue = 0.0f;
    float oxygen_drain_rate = 0.5f;
    float hunger_rate = 0.1f;
    float fatigue_rate = 0.05f;

    bool isAlive() const { return oxygen > 0.0f; }
    bool isStarving() const { return hunger >= 80.0f; }
    bool isExhausted() const { return fatigue >= 80.0f; }

    COMPONENT_TYPE(SurvivalNeeds)
};

class Fabricator : public ecs::Component {
public:
    bool is_active = false;
    std::string current_recipe;
    float progress = 0.0f;
    float craft_speed = 1.0f;
    std::vector<std::string> known_recipes;
    int max_queue = 5;

    COMPONENT_TYPE(Fabricator)
};

// ==================== Menu & Game Flow ====================

class MenuState : public ecs::Component {
public:
    enum class Screen {
        TitleScreen,
        NewGame,
        LoadGame,
        ModMenu,
        MultiplayerMenu,
        CharacterCreation,
        ShipSelection,
        HangarSpawn,
        FleetCommand,
        InGame,
        PauseMenu
    };

    Screen current_screen = Screen::TitleScreen;
    Screen previous_screen = Screen::TitleScreen;
    float transition_timer = 0.0f;
    bool transition_active = false;

    COMPONENT_TYPE(MenuState)
};

class MultiplayerSession : public ecs::Component {
public:
    enum class Role { None, Host, Client };

    Role role = Role::None;
    std::string host_address;
    int port = 7777;
    int max_players = 20;
    int connected_players = 0;
    bool mod_validation_passed = false;
    uint64_t world_seed = 0;

    COMPONENT_TYPE(MultiplayerSession)
};

// ==================== NPC Crew Simulation ====================

class CrewMember : public ecs::Component {
public:
    enum class CrewRole {
        Engineer, Pilot, Gunner, Medic,
        Scientist, Miner, Cook, Security
    };
    enum class Activity {
        Idle, Working, Walking, Resting, Eating, Repairing, Manning
    };

    CrewRole role = CrewRole::Engineer;
    Activity current_activity = Activity::Idle;
    std::string assigned_room_id;
    std::string current_room_id;
    float skill_level = 1.0f;
    float morale = 50.0f;
    float efficiency_bonus = 0.0f;

    COMPONENT_TYPE(CrewMember)
};

class ShipCrew : public ecs::Component {
public:
    int max_crew = 10;
    int current_crew = 0;
    std::vector<std::string> crew_member_ids;
    float overall_efficiency = 1.0f;
    float morale_average = 50.0f;

    COMPONENT_TYPE(ShipCrew)
};

// ==================== Salvage Exploration ====================

class SalvageSite : public ecs::Component {
public:
    enum class SiteType { ShipWreck, DerelictStation, Ruins, DebrisField, AncientSite };

    SiteType type = SiteType::ShipWreck;
    int total_loot_nodes = 0;
    int discovered_nodes = 0;
    int looted_nodes = 0;
    float scan_difficulty = 0.5f;
    bool has_hostiles = false;
    int hostile_count = 0;
    bool has_ancient_tech = false;

    int trinket_count = 0;
    bool has_rare_bobblehead = false;

    COMPONENT_TYPE(SalvageSite)
};

class SalvageTool : public ecs::Component {
public:
    enum class ToolType { Cutter, GravGun, Scanner, RepairTool };

    ToolType type = ToolType::Cutter;
    float efficiency = 1.0f;
    float power_usage = 5.0f;
    float durability = 100.0f;
    float max_durability = 100.0f;
    int tier = 1;

    COMPONENT_TYPE(SalvageTool)
};

// ==================== Interior-Exterior Coupling ====================

class InteriorExteriorLink : public ecs::Component {
public:
    struct ExteriorEffect {
        std::string module_type;
        float hull_deformation = 0.0f;
        bool visible_on_exterior = true;
        float scale = 1.0f;
    };

    std::vector<ExteriorEffect> effects;
    float total_hull_deformation = 0.0f;
    int visible_module_count = 0;

    void addEffect(const std::string& type, float deform, bool visible, float scale) {
        effects.push_back({type, deform, visible, scale});
        if (visible) visible_module_count++;
        total_hull_deformation += deform;
    }

    void clearEffects() {
        effects.clear();
        total_hull_deformation = 0.0f;
        visible_module_count = 0;
    }

    COMPONENT_TYPE(InteriorExteriorLink)
};

// ==================== Race & Lore ====================

class RaceInfo : public ecs::Component {
public:
    enum class RaceName { TerranDescendant, SynthBorn, PureAlien, HybridEvolutionary };

    RaceName race = RaceName::TerranDescendant;

    float learning_rate = 1.0f;
    float diplomacy_modifier = 0.0f;
    float automation_bonus = 0.0f;
    float environmental_resilience = 1.0f;
    float mutation_rate = 0.0f;

    std::string preferred_tech;
    float faction_standing_modifier = 0.0f;

    static void applyRaceDefaults(RaceInfo& info) {
        switch (info.race) {
            case RaceName::TerranDescendant:
                info.learning_rate = 1.2f;
                info.diplomacy_modifier = 0.15f;
                info.preferred_tech = "balanced";
                break;
            case RaceName::SynthBorn:
                info.automation_bonus = 0.25f;
                info.environmental_resilience = 0.8f;
                info.preferred_tech = "drone";
                break;
            case RaceName::PureAlien:
                info.environmental_resilience = 1.3f;
                info.preferred_tech = "exotic";
                break;
            case RaceName::HybridEvolutionary:
                info.learning_rate = 1.1f;
                info.environmental_resilience = 1.1f;
                info.mutation_rate = 0.05f;
                info.preferred_tech = "hybrid";
                break;
        }
    }

    COMPONENT_TYPE(RaceInfo)
};

class LoreEntry : public ecs::Component {
public:
    struct LogEntry {
        std::string title;
        std::string content;
        float discovery_timestamp;
        std::string source;
    };

    std::vector<LogEntry> discovered_lore;
    int max_entries = 100;

    void addLore(const std::string& title, const std::string& content,
                 float ts, const std::string& source) {
        LogEntry entry{title, content, ts, source};
        discovered_lore.push_back(entry);
        if (static_cast<int>(discovered_lore.size()) > max_entries) {
            discovered_lore.erase(discovered_lore.begin());
        }
    }

    int getLoreCount() const { return static_cast<int>(discovered_lore.size()); }

    COMPONENT_TYPE(LoreEntry)
};

// ==================== Enhanced Market ====================

class MarketOrder : public ecs::Component {
public:
    enum class OrderType { Buy, Sell };

    OrderType type = OrderType::Buy;
    std::string item_type;
    int quantity = 0;
    int quantity_remaining = 0;
    float price_per_unit = 0.0f;
    std::string region_id;
    std::string station_id;
    std::string owner_id;
    float expiry_time = 86400.0f;
    float elapsed_time = 0.0f;
    bool is_npc_order = false;
    bool is_filled = false;

    COMPONENT_TYPE(MarketOrder)
};

class AIFleetDispatch : public ecs::Component {
public:
    enum class DispatchType { Mining, Hauling, Production };

    DispatchType type = DispatchType::Mining;
    std::string target_system_id;
    std::string order_id;
    bool dispatched = false;
    float estimated_completion = 0.0f;
    float elapsed = 0.0f;
    int fleet_size = 1;

    bool isComplete() const { return elapsed >= estimated_completion; }

    COMPONENT_TYPE(AIFleetDispatch)
};

// ==================== Living Universe: AI Economic Actors ====================

/**
 * @brief Makes NPCs real economic participants with wallets, ship ownership,
 *        and permanent death.
 *
 * NPCs are no longer disposable spawns — they own ships, earn and spend Credits,
 * and permanently die if they cannot afford a replacement hull.
 */
class AIEconomicActor : public ecs::Component {
public:
    std::string owned_ship_type;        // e.g. "Rifter", "Caracal"
    double ship_value = 0.0;            // estimated hull Credits value
    double repair_cost_ratio = 0.15;    // fraction of ship_value for repairs
    bool is_destroyed = false;          // ship currently destroyed
    bool permanently_dead = false;      // cannot afford replacement — removed from sim
    int destruction_count = 0;          // total times destroyed
    int replacement_count = 0;          // total times replaced ship
    float time_alive = 0.0f;           // seconds since last respawn / spawn

    bool canAffordReplacement(double wallet) const {
        return wallet >= ship_value;
    }

    COMPONENT_TYPE(AIEconomicActor)
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

class Alliance : public ecs::Component {
public:
    std::string alliance_id;
    std::string alliance_name;
    std::string ticker;
    std::string executor_corp_id;  // The corp that leads the alliance
    std::vector<std::string> member_corp_ids;
    double alliance_wallet = 0.0;
    float tax_rate = 0.0f;
    std::string founded_date;
    bool is_open = false;  // Open for new applications
    int max_corps = 100;
    COMPONENT_TYPE(Alliance)
};

class Sovereignty : public ecs::Component {
public:
    std::string system_id;        // Star system this sovereignty covers
    std::string owner_id;         // Alliance or corp entity that owns it
    std::string system_name;
    float control_level = 0.0f;   // 0.0 to 1.0 (contested to full control)
    float vulnerability_timer = 0.0f; // hours until vulnerable
    bool is_contested = false;
    float strategic_index = 0.0f; // 0.0 to 5.0 based on activity
    float military_index = 0.0f;  // 0.0 to 5.0 based on NPC kills
    float industrial_index = 0.0f;// 0.0 to 5.0 based on mining/production
    std::string infrastructure_hub_id;
    int upgrade_level = 0;        // 0-5
    COMPONENT_TYPE(Sovereignty)
};

class WarDeclaration : public ecs::Component {
public:
    std::string war_id;
    std::string aggressor_id;     // Corp or alliance that declared war
    std::string defender_id;      // Corp or alliance being attacked
    std::string status = "pending"; // "pending", "active", "mutual", "surrendered", "retracted", "finished"
    float duration_hours = 168.0f; // Default 1 week
    float elapsed_hours = 0.0f;
    double war_cost = 100000000.0; // Credits cost to declare war (100M)
    int aggressor_kills = 0;
    int defender_kills = 0;
    double aggressor_isk_destroyed = 0.0;
    double defender_isk_destroyed = 0.0;
    bool is_mutual = false;
    COMPONENT_TYPE(WarDeclaration)
};

// ==================== Phase 15: Convoy Ambush AI ====================

/**
 * @brief Represents a trade route between two systems with associated cargo info.
 */
class ConvoyRoute : public ecs::Component {
public:
    std::string route_id;
    std::string origin_system;
    std::string destination_system;
    std::string cargo_type;
    double cargo_value = 0.0;
    float security_level = 0.5f;    // 0.0 (lawless) to 1.0 (high-sec)
    int active_convoys = 0;          // currently travelling this route
    float pirate_interest = 0.0f;   // 0.0 to 1.0, grows with cargo value

    COMPONENT_TYPE(ConvoyRoute)
};

/**
 * @brief Represents a planned or active pirate ambush on a convoy route.
 */
class ConvoyAmbush : public ecs::Component {
public:
    enum class AmbushState { Planned, Active, Successful, Failed, Dispersed };

    std::string ambush_id;
    std::string pirate_entity_id;
    std::string route_id;
    AmbushState state = AmbushState::Planned;
    float cooldown_remaining = 0.0f;
    double loot_value = 0.0;         // Credits value captured
    int ships_attacked = 0;

    static std::string stateToString(AmbushState s) {
        switch (s) {
            case AmbushState::Planned:     return "planned";
            case AmbushState::Active:      return "active";
            case AmbushState::Successful:  return "successful";
            case AmbushState::Failed:      return "failed";
            case AmbushState::Dispersed:   return "dispersed";
            default:                       return "unknown";
        }
    }

    COMPONENT_TYPE(ConvoyAmbush)
};

// ==================== Phase 15: NPC Dialogue (Legend References) ====================

/**
 * @brief NPC dialogue component that records observed player legend events
 *        and generates contextual remarks about legendary players.
 */
class NPCDialogue : public ecs::Component {
public:
    struct ObservedLegend {
        std::string player_id;
        std::string event_type;
        float timestamp = 0.0f;
    };

    std::vector<ObservedLegend> observed_legends;
    std::vector<std::string> generated_lines;
    int max_lines = 20;

    void observeLegend(const std::string& player_id, const std::string& event_type, float ts) {
        observed_legends.push_back({player_id, event_type, ts});
    }

    void addLine(const std::string& line) {
        generated_lines.push_back(line);
        if (static_cast<int>(generated_lines.size()) > max_lines) {
            generated_lines.erase(generated_lines.begin());
        }
    }

    int getLineCount() const { return static_cast<int>(generated_lines.size()); }
    int getObservedCount() const { return static_cast<int>(observed_legends.size()); }

    COMPONENT_TYPE(NPCDialogue)
};

// ==================== Phase 15: Station Monuments ====================

/**
 * @brief Represents a monument or statue erected in a station for a legendary player.
 */
class StationMonument : public ecs::Component {
public:
    enum class MonumentType { Plaque, Bust, Statue, HeroicStatue, MythicShrine };

    std::string station_id;
    std::string player_id;
    std::string player_name;
    MonumentType type = MonumentType::Plaque;
    int legend_score_at_creation = 0;
    float creation_timestamp = 0.0f;
    std::string inscription;

    static MonumentType scoreToType(int score) {
        if (score >= 500) return MonumentType::MythicShrine;
        if (score >= 200) return MonumentType::HeroicStatue;
        if (score >= 100) return MonumentType::Statue;
        if (score >= 50)  return MonumentType::Bust;
        return MonumentType::Plaque;
    }

    static std::string typeToString(MonumentType t) {
        switch (t) {
            case MonumentType::Plaque:        return "Plaque";
            case MonumentType::Bust:          return "Bust";
            case MonumentType::Statue:        return "Statue";
            case MonumentType::HeroicStatue:  return "HeroicStatue";
            case MonumentType::MythicShrine:  return "MythicShrine";
            default:                          return "Unknown";
        }
    }

    COMPONENT_TYPE(StationMonument)
};

// ==================== Living Universe: Information Propagation ====================

/**
 * @brief Tracks rumors about player actions in a star system.
 * Rumors propagate to neighboring systems over time and decay if unconfirmed.
 */
class InformationPropagation : public ecs::Component {
public:
    struct Rumor {
        std::string rumor_id;
        std::string player_id;
        std::string action_type;  // "combat", "mining", "trade", "exploration", "piracy"
        std::string origin_system;
        float belief_strength = 1.0f;  // 0.0 to 1.0
        float age = 0.0f;              // seconds since creation
        int hops = 0;                  // how many systems this has propagated through
        bool personally_witnessed = false;
    };

    std::vector<Rumor> rumors;
    std::vector<std::string> neighbor_system_ids;  // systems this can propagate to
    float propagation_interval = 30.0f;     // seconds between propagation attempts
    float propagation_timer = 0.0f;
    float decay_rate = 0.01f;               // belief decay per second
    float max_rumor_age = 300.0f;           // rumors older than this are removed
    int max_rumors = 50;
    int max_hops = 5;                       // max propagation distance

    void addRumor(const std::string& rumor_id, const std::string& player_id,
                  const std::string& action_type, const std::string& origin_system,
                  bool witnessed = true) {
        // Don't add duplicate rumors
        for (auto& r : rumors) {
            if (r.rumor_id == rumor_id) {
                if (witnessed) r.belief_strength = (std::min)(r.belief_strength + 0.3f, 1.0f);
                return;
            }
        }
        Rumor rumor;
        rumor.rumor_id = rumor_id;
        rumor.player_id = player_id;
        rumor.action_type = action_type;
        rumor.origin_system = origin_system;
        rumor.belief_strength = witnessed ? 1.0f : 0.5f;
        rumor.personally_witnessed = witnessed;
        rumors.push_back(rumor);
        if (static_cast<int>(rumors.size()) > max_rumors) {
            rumors.erase(rumors.begin());
        }
    }

    int getRumorCount() const { return static_cast<int>(rumors.size()); }

    COMPONENT_TYPE(InformationPropagation)
};

// ==================== Phase 13: Crew Activity AI ====================

/**
 * @brief Tracks crew member activity state and room assignment.
 * Crew AI transitions between activities based on ship state and needs.
 */
class CrewActivity : public ecs::Component {
public:
    enum class Activity { Idle, Working, Walking, Resting, Eating, Repairing, Manning };

    std::string crew_member_id;
    std::string assigned_room_id;
    Activity current_activity = Activity::Idle;
    float activity_timer = 0.0f;        // time in current activity
    float activity_duration = 60.0f;    // how long to stay in activity
    float fatigue = 0.0f;               // 0.0 to 1.0
    float hunger = 0.0f;                // 0.0 to 1.0
    bool ship_damaged = false;          // triggers repair activity
    bool station_manned = false;        // at workstation

    static std::string activityToString(Activity a) {
        switch (a) {
            case Activity::Idle:      return "Idle";
            case Activity::Working:   return "Working";
            case Activity::Walking:   return "Walking";
            case Activity::Resting:   return "Resting";
            case Activity::Eating:    return "Eating";
            case Activity::Repairing: return "Repairing";
            case Activity::Manning:   return "Manning";
            default:                  return "Unknown";
        }
    }

    COMPONENT_TYPE(CrewActivity)
};

// ==================== Living Universe: Visual Cues ====================

class VisualCue : public ecs::Component {
public:
    // Lockdown visual state
    bool lockdown_active = false;
    float lockdown_intensity = 0.0f;       // 0.0 to 1.0

    // Traffic density visualization
    float traffic_density = 0.0f;          // 0.0 (empty) to 1.0 (congested)
    int traffic_ship_count = 0;

    // Threat visualization
    float threat_glow = 0.0f;              // red glow intensity for dangerous systems

    // Economic state visualization
    float prosperity_indicator = 0.5f;     // 0.0 (depressed) to 1.0 (booming)

    // Pirate presence
    float pirate_warning = 0.0f;           // 0.0 (safe) to 1.0 (infested)

    // Resource availability visualization
    float resource_highlight = 0.5f;       // 0.0 (depleted) to 1.0 (rich)

    // Faction influence coloring
    std::string dominant_faction;
    float faction_influence_strength = 0.0f;

    COMPONENT_TYPE(VisualCue)
};

// ==================== Living Universe: Supply/Demand Economy ====================

class SupplyDemand : public ecs::Component {
public:
    struct CommodityState {
        std::string commodity_id;
        float supply = 100.0f;        // current supply units
        float demand = 100.0f;        // current demand units
        float base_price = 100.0f;    // base price per unit
        float current_price = 100.0f; // computed price
        float supply_rate = 1.0f;     // units produced per tick by NPCs
        float demand_rate = 1.0f;     // units consumed per tick by NPCs
    };

    std::vector<CommodityState> commodities;
    float price_elasticity = 0.5f;          // how sensitive prices are to supply/demand ratio
    float npc_activity_modifier = 1.0f;     // scales NPC production/consumption rates
    float price_floor_multiplier = 0.2f;    // minimum price = base_price * floor
    float price_ceiling_multiplier = 5.0f;  // maximum price = base_price * ceiling
    float supply_decay_rate = 0.01f;        // natural supply reduction per tick (consumption)
    float demand_drift_rate = 0.005f;       // demand drifts toward baseline per tick

    void addCommodity(const std::string& id, float base_price, float initial_supply, float initial_demand) {
        for (auto& c : commodities) {
            if (c.commodity_id == id) return; // already exists
        }
        CommodityState cs;
        cs.commodity_id = id;
        cs.base_price = base_price;
        cs.current_price = base_price;
        cs.supply = initial_supply;
        cs.demand = initial_demand;
        commodities.push_back(cs);
    }

    CommodityState* getCommodity(const std::string& id) {
        for (auto& c : commodities) {
            if (c.commodity_id == id) return &c;
        }
        return nullptr;
    }

    const CommodityState* getCommodity(const std::string& id) const {
        for (const auto& c : commodities) {
            if (c.commodity_id == id) return &c;
        }
        return nullptr;
    }

    int getCommodityCount() const { return static_cast<int>(commodities.size()); }

    COMPONENT_TYPE(SupplyDemand)
};

// ==================== Phase 15: Black Market & Smuggling ====================

class BlackMarket : public ecs::Component {
public:
    struct Listing {
        std::string item_id;
        std::string seller_id;
        float price = 0.0f;
        int quantity = 0;
        float risk_level = 0.0f;    // 0.0 (safe) to 1.0 (dangerous)
        bool contraband = false;
        float expiry_timer = 0.0f;
        float max_expiry = 600.0f;  // 10 minutes default
    };

    std::vector<Listing> listings;
    float security_level = 0.5f;            // system security (higher = more risky to trade)
    float detection_chance_base = 0.1f;     // base chance of being detected per transaction
    float price_markup = 1.5f;              // black market items cost more
    int max_listings = 20;
    float listing_refresh_timer = 0.0f;
    float listing_refresh_interval = 120.0f;  // new listings every 2 minutes

    void addListing(const std::string& item_id, const std::string& seller_id,
                    float price, int quantity, bool contraband_item, float risk) {
        if (static_cast<int>(listings.size()) >= max_listings) {
            listings.erase(listings.begin());  // remove oldest
        }
        Listing l;
        l.item_id = item_id;
        l.seller_id = seller_id;
        l.price = price * price_markup;
        l.quantity = quantity;
        l.contraband = contraband_item;
        l.risk_level = risk;
        l.max_expiry = 600.0f;
        listings.push_back(l);
    }

    int getListingCount() const { return static_cast<int>(listings.size()); }

    Listing* findListing(const std::string& item_id) {
        for (auto& l : listings) {
            if (l.item_id == item_id) return &l;
        }
        return nullptr;
    }

    const Listing* findListing(const std::string& item_id) const {
        for (const auto& l : listings) {
            if (l.item_id == item_id) return &l;
        }
        return nullptr;
    }

    COMPONENT_TYPE(BlackMarket)
};

// ==================== Price History for Dynamic Price Graphs ====================

/**
 * @brief Tracks historical price data for market items to enable price graph visualization
 * 
 * Stores timestamped price snapshots with sell price, buy price, and volume.
 * Used by PriceHistorySystem to provide trend analysis and historical data.
 */
class PriceHistory : public ecs::Component {
public:
    struct PriceEntry {
        std::string item_id;
        float timestamp = 0.0f;        // game time when recorded
        double sell_price = 0.0;       // lowest sell price at snapshot time
        double buy_price = 0.0;        // highest buy price at snapshot time
        int volume = 0;                // trade volume in this period
        
        PriceEntry() = default;
        PriceEntry(const std::string& id, float ts, double sell, double buy, int vol)
            : item_id(id), timestamp(ts), sell_price(sell), buy_price(buy), volume(vol) {}
    };

    std::string region_id;
    std::map<std::string, std::vector<PriceEntry>> item_history;  // item_id -> history
    int max_entries_per_item = 720;  // 30 days of hourly data

    void addEntry(const std::string& item_id, float timestamp, 
                  double sell_price, double buy_price, int volume) {
        auto& history = item_history[item_id];
        history.push_back(PriceEntry(item_id, timestamp, sell_price, buy_price, volume));
        
        // Trim to max entries
        while (static_cast<int>(history.size()) > max_entries_per_item) {
            history.erase(history.begin());
        }
    }

    std::vector<PriceEntry> getHistory(const std::string& item_id, int max_count = 0) const {
        auto it = item_history.find(item_id);
        if (it == item_history.end()) return {};
        
        const auto& history = it->second;
        if (max_count <= 0 || max_count >= static_cast<int>(history.size())) {
            return history;
        }
        
        // Return last max_count entries
        return std::vector<PriceEntry>(history.end() - max_count, history.end());
    }

    int getEntryCount(const std::string& item_id) const {
        auto it = item_history.find(item_id);
        return (it != item_history.end()) ? static_cast<int>(it->second.size()) : 0;
    }

    COMPONENT_TYPE(PriceHistory)
};

// ==================== Propaganda & False Myths System ====================

/**
 * @brief Network of NPC-propagated myths, rumors, and misinformation
 * 
 * Tracks fabricated stories about players and factions that NPCs spread
 * through dialogue. Myths have credibility scores that decay over time
 * and can be debunked by players.
 */
class PropagandaNetwork : public ecs::Component {
public:
    enum class MythType {
        Heroic,      // Inflated positive reputation
        Villainous,  // False accusations of wrongdoing
        Mysterious,  // Cryptic rumors about hidden activities
        Exaggerated, // Real events blown out of proportion
        Fabricated   // Completely made up stories
    };

    struct MythEntry {
        std::string myth_id;
        std::string subject_id;        // Who/what the myth is about
        std::string source_faction;    // Faction that originated the myth
        MythType type = MythType::Fabricated;
        std::string content;           // The actual myth text
        std::string base_event;        // Real event being distorted (if any)
        float credibility = 1.0f;      // 0 = debunked, 1 = fully believed
        float timestamp = 0.0f;        // When created
        int spread_count = 1;          // How many NPCs have heard it
        bool debunked = false;
        
        MythEntry() = default;
    };

    std::vector<MythEntry> myths;
    float credibility_decay_rate = 0.001f;  // Credibility lost per update
    int max_myths = 100;

    MythEntry* findMyth(const std::string& myth_id) {
        for (auto& m : myths) {
            if (m.myth_id == myth_id) return &m;
        }
        return nullptr;
    }

    const MythEntry* findMyth(const std::string& myth_id) const {
        for (const auto& m : myths) {
            if (m.myth_id == myth_id) return &m;
        }
        return nullptr;
    }

    std::vector<MythEntry> getMythsAbout(const std::string& subject_id, bool include_debunked = false) const {
        std::vector<MythEntry> result;
        for (const auto& m : myths) {
            if (m.subject_id == subject_id && (include_debunked || !m.debunked)) {
                result.push_back(m);
            }
        }
        return result;
    }

    int getActiveMythCount() const {
        int count = 0;
        for (const auto& m : myths) {
            if (!m.debunked && m.credibility > 0.0f) count++;
        }
        return count;
    }

    COMPONENT_TYPE(PropagandaNetwork)
};

// ==================== Rest Station (Bed & Quarters) ====================

/**
 * @brief A rest facility (bed, quarters, etc.) for fatigue recovery
 * 
 * Provides mechanics for characters to rest and recover fatigue.
 * Quality level affects recovery rate.
 */
class RestStation : public ecs::Component {
public:
    enum class StationType {
        Bunk,       // Basic bunk bed (quality 0.5)
        Bed,        // Standard bed (quality 1.0)
        Quarters,   // Private quarters (quality 1.5)
        Luxury      // Luxury cabin (quality 2.0)
    };

    StationType type = StationType::Bed;
    float quality = 1.0f;              // Recovery rate multiplier (0.5 - 2.0)
    std::string occupant_id;           // Entity currently using this station
    bool is_occupied = false;
    float rest_start_time = 0.0f;      // When current rest session started
    float total_rest_time = 0.0f;      // Accumulated rest time this session

    bool isAvailable() const { return !is_occupied; }

    void startRest(const std::string& entity_id, float current_time) {
        occupant_id = entity_id;
        is_occupied = true;
        rest_start_time = current_time;
        total_rest_time = 0.0f;
    }

    void endRest() {
        occupant_id.clear();
        is_occupied = false;
        rest_start_time = 0.0f;
        total_rest_time = 0.0f;
    }

    static float getQualityForType(StationType t) {
        switch (t) {
            case StationType::Bunk: return 0.5f;
            case StationType::Bed: return 1.0f;
            case StationType::Quarters: return 1.5f;
            case StationType::Luxury: return 2.0f;
            default: return 1.0f;
        }
    }

    static std::string getTypeName(StationType t) {
        switch (t) {
            case StationType::Bunk: return "Bunk";
            case StationType::Bed: return "Bed";
            case StationType::Quarters: return "Quarters";
            case StationType::Luxury: return "Luxury Cabin";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(RestStation)
};

/**
 * @brief Tracks an entity's current resting state
 * 
 * Added to entities that are currently resting at a RestStation.
 */
class RestingState : public ecs::Component {
public:
    std::string rest_station_id;   // RestStation entity being used
    float rest_start_time = 0.0f;  // When rest started
    float fatigue_at_start = 0.0f; // Fatigue level when rest began
    bool is_resting = false;

    COMPONENT_TYPE(RestingState)
};

// ==================== Myth Boss Encounters ====================

/**
 * @brief A boss encounter generated from myth/propaganda content
 * 
 * When myths about events, players, or factions reach critical mass,
 * ancient sites manifest as boss encounters reflecting the myth.
 */
class MythBossEncounter : public ecs::Component {
public:
    enum class BossType {
        Guardian,   // From heroic myths - protective ancient entity
        Destroyer,  // From villainous myths - aggressive ancient weapon
        Phantom,    // From mysterious myths - elusive anomaly entity
        Colossus,   // From exaggerated myths - massive ancient construct
        Mirage      // From fabricated myths - illusion-based encounter
    };

    struct LootEntry {
        std::string item_id;
        float drop_chance = 0.5f;
        int quantity = 1;
    };

    std::string encounter_id;
    std::string myth_id;          // Source myth that generated this encounter
    std::string system_id;        // Star system where encounter spawns
    BossType boss_type = BossType::Guardian;
    float difficulty = 1.0f;      // 1.0 = normal, 5.0 = extreme
    float active_time = 0.0f;     // Time since encounter started
    float max_duration = 3600.0f; // Max time before encounter despawns (1 hour)
    bool active = true;
    bool completion_success = false;
    float shield_hp = 1000.0f;
    float armor_hp = 500.0f;
    float hull_hp = 2000.0f;
    std::vector<LootEntry> loot_table;
    int recommended_fleet_size = 3;

    bool isActive() const { return active && active_time < max_duration; }
    bool isExpired() const { return active_time >= max_duration; }

    static std::string getBossTypeName(BossType t) {
        switch (t) {
            case BossType::Guardian: return "Guardian";
            case BossType::Destroyer: return "Destroyer";
            case BossType::Phantom: return "Phantom";
            case BossType::Colossus: return "Colossus";
            case BossType::Mirage: return "Mirage";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(MythBossEncounter)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_GAME_COMPONENTS_H
