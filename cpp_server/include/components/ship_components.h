#ifndef NOVAFORGE_COMPONENTS_SHIP_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_SHIP_COMPONENTS_H

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
// ==================== Station Hangar ====================

/**
 * @brief Station hangar — personal or corporation ship storage
 *
 * Hangars come in different types depending on ownership:
 *   - Personal: owned by a player, built or leased at a station
 *   - Corporation: shared corp hangar, upgradable
 *   - Leased: rented from a station, charged per day
 *
 * Ships that are not capital-class dock inside the hangar.
 * Capital-class and larger ships tether to external docking arms instead.
 *
 * The hangar also serves as the FPS spawn point when a player is docked.
 */
class StationHangar : public ecs::Component {
public:
    enum class HangarType {
        Personal,       // Player-owned hangar
        Corporation,    // Corp shared hangar (upgradable)
        Leased          // Rented from station (daily charge)
    };

    enum class UpgradeLevel {
        Basic = 0,      // 1 small ship slot
        Standard = 1,   // 2 small/medium ship slots
        Advanced = 2,   // 3 slots including large ships
        Premium = 3     // 4 slots with maintenance bay
    };

    std::string hangar_id;
    std::string station_id;               // Station this hangar belongs to
    std::string owner_id;                 // Player or corp who owns/leases
    HangarType type = HangarType::Leased;
    UpgradeLevel upgrade_level = UpgradeLevel::Basic;

    double daily_rental_cost = 5000.0;    // Credits per day (leased only)
    double accumulated_rental = 0.0;      // Unpaid rental balance
    float days_rented = 0.0f;             // Total days rented

    int max_ship_slots = 1;               // Depends on upgrade level
    int occupied_ship_slots = 0;
    std::vector<std::string> stored_ship_ids;  // Ships parked in hangar

    float capacity_volume = 500.0f;       // m³ storage for items
    float used_volume = 0.0f;

    // FPS spawn offset inside the hangar (relative to station)
    float spawn_x = 0.0f;
    float spawn_y = 0.0f;
    float spawn_z = 0.0f;

    static int maxSlotsForLevel(UpgradeLevel lvl) {
        switch (lvl) {
            case UpgradeLevel::Basic:    return 1;
            case UpgradeLevel::Standard: return 2;
            case UpgradeLevel::Advanced: return 3;
            case UpgradeLevel::Premium:  return 4;
            default: return 1;
        }
    }

    static float capacityForLevel(UpgradeLevel lvl) {
        switch (lvl) {
            case UpgradeLevel::Basic:    return 500.0f;
            case UpgradeLevel::Standard: return 1500.0f;
            case UpgradeLevel::Advanced: return 5000.0f;
            case UpgradeLevel::Premium:  return 15000.0f;
            default: return 500.0f;
        }
    }

    bool hasRoom() const { return occupied_ship_slots < max_ship_slots; }
    bool isLeased() const { return type == HangarType::Leased; }

    COMPONENT_TYPE(StationHangar)
};

// ==================== Tether Docking Arm ====================

/**
 * @brief Tether docking arm — external arm for capital+ ships
 *
 * Capital-class ships and larger (Carrier, Dreadnought, Titan) are too
 * large to fit inside a station hangar. Instead they tether to an
 * external docking arm that extends from the station.
 *
 * While tethered:
 *   - The ship remains visible in space, attached to the arm
 *   - Crew can transition between the ship interior and the station
 *   - The ship receives station shield protection while tethered
 *   - Undocking requires retracting the arm first
 */
class TetherDockingArm : public ecs::Component {
public:
    enum class ArmState {
        Retracted,      // Arm stowed, no ship attached
        Extending,      // Arm reaching out to lock ship
        Locked,         // Ship secured to arm
        Retracting      // Arm pulling back after undock
    };

    std::string arm_id;
    std::string station_id;
    std::string tethered_ship_id;         // Ship currently tethered
    ArmState state = ArmState::Retracted;

    float extend_progress = 0.0f;         // 0.0 = retracted, 1.0 = fully extended
    float extend_speed = 0.5f;            // Progress per second
    float arm_length = 500.0f;            // Metres
    float min_ship_mass = 50000.0f;       // Only capital+ ships (mass threshold)

    bool station_shield_active = true;    // Whether tethered ship is protected
    bool crew_transfer_enabled = false;   // True only when fully locked

    bool isOccupied() const { return !tethered_ship_id.empty(); }
    bool isFullyExtended() const { return extend_progress >= 1.0f; }
    bool isFullyRetracted() const { return extend_progress <= 0.0f; }

    COMPONENT_TYPE(TetherDockingArm)
};

class CloakingState : public ecs::Component {
public:
    enum class CloakType { Prototype, Improved, CovertOps };
    enum class CloakPhase { Inactive, Activating, Cloaked, Deactivating };

    CloakType cloak_type = CloakType::Prototype;
    CloakPhase phase = CloakPhase::Inactive;
    float activation_time = 5.0f;       // seconds to fully cloak
    float deactivation_time = 2.0f;     // seconds to fully decloak
    float phase_timer = 0.0f;           // progress in current phase
    float fuel_per_second = 1.0f;       // capacitor drain while cloaked
    float speed_penalty = 0.75f;        // max speed multiplier while cloaked (0.75 = 25% slower)
    float targeting_delay = 10.0f;      // seconds of targeting lockout after decloak
    float targeting_lockout_remaining = 0.0f;
    float proximity_decloak_range = 2000.0f;  // meters - entities within range decloak ship
    bool can_warp_while_cloaked = false;      // only CovertOps cloaks allow warp
    int decloak_count = 0;                     // times decloaked (for tracking)
    std::string owner_id;

    COMPONENT_TYPE(CloakingState)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_SHIP_COMPONENTS_H
