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
    double isc_drop = 0.0;     // Credits bounty

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

class CargoScanState : public ecs::Component {
public:
    enum class ScanPhase { Idle, Scanning, Complete, Failed };
    enum class ContrabandType { None, Narcotics, Weapons, Stolen, Counterfeit, Exotic };

    ScanPhase phase = ScanPhase::Idle;
    float scan_time = 5.0f;              // seconds for a complete scan
    float scan_timer = 0.0f;             // current scan progress
    float scan_range = 5000.0f;          // max scan range in meters
    float detection_chance = 0.8f;       // base chance to detect contraband (0-1)
    std::string target_entity_id;        // entity being scanned
    std::string scanner_owner_id;        // entity that owns this scanner
    int contraband_found = 0;            // contraband items found in last scan
    int total_scans = 0;                 // total scans performed
    int total_contraband_detected = 0;   // lifetime contraband detected
    float fine_per_contraband = 1000.0f; // Credits fine per contraband item
    double total_fines_issued = 0.0;     // total Credits in fines issued
    bool is_customs_scanner = false;     // true for gate/station customs
    std::vector<ContrabandType> detected_types;  // contraband types present on this entity

    static std::string phaseToString(ScanPhase p) {
        switch (p) {
            case ScanPhase::Idle: return "idle";
            case ScanPhase::Scanning: return "scanning";
            case ScanPhase::Complete: return "complete";
            case ScanPhase::Failed: return "failed";
            default: return "unknown";
        }
    }

