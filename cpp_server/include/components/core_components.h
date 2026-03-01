#ifndef NOVAFORGE_COMPONENTS_CORE_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_CORE_COMPONENTS_H

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
        Mining,
        Hauling
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
    std::string haul_station_id;    // destination station for hauling ore
    
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


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_CORE_COMPONENTS_H