    static std::string contrabandToString(ContrabandType c) {
        switch (c) {
            case ContrabandType::None: return "none";
            case ContrabandType::Narcotics: return "narcotics";
            case ContrabandType::Weapons: return "weapons";
            case ContrabandType::Stolen: return "stolen";
            case ContrabandType::Counterfeit: return "counterfeit";
            case ContrabandType::Exotic: return "exotic";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(CargoScanState)
};

// ==================== Docking Ring Extension System (Phase 13) ====================

/**
 * @brief Docking ring module for ship-to-ship docking
 *
 * Manages extension/retraction of docking rings with alignment tracking,
 * pressure sealing, and connection management.
 */
class DockingRingExtension : public ecs::Component {
public:
    enum class RingState {
        Retracted,
        Extending,
        Extended,
        Retracting
    };

    enum class ConnectionType {
        ShipToShip,
        ShipToStation,
        Emergency
    };

    RingState state = RingState::Retracted;
    float extension_progress = 0.0f;      // 0.0-1.0
    float extension_speed = 0.5f;          // per second
    float alignment_angle = 0.0f;          // degrees, 0 = perfect
    float alignment_threshold = 5.0f;      // degrees
    bool pressure_sealed = false;
    std::string connected_entity_id;
    ConnectionType connection_type = ConnectionType::ShipToShip;
    bool is_connected = false;
    bool is_powered = true;
    float ring_diameter = 10.0f;           // meters
    float ring_integrity = 1.0f;           // 0.0-1.0
    int total_dockings = 0;

    static std::string stateToString(RingState s) {
        switch (s) {
            case RingState::Retracted: return "retracted";
            case RingState::Extending: return "extending";
            case RingState::Extended: return "extended";
            case RingState::Retracting: return "retracting";
            default: return "unknown";
        }
    }

    static std::string connectionTypeToString(ConnectionType c) {
        switch (c) {
            case ConnectionType::ShipToShip: return "ship_to_ship";
            case ConnectionType::ShipToStation: return "ship_to_station";
            case ConnectionType::Emergency: return "emergency";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(DockingRingExtension)
};

// ==================== Rover Bay Ramp System (Phase 13/14) ====================

/**
 * @brief Belly hangar with folding ramp for rover deployment
 *
 * Manages ramp extension, rover storage/deployment, atmosphere safety checks,
 * and bay pressurization.
 */
class RoverBayRamp : public ecs::Component {
public:
    enum class RampState {
        Closed,
        Opening,
        Open,
        Closing
    };

    enum class AtmosphereType {
        None,
        Breathable,
        Toxic,
        Corrosive
    };

    RampState state = RampState::Closed;
    float ramp_progress = 0.0f;            // 0.0-1.0
    float ramp_speed = 0.3f;               // per second
    int max_rovers = 2;
    std::vector<std::string> stored_rover_ids;
    std::vector<std::string> deployed_rover_ids;
    AtmosphereType external_atmosphere = AtmosphereType::None;
    float external_gravity = 1.0f;         // relative to standard (0.0-2.0)
    bool is_pressurized = true;
    bool is_powered = true;
    int total_deployments = 0;
    float bay_temperature = 20.0f;         // celsius

    static std::string stateToString(RampState s) {
        switch (s) {
            case RampState::Closed: return "closed";
            case RampState::Opening: return "opening";
            case RampState::Open: return "open";
            case RampState::Closing: return "closing";
            default: return "unknown";
        }
    }

    static std::string atmosphereToString(AtmosphereType a) {
        switch (a) {
            case AtmosphereType::None: return "none";
            case AtmosphereType::Breathable: return "breathable";
            case AtmosphereType::Toxic: return "toxic";
            case AtmosphereType::Corrosive: return "corrosive";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(RoverBayRamp)
};

// ==================== Fleet Hangar System (Phase 14) ====================

/**
 * @brief Fleet-scale hangars large enough for full fleet once upgraded
 *
 * Manages tiered hangar bays with ship storage, locking, repair, and
 * power-dependent maintenance costs.
 */
class FleetHangar : public ecs::Component {
public:
    struct StoredShip {
        std::string ship_id;
        std::string ship_class;
        float hull_integrity = 100.0f;
        bool is_locked = false;
    };

    std::string owner_entity_id;
    std::string hangar_name;
    int tier = 1;                           // 1-5
    int max_ship_slots = 5;                 // depends on tier
    std::vector<StoredShip> current_ships;
    double upgrade_cost = 1000.0;           // doubles per tier
    bool is_powered = true;
    float power_consumption = 10.0f;
    float total_cargo_capacity = 1000.0f;
    float used_cargo = 0.0f;
    float maintenance_cost_per_tick = 1.0f;
    double total_maintenance_paid = 0.0;
    int total_ships_docked = 0;

    static int getMaxSlotsForTier(int t) {
        switch (t) {
            case 1: return 5;
            case 2: return 10;
            case 3: return 20;
            case 4: return 35;
            case 5: return 50;
            default: return 5;
        }
    }

    static double getUpgradeCostForTier(int t) {
        return 1000.0 * std::pow(2.0, (std::max)(1, t) - 1);
    }

    static std::string tierToString(int t) {
        switch (t) {
            case 1: return "tier_1";
            case 2: return "tier_2";
            case 3: return "tier_3";
            case 4: return "tier_4";
            case 5: return "tier_5";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(FleetHangar)
};

// ==================== Hangar Environment System (Phase 14) ====================

/**
 * @brief Open hangar in unsafe environment damages unsuited players
 *
 * Tracks atmospheric mixing, toxicity, corrosion, and occupant damage
 * when a hangar is opened in hazardous environments.
 */
class HangarEnvironment : public ecs::Component {
public:
    enum class AtmosphereType {
        None,
        Breathable,
        Toxic,
        Corrosive,
        Extreme
    };

    struct OccupantInfo {
        std::string entity_id;
        bool has_suit = false;
        float suit_rating = 0.0f;       // 0-1, reduces damage taken
    };

    std::string hangar_entity_id;
    bool is_hangar_open = false;
    AtmosphereType atmosphere_type = AtmosphereType::Breathable;
    float external_temperature = 22.0f;
    float external_pressure = 1.0f;
    float internal_temperature = 22.0f;
    float internal_pressure = 1.0f;
    float atmosphere_mix_rate = 0.1f;   // how fast external atmosphere enters when open
    float current_toxicity = 0.0f;      // 0-1
    float current_corrosion = 0.0f;     // 0-1
    float current_heat_damage_rate = 0.0f;
    float damage_per_tick = 10.0f;
    std::vector<OccupantInfo> occupants;
    float total_exposure_time = 0.0f;
    bool is_alarm_active = false;

    static std::string atmosphereTypeToString(AtmosphereType a) {
        switch (a) {
            case AtmosphereType::None: return "none";
            case AtmosphereType::Breathable: return "breathable";
            case AtmosphereType::Toxic: return "toxic";
            case AtmosphereType::Corrosive: return "corrosive";
            case AtmosphereType::Extreme: return "extreme";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(HangarEnvironment)
};

// ==================== Visual Coupling ====================

/**
 * @brief Interior-exterior visual coupling component (Phase 13)
 *
 * Maps interior module types to exterior visual markers on the ship hull.
 * When interior modules are installed, corresponding exterior features
 * become visible (solar panels, ore containers, vents, antennas, etc).
 */
class VisualCoupling : public ecs::Component {
public:
    enum class ExteriorFeature {
        SolarPanel,
        OreContainer,
        Vent,
        Antenna,
        WeaponMount,
        ShieldEmitter,
        EngineBooster,
        CargoRack
    };

    struct CouplingEntry {
        std::string module_id;
        ExteriorFeature feature;
        float scale = 1.0f;
        bool visible = true;
        float x_offset = 0.0f;
        float y_offset = 0.0f;
        float z_offset = 0.0f;
    };

    std::string ship_id;
    std::vector<CouplingEntry> entries;
    int max_entries = 32;
    bool auto_update = true;
    int total_updates = 0;

    const CouplingEntry* findEntry(const std::string& module_id) const {
        for (const auto& e : entries) {
            if (e.module_id == module_id) return &e;
        }
        return nullptr;
    }

    CouplingEntry* findEntry(const std::string& module_id) {
        for (auto& e : entries) {
            if (e.module_id == module_id) return &e;
        }
        return nullptr;
    }

    int visibleCount() const {
        int count = 0;
        for (const auto& e : entries) {
            if (e.visible) count++;
        }
        return count;
    }

    int countByFeature(ExteriorFeature feat) const {
        int count = 0;
        for (const auto& e : entries) {
            if (e.feature == feat) count++;
        }
        return count;
    }

    COMPONENT_TYPE(VisualCoupling)
};

class ShipCapabilityRating : public ecs::Component {
public:
    float combat_score = 0.0f;       // 0.0 - 5.0 stars
    float mining_score = 0.0f;       
    float exploration_score = 0.0f;  
    float cargo_score = 0.0f;        
    float defense_score = 0.0f;      
    float fabrication_score = 0.0f;  
    
    int weapon_count = 0;
    int mining_module_count = 0;
    int scanner_count = 0;
    float cargo_capacity = 0.0f;     // m3
    float total_ehp = 0.0f;         // effective HP (shield+armor+hull)
    int industry_module_count = 0;
    
    float overall_rating = 0.0f;     // average of all 6 categories
    bool needs_recalculation = true;
    bool active = true;
    
    COMPONENT_TYPE(ShipCapabilityRating)
};

class ModuleCascadingFailure : public ecs::Component {
public:
    struct ModuleState {
        std::string module_id;
        std::string module_type;  // "Weapon", "Engine", "Shield", "Power", "Sensor", "Cargo"
        float hp = 100.0f;
        float max_hp = 100.0f;
        bool online = true;
        bool destroyed = false;
        std::vector<std::string> depends_on;  // module_ids this depends on
    };
    
    std::vector<ModuleState> modules;
    int max_modules = 20;
    int total_failures = 0;
    int cascade_events = 0;
    bool active = true;
    
    COMPONENT_TYPE(ModuleCascadingFailure)
};

/**
 * @brief Unified cargo manifest with volume tracking and ore hold separation
 */
class CargoManifest : public ecs::Component {
public:
    struct CargoItem {
        std::string item_id;
        std::string item_name;
        std::string category;     // "ore", "mineral", "module", "ammo", "salvage"
        int quantity = 0;
        double volume_per_unit = 1.0;
    };

    double general_capacity = 400.0;   // m³
    double general_used = 0.0;
    double ore_hold_capacity = 0.0;    // 0 = no ore hold
    double ore_hold_used = 0.0;
    bool active = true;
    std::vector<CargoItem> items;

    COMPONENT_TYPE(CargoManifest)
};

/**
 * @brief Saved ship loadout for persistence and quick-swap
 */
class SavedLoadout : public ecs::Component {
public:
    struct ModuleSlot {
        std::string module_id;
        std::string module_name;
        int slot_index = 0;
        std::string slot_type;   // "high", "mid", "low", "rig"
        bool online = true;
    };

    struct Loadout {
        std::string loadout_id;
        std::string loadout_name;
        std::string ship_class;
        float saved_at = 0.0f;
        std::vector<ModuleSlot> modules;
    };

    int max_loadouts = 10;
    float elapsed = 0.0f;
    bool active = true;
    std::string active_loadout_id;
    std::vector<Loadout> loadouts;

    COMPONENT_TYPE(SavedLoadout)
};

/**
 * @brief Tracks per-entity heat state for module thermal management
 *
 * Weapons and active modules generate heat each cycle.  The system dissipates
 * heat each tick based on the entity's radiator capacity.  At high heat levels
 * penalties apply: accuracy degrades above 75% and modules force-offline at 100%.
 */
class ThermalState : public ecs::Component {
public:
    float current_heat = 0.0f;
    float max_heat = 100.0f;
    float dissipation_rate = 5.0f;      // heat units per second
    float heat_warning_threshold = 0.75f; // fraction of max_heat
    float overheat_threshold = 1.0f;     // fraction of max_heat
    int modules_overheated = 0;
    int total_overheat_events = 0;
    float total_heat_generated = 0.0f;
    float total_heat_dissipated = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ThermalState)
};

/**
 * @brief Tracks runtime CPU and powergrid budget for fitted modules
 *
 * Enforces that active modules do not exceed the ship's available CPU
 * and powergrid.  Modules that would push the budget over limit are
 * rejected.  If a reactor is damaged and total PG drops, excess modules
 * are forced offline.
 */
class ModulePowerGrid : public ecs::Component {
public:
    struct FittedModule {
        std::string module_id;
        std::string module_name;
        float cpu_usage = 0.0f;
        float pg_usage = 0.0f;
        bool online = true;
    };

    std::vector<FittedModule> modules;
    float total_cpu = 100.0f;
    float total_pg = 200.0f;
    float cpu_used = 0.0f;
    float pg_used = 0.0f;
    int modules_forced_offline = 0;
    int total_overload_events = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ModulePowerGrid)
};

// ==================== Ship Repair Cost ====================

/**
 * @brief Tracks cumulative repair costs from combat damage
 *
 * As a ship takes damage, repair cost accumulates based on damage type
 * and layer (shield/armor/hull).  Upon docking, costs can be applied
 * to the player's wallet.  Hull repairs cost more than armor, which
 * costs more than shield.
 */
class ShipRepairCost : public ecs::Component {
public:
    struct DamageRecord {
        std::string source_id;
        float shield_damage = 0.0f;
        float armor_damage = 0.0f;
        float hull_damage = 0.0f;
        float timestamp = 0.0f;
    };

    std::vector<DamageRecord> damage_records;
    float shield_cost_rate = 1.0f;       // ISC per point of shield damage
    float armor_cost_rate = 3.0f;        // ISC per point of armor damage
    float hull_cost_rate = 10.0f;        // ISC per point of hull damage
    double total_repair_cost = 0.0;      // accumulated repair ISC cost
    double total_isc_spent_on_repairs = 0.0;
    float discount_rate = 0.0f;          // 0.0 = no discount, 0.5 = 50% off
    bool docked = false;                 // repair only applies when docked
    int total_repairs_completed = 0;
    int total_damage_events = 0;
    int max_records = 100;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ShipRepairCost)
};

/**
 * @brief Ship insurance policy tracking
 *
 * Players can purchase insurance on their ship.  If the ship is destroyed
 * while the policy is active the insured payout is awarded.  Policies
 * expire after a configurable duration and only one tier may be active
 * at a time.
 */
class InsuranceClaim : public ecs::Component {
public:
    enum class PolicyState { Uninsured, Active, ClaimPending, ClaimPaid, Expired };

    struct PolicyTier {
        std::string tier_name;       // e.g. "Basic", "Standard", "Platinum"
        double premium_cost = 0.0;   // ISC cost to buy
        double payout_amount = 0.0;  // ISC received on destruction
        float duration = 3600.0f;    // seconds the policy lasts
    };

    std::vector<PolicyTier> available_tiers;
    PolicyState state = PolicyState::Uninsured;
    std::string active_tier_name;
    double active_payout = 0.0;
    double active_premium_paid = 0.0;
    float policy_time_remaining = 0.0f;
    std::string ship_id;
    std::string owner_id;
    double total_premiums_paid = 0.0;
    double total_payouts_received = 0.0;
    int total_claims = 0;
    int total_policies_purchased = 0;
    int total_policies_expired = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(InsuranceClaim)
};

/**
 * @brief Ship crew management with hiring, morale and role bonuses
 *
 * Tracks crew members assigned to a ship, their roles, morale levels,
 * and the aggregate bonuses they provide to ship performance.  Low
 * morale reduces efficiency; high morale grants bonus multipliers.
 */
class CrewManagement : public ecs::Component {
public:
    enum class CrewRole { Pilot, Engineer, Gunner, Navigator, Medic, ScienceOfficer };
    enum class MoraleLevel { Mutinous, Low, Normal, High, Exceptional };

    struct CrewMember {
        std::string name;
        CrewRole role = CrewRole::Pilot;
        int skill_level = 1;         // 1-10
        float morale = 0.5f;         // 0.0 = mutinous, 1.0 = exceptional
        float salary_per_hour = 10.0f;
        bool assigned = false;
    };

    std::vector<CrewMember> crew;
    int max_crew = 10;
    float average_morale = 0.5f;
    float efficiency_multiplier = 1.0f;  // derived from morale + skills
    double total_salary_paid = 0.0;
    int total_hired = 0;
    int total_dismissed = 0;
    float salary_timer = 0.0f;
    float salary_interval = 3600.0f;     // pay every hour (game time)
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CrewManagement)
};

/**
 * @brief Ship maintenance and wear tracking
 *
 * Tracks hull and module degradation over time.  Ships accumulate
 * wear from combat, warping, and normal operation.  When wear
 * exceeds threshold levels the ship suffers performance penalties.
 * Repairs are scheduled at stations for an ISC cost.
 */
class ShipMaintenance : public ecs::Component {
public:
    enum class Condition { Pristine, Good, Fair, Poor, Critical };

    struct RepairOrder {
        std::string module_name;
        double cost = 0.0;
        float time_required = 0.0f;
        float time_elapsed = 0.0f;
        bool completed = false;
    };

    std::string ship_id;
    Condition condition = Condition::Pristine;
    float hull_integrity = 1.0f;           // 0.0–1.0
    float wear_rate = 0.001f;              // wear per second while active
    float combat_wear_rate = 0.01f;        // additional wear per second in combat
    float performance_penalty = 0.0f;      // 0.0–1.0, derived from condition
    std::vector<RepairOrder> repair_queue;
    double total_repair_cost = 0.0;
    int total_repairs_completed = 0;
    float elapsed = 0.0f;
    bool in_combat = false;
    bool docked = false;
    bool active = true;

    COMPONENT_TYPE(ShipMaintenance)
};

/**
 * @brief Ship fuel tank and consumption tracking
 *
 * Manages fuel levels for ships.  Warp drives and thrusters consume
 * fuel at different rates.  Ships with empty tanks cannot warp and
 * have reduced thruster output.  Fuel is purchased at stations.
 */
class FuelTank : public ecs::Component {
public:
    enum class FuelType { Standard, HighGrade, Experimental };

    std::string ship_id;
    FuelType fuel_type = FuelType::Standard;
    double current_fuel = 100.0;         // current units
    double max_fuel = 100.0;             // capacity
    double warp_consumption_rate = 5.0;  // units per warp second
    double thrust_consumption_rate = 0.5;// units per thrust second
    double idle_consumption_rate = 0.01; // units per second idling
    bool warping = false;
    bool thrusting = false;
    double total_fuel_consumed = 0.0;
    double total_fuel_purchased = 0.0;
    int refuel_count = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FuelTank)
};

// ==================== Tractor Beam ====================

class TractorBeam : public ecs::Component {
public:
    std::string target_id;
    float range = 20000.0f;          // meters — max pull range
    float pull_speed = 500.0f;       // m/s toward ship
    float current_distance = 0.0f;
    float collection_distance = 50.0f;  // auto-collect threshold
    int items_collected = 0;
    int items_failed = 0;
    bool locked = false;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(TractorBeam)
};

// ==================== Asteroid Mining Laser ====================

/**
 * @brief Active mining laser state — ore extraction and yield tracking
 *
 * Manages a set of mining lasers fitted to a ship.  Each laser has a
 * cycle time, yield per cycle, and an optional crystal that modifies
 * yield.  The update tick advances active cycles and accumulates ore
 * mined into the hold (up to cargo capacity).
 */
class AsteroidMiningLaser : public ecs::Component {
public:
    struct MiningLaser {
        std::string laser_id;
        std::string crystal_id;          // empty = no crystal
        float yield_per_cycle = 10.0f;   // m3 ore per cycle
        float crystal_bonus = 0.0f;      // multiplier bonus (e.g. 0.625 = +62.5%)
        float cycle_time = 60.0f;        // seconds per cycle
        float cycle_progress = 0.0f;     // 0.0–cycle_time
        bool cycling = false;
    };

    std::vector<MiningLaser> lasers;
    std::string target_asteroid_id;
    int max_lasers = 3;
    double ore_hold_capacity = 5000.0;   // m3
    double ore_hold_current = 0.0;       // m3
    double total_ore_mined = 0.0;
    int total_cycles_completed = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(AsteroidMiningLaser)
};

// ==================== Power Grid Management ====================

/**
 * @brief Ship power grid budget tracking
 *
 * Tracks total powergrid (MW) output and per-module power draw.
 * Modules may be onlined/offlined.  Overloaded grid triggers an
 * automatic module shutdown (lowest-priority first) each tick.
 */
class PowerGridState : public ecs::Component {
public:
    struct FittedModule {
        std::string module_id;
        float power_draw = 0.0f;    // MW
        int priority = 5;           // 1 = lowest, 10 = highest
        bool online = false;
    };

    std::vector<FittedModule> modules;
    int max_modules = 12;
    float total_output = 1000.0f;   // MW available
    float total_draw = 0.0f;        // MW currently consumed
    int total_overloads = 0;
    int total_onlined = 0;
    int total_offlined = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(PowerGridState)
};

// ==================== Salvage Drone ====================

/**
 * @brief Salvage drone bay — autonomous wreck salvaging
 *
 * Manages a bay of salvage drones that can be deployed to wrecks.
 * Each drone locks a wreck, cycles a salvage attempt, and on success
 * deposits the recovered material into the ship's hold.
 */
class SalvageDroneBay : public ecs::Component {
public:
    enum class DroneState { Idle, Deployed, Salvaging, Returning };

    struct SalvageDrone {
        std::string drone_id;
        std::string wreck_target_id;
        DroneState state = DroneState::Idle;
        float cycle_time = 10.0f;        // seconds per salvage attempt
        float cycle_progress = 0.0f;
        float success_chance = 0.5f;     // 0.0–1.0
        int successful_salvages = 0;
    };

    std::vector<SalvageDrone> drones;
    int max_drones = 5;
    int total_salvages = 0;
    int total_failures = 0;
    int total_deployed = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SalvageDroneBay)
};

// ==================== Cargo Hold Management ====================

/**
 * @brief Cargo hold capacity and item tracking
 *
 * Manages a ship's cargo bay with per-item volume accounting.  Items
 * stack by item_id.  Adding items that exceed remaining capacity is
 * rejected.  Items can be jettisoned (removed and flagged for space
 * loot spawning).
 */
class CargoHoldState : public ecs::Component {
public:
    struct CargoItem {
        std::string item_id;
        int quantity = 0;
        float volume_per_unit = 1.0f;   // m³ per unit
    };

    std::vector<CargoItem> items;
    int max_item_stacks = 50;
    float max_volume = 500.0f;          // m³ total capacity
    float used_volume = 0.0f;           // m³ currently used
    int total_jettisoned = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CargoHoldState)
};

/**
 * @brief Skill training queue with SP accrual and level completion tracking
 *
 * Skills have 5 levels; SP required = base_sp_cost × level².  The front
 * skill in the queue accrues SP each tick.  When complete, training moves
 * to the next entry.  Supports pause/resume and a max queue size of 10.
 */
class SkillTrainingState : public ecs::Component {
public:
    struct SkillEntry {
        std::string skill_id;
        int target_level = 1;    // 1–5
        int current_level = 0;
        int base_sp_cost = 1000;
        float accumulated_sp = 0.0f;
        bool completed = false;
    };

    std::vector<SkillEntry> queue;
    int max_queue_size = 10;
    float sp_per_second = 10.0f;
    int total_skills_completed = 0;
    float total_sp_earned = 0.0f;
    bool paused = false;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SkillTrainingState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_SHIP_COMPONENTS_H
