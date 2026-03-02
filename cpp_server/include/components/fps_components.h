#ifndef NOVAFORGE_COMPONENTS_FPS_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_FPS_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

// ==================== FPS Spawn Point ====================

/**
 * @brief FPS spawn point — where a player materialises in first-person mode
 *
 * When a player transitions to FPS / Interior mode (e.g. after docking),
 * this component defines the position and orientation of the spawn.
 *
 * Spawn locations depend on context:
 *   - Hangar: spawn inside the hangar next to docked ship
 *   - Station: spawn in the station main concourse
 *   - Ship: spawn in the bridge / cockpit area
 *   - Tether arm: spawn at the docking arm airlock
 */
class FPSSpawnPoint : public ecs::Component {
public:
    enum class SpawnContext {
        Hangar,         // Inside station hangar (small/medium ships)
        StationLobby,   // Station main concourse
        ShipBridge,     // Ship bridge / cockpit
        TetherAirlock,  // Docking arm airlock (capital+ ships)
        EVAHatch        // EVA exit point
    };

    std::string spawn_id;
    std::string parent_entity_id;         // Station, ship, or arm entity
    SpawnContext context = SpawnContext::Hangar;

    float pos_x = 0.0f;
    float pos_y = 0.0f;
    float pos_z = 0.0f;
    float yaw = 0.0f;                    // Facing direction (degrees)

    bool is_active = true;               // Can be disabled during events

    COMPONENT_TYPE(FPSSpawnPoint)
};

/**
 * @brief Runtime state for a player-controlled first-person character
 *        moving inside a ship or station interior.
 */
class FPSCharacterState : public ecs::Component {
public:
    enum class Stance {
        Standing = 0,
        Crouching = 1,
        Sprinting = 2
    };

    std::string player_id;
    std::string interior_id;      // Which ship/station interior the character is in
    std::string current_room_id;  // Which room inside the interior (for hazard scoping)

    // Position in interior-local coordinates
    float pos_x = 0.0f;
    float pos_y = 0.0f;  // vertical (up)
    float pos_z = 0.0f;

    // Orientation
    float yaw = 0.0f;     // degrees, horizontal look
    float pitch = 0.0f;   // degrees, vertical look (clamped ±89)

    // Movement state
    float move_x = 0.0f;  // Input direction X (-1..1)
    float move_z = 0.0f;  // Input direction Z (-1..1)
    float vel_y = 0.0f;   // Vertical velocity (for jump/fall)

    int stance = 0;        // Stance enum as int
    bool grounded = true;
    bool jump_requested = false;

    // Movement parameters
    float walk_speed = 4.0f;       // m/s
    float sprint_speed = 7.0f;     // m/s
    float crouch_speed = 2.0f;     // m/s
    float jump_impulse = 5.0f;     // m/s upward
    float gravity = 9.81f;         // m/s² (can be 0 for zero-g)
    float standing_height = 1.8f;  // metres
    float crouch_height = 1.0f;    // metres

    // Stamina for sprint
    float stamina = 100.0f;
    float stamina_max = 100.0f;
    float stamina_drain = 20.0f;   // per second while sprinting
    float stamina_regen = 10.0f;   // per second while not sprinting

    COMPONENT_TYPE(FPSCharacterState)
};

// ==================== Interior Door ====================

/**
 * @brief Interior door with state machine, access control, and pressure management
 *
 * Doors can be standard, airlock (pressure-sealed), or security (restricted).
 * They support open/close animations, locking, and pressure-aware operation
 * (airlocks refuse to open if there is a pressure differential).
 */
class InteriorDoor : public ecs::Component {
public:
    enum class DoorType {
        Standard = 0,   // Normal interior door
        Airlock = 1,    // Pressure-sealed door (between pressurised/vacuum zones)
        Security = 2    // Access-restricted door (keycard/hack required)
    };

    enum class DoorState {
        Closed = 0,
        Opening = 1,
        Open = 2,
        Closing = 3,
        Locked = 4
    };

    std::string door_id;
    std::string interior_id;        // Parent ship/station interior
    std::string room_a_id;          // Room on side A
    std::string room_b_id;          // Room on side B

    int door_type = 0;              // DoorType as int
    int door_state = 0;             // DoorState as int

    float open_progress = 0.0f;     // 0 = closed, 1 = fully open
    float open_speed = 2.0f;        // Seconds to fully open/close

    bool is_locked = false;
    std::string required_access;    // Access level required (empty = none)
    float pressure_a = 1.0f;        // Atmospheric pressure on side A (0=vacuum, 1=normal)
    float pressure_b = 1.0f;        // Atmospheric pressure on side B
    float pressure_threshold = 0.5f; // Max pressure differential for airlock operation
    bool pressure_warning = false;   // True if pressure differential exceeds threshold

    float auto_close_timer = 0.0f;  // Seconds until auto-close (0 = no auto-close)
    float auto_close_delay = 5.0f;  // How long to stay open before auto-closing

    COMPONENT_TYPE(InteriorDoor)
};

// ==================== EVA Airlock ====================

/**
 * @brief EVA airlock state for transitioning between interior and space
 *
 * Manages the multi-step EVA exit/entry sequence:
 *   1. Enter airlock chamber
 *   2. Inner door seals
 *   3. Chamber depressurizes
 *   4. Outer door opens
 *   5. Player exits into space (EVA mode)
 *
 * Re-entry reverses the sequence. The player must have a suit with
 * sufficient oxygen to perform EVA.
 */
class EVAAirlockState : public ecs::Component {
public:
    enum class Phase {
        Idle = 0,              // No EVA in progress
        EnterChamber = 1,      // Player entering airlock chamber
        InnerSeal = 2,         // Inner door sealing
        Depressurize = 3,      // Chamber depressurizing
        OuterOpen = 4,         // Outer door opening
        EVAActive = 5,         // Player is in space (EVA)
        OuterSeal = 6,         // Outer door sealing (re-entry)
        Repressurize = 7,      // Chamber repressurizing
        InnerOpen = 8,         // Inner door opening (re-entry)
        Complete = 9           // Sequence complete
    };

    std::string airlock_id;
    std::string ship_id;           // Parent ship/station
    std::string player_id;         // Player currently using the airlock

    int phase = 0;                 // Phase enum as int
    float phase_progress = 0.0f;   // 0..1 progress through current phase
    float phase_duration = 2.0f;   // Seconds per phase step

    float chamber_pressure = 1.0f; // 0=vacuum, 1=normal atmosphere
    bool inner_door_open = false;
    bool outer_door_open = false;

    float min_suit_oxygen = 10.0f;  // Minimum oxygen to allow EVA
    bool suit_check_passed = false;

    bool abort_requested = false;

    COMPONENT_TYPE(EVAAirlockState)
};

// ---------------------------------------------------------------------------
// FPS Interaction
// ---------------------------------------------------------------------------

class FPSInteractable : public ecs::Component {
public:
    enum class InteractionType {
        Door = 0,
        Airlock = 1,
        Terminal = 2,
        LootContainer = 3,
        Fabricator = 4,
        MedicalBay = 5
    };

    std::string interactable_id;
    std::string interior_id;          // Which interior this belongs to
    std::string linked_entity_id;     // Door/airlock/terminal entity this triggers

    int interaction_type = 0;         // InteractionType as int
    float interact_range = 2.0f;      // Max distance to interact (metres)

    float pos_x = 0.0f;              // Position within the interior
    float pos_y = 0.0f;
    float pos_z = 0.0f;

    std::string display_name;         // UI prompt text (e.g. "Open Door", "Use Terminal")
    std::string required_access;      // Access level required (empty = none)
    bool is_enabled = true;           // Can be disabled during events

    COMPONENT_TYPE(FPSInteractable)
};

// ---------------------------------------------------------------------------
// FPS Personal Weapon (distinct from ship Weapon)
// ---------------------------------------------------------------------------

class FPSWeapon : public ecs::Component {
public:
    enum class WeaponCategory {
        Sidearm = 0,
        Rifle = 1,
        Shotgun = 2,
        Tool = 3          // Repair tool, mining laser, etc.
    };

    std::string weapon_id;
    std::string owner_id;             // Player or NPC who owns this weapon

    int category = 0;                 // WeaponCategory as int
    std::string damage_type = "kinetic";  // em, thermal, kinetic, explosive
    float damage = 15.0f;
    float range = 50.0f;              // Effective range (metres)
    float fire_rate = 0.5f;           // Seconds between shots
    float cooldown = 0.0f;            // Current cooldown timer
    float spread = 1.0f;              // Accuracy cone (degrees, 0 = perfect)

    int ammo = 30;
    int ammo_max = 30;
    float reload_time = 2.0f;         // Seconds to reload
    float reload_progress = 0.0f;     // 0..1 during reload
    bool is_reloading = false;

    bool is_equipped = false;

    COMPONENT_TYPE(FPSWeapon)
};

// ---------------------------------------------------------------------------
// FPS Personal Health (separate from ship Health)
// ---------------------------------------------------------------------------

class FPSHealth : public ecs::Component {
public:
    std::string owner_id;

    float health = 100.0f;
    float health_max = 100.0f;
    float shield = 50.0f;
    float shield_max = 50.0f;
    float shield_recharge_rate = 5.0f;    // Per second
    float shield_recharge_delay = 3.0f;   // Seconds after last hit before recharge
    float time_since_last_hit = 999.0f;   // Tracks delay

    bool is_alive = true;

    float getHealthFraction() const {
        return (health_max > 0.0f) ? health / health_max : 0.0f;
    }
    float getShieldFraction() const {
        return (shield_max > 0.0f) ? shield / shield_max : 0.0f;
    }

    COMPONENT_TYPE(FPSHealth)
};

// ---------------------------------------------------------------------------
// FPS Inventory (personal items carried on foot)
// ---------------------------------------------------------------------------

class FPSInventoryComponent : public ecs::Component {
public:
    struct InventorySlot {
        std::string item_id;
        std::string item_name;
        int quantity = 1;
    };

    std::string owner_id;
    std::vector<InventorySlot> slots;
    int max_slots = 8;

    std::string equipped_weapon_id;         // Currently equipped FPSWeapon entity
    std::string equipped_tool_id;           // Currently equipped tool entity

    bool hasItem(const std::string& item_id) const {
        for (const auto& s : slots) {
            if (s.item_id == item_id) return true;
        }
        return false;
    }

    int itemCount() const { return static_cast<int>(slots.size()); }

    bool isFull() const { return static_cast<int>(slots.size()) >= max_slots; }

    COMPONENT_TYPE(FPSInventoryComponent)
};

// ==================== Ship Interior Layout ====================

/**
 * @brief Procedural room layout for a ship interior
 *
 * Defines rooms, corridors, and connections that make up a ship's
 * walkable interior.  Room types depend on ship class and size.
 */
class ShipInteriorLayout : public ecs::Component {
public:
    enum class RoomType {
        Bridge = 0,
        Engineering = 1,
        CargoHold = 2,
        CrewQuarters = 3,
        MedicalBay = 4,
        Armory = 5,
        Corridor = 6,
        Airlock = 7,
        HangarBay = 8,
        ScienceLab = 9
    };

    struct Room {
        std::string room_id;
        int room_type = 0;                // RoomType as int
        float size_x = 6.0f;              // metres
        float size_y = 3.0f;              // ceiling height
        float size_z = 6.0f;
        float pos_x = 0.0f;              // position in interior-local coords
        float pos_y = 0.0f;
        float pos_z = 0.0f;
        bool has_gravity = true;
        bool is_pressurized = true;
    };

    struct Connection {
        std::string from_room_id;
        std::string to_room_id;
        std::string door_id;              // links to InteriorDoor entity
    };

    std::string interior_id;
    std::string ship_id;
    std::string ship_class;                // "frigate", "cruiser", "battleship", etc.
    std::vector<Room> rooms;
    std::vector<Connection> connections;

    int roomCount() const { return static_cast<int>(rooms.size()); }
    int connectionCount() const { return static_cast<int>(connections.size()); }

    bool hasRoom(const std::string& room_id) const {
        for (const auto& r : rooms) {
            if (r.room_id == room_id) return true;
        }
        return false;
    }

    const Room* getRoom(const std::string& room_id) const {
        for (const auto& r : rooms) {
            if (r.room_id == room_id) return &r;
        }
        return nullptr;
    }

    bool areConnected(const std::string& a, const std::string& b) const {
        for (const auto& c : connections) {
            if ((c.from_room_id == a && c.to_room_id == b) ||
                (c.from_room_id == b && c.to_room_id == a))
                return true;
        }
        return false;
    }

    static std::string roomTypeName(int type) {
        switch (static_cast<RoomType>(type)) {
            case RoomType::Bridge:       return "Bridge";
            case RoomType::Engineering:  return "Engineering";
            case RoomType::CargoHold:    return "CargoHold";
            case RoomType::CrewQuarters: return "CrewQuarters";
            case RoomType::MedicalBay:   return "MedicalBay";
            case RoomType::Armory:       return "Armory";
            case RoomType::Corridor:     return "Corridor";
            case RoomType::Airlock:      return "Airlock";
            case RoomType::HangarBay:    return "HangarBay";
            case RoomType::ScienceLab:   return "ScienceLab";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(ShipInteriorLayout)
};

// ==================== Environmental Hazard ====================

/**
 * @brief Environmental hazard affecting a room or area within a ship interior
 *
 * Hazards have severity levels, spread over time if unrepaired, and
 * deal damage to FPS characters in affected areas.
 */
class EnvironmentalHazard : public ecs::Component {
public:
    enum class HazardType {
        HullBreach = 0,      // Causes depressurization
        Fire = 1,            // Deals heat damage, spreads to adjacent rooms
        Radiation = 2,       // Deals radiation damage through walls
        ElectricalFault = 3, // Disables room systems, shocks nearby characters
        ToxicLeak = 4        // Poisons characters, requires EVA suit
    };

    enum class Severity {
        Minor = 0,           // Slow damage, easily repaired
        Moderate = 1,        // Moderate damage, requires tools
        Critical = 2,        // High damage, may spread
        Catastrophic = 3     // Extreme damage, spreads rapidly
    };

    std::string hazard_id;
    std::string room_id;         // The room this hazard affects
    std::string interior_id;     // Which ship/station interior

    int hazard_type = 0;         // HazardType as int
    int severity = 0;            // Severity as int

    float damage_per_second = 5.0f;   // DPS to characters in the room
    float spread_timer = 0.0f;        // Time until hazard spreads to adjacent room
    float spread_interval = 30.0f;    // Seconds between spread attempts
    float repair_progress = 0.0f;     // 0.0 = unrepaired, 1.0 = fully repaired
    float repair_rate = 0.1f;         // Progress per second when being repaired

    bool is_active = true;
    bool is_spreading = false;
    bool is_being_repaired = false;

    static std::string hazardTypeName(int type) {
        switch (static_cast<HazardType>(type)) {
            case HazardType::HullBreach:      return "HullBreach";
            case HazardType::Fire:            return "Fire";
            case HazardType::Radiation:       return "Radiation";
            case HazardType::ElectricalFault: return "ElectricalFault";
            case HazardType::ToxicLeak:       return "ToxicLeak";
            default: return "Unknown";
        }
    }

    static std::string severityName(int sev) {
        switch (static_cast<Severity>(sev)) {
            case Severity::Minor:        return "Minor";
            case Severity::Moderate:     return "Moderate";
            case Severity::Critical:     return "Critical";
            case Severity::Catastrophic: return "Catastrophic";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(EnvironmentalHazard)
};

// ==================== FPS Objective ====================

/**
 * @brief On-foot mission objective for FPS gameplay
 *
 * Tracks progress on boarding actions, rescue, sabotage, defense,
 * and other objectives that occur while a player is on foot inside
 * a ship or station.
 */
class FPSObjective : public ecs::Component {
public:
    enum class ObjectiveType {
        EliminateHostiles = 0,   // Kill all enemies in the area
        RescueVIP = 1,           // Escort/find a VIP NPC
        Sabotage = 2,            // Destroy or disable a target object
        DefendPoint = 3,         // Hold a position for a duration
        RetrieveItem = 4,        // Find and pick up an item
        RepairSystem = 5,        // Fix a broken system (ties to EnvironmentalHazard)
        Escape = 6               // Reach an extraction point
    };

    enum class ObjectiveState {
        Inactive = 0,
        Active = 1,
        Completed = 2,
        Failed = 3
    };

    std::string objective_id;
    std::string interior_id;       // Which ship/station interior
    std::string room_id;           // Target room (if applicable)
    std::string assigned_player;   // Player this objective belongs to

    int objective_type = 0;        // ObjectiveType as int
    int state = 0;                 // ObjectiveState as int

    std::string description;       // Human-readable description

    // Progress tracking
    float progress = 0.0f;         // 0.0 to 1.0
    float time_limit = 0.0f;       // Seconds (0 = no time limit)
    float elapsed_time = 0.0f;

    // Type-specific fields
    int hostiles_required = 0;     // For EliminateHostiles
    int hostiles_killed = 0;
    float defend_duration = 0.0f;  // For DefendPoint
    float defend_elapsed = 0.0f;
    std::string target_item_id;    // For RetrieveItem / Sabotage
    bool item_collected = false;

    static std::string objectiveTypeName(int type) {
        switch (static_cast<ObjectiveType>(type)) {
            case ObjectiveType::EliminateHostiles: return "EliminateHostiles";
            case ObjectiveType::RescueVIP:         return "RescueVIP";
            case ObjectiveType::Sabotage:          return "Sabotage";
            case ObjectiveType::DefendPoint:       return "DefendPoint";
            case ObjectiveType::RetrieveItem:      return "RetrieveItem";
            case ObjectiveType::RepairSystem:      return "RepairSystem";
            case ObjectiveType::Escape:            return "Escape";
            default: return "Unknown";
        }
    }

    static std::string stateName(int s) {
        switch (static_cast<ObjectiveState>(s)) {
            case ObjectiveState::Inactive:  return "Inactive";
            case ObjectiveState::Active:    return "Active";
            case ObjectiveState::Completed: return "Completed";
            case ObjectiveState::Failed:    return "Failed";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(FPSObjective)
};


// ==================== Food Processor ====================

/**
 * @brief Survival module food processing component (Phase 13)
 *
 * Stores recipes, active crafting jobs, and processor state for
 * turning raw ingredients into nutrition items.
 */
class FoodProcessor : public ecs::Component {
public:
    struct Recipe {
        std::string recipe_id;
        std::string output_item;
        int output_quantity = 1;
        std::vector<std::pair<std::string, int>> ingredients;
        float craft_time = 10.0f;
        float nutrition_value = 25.0f;
    };

    struct CraftJob {
        std::string recipe_id;
        float time_remaining = 0.0f;
        float total_time = 10.0f;
        bool completed = false;
        std::string owner_id;
    };

    std::vector<Recipe> available_recipes;
    std::vector<CraftJob> active_jobs;
    int max_concurrent_jobs = 1;
    float efficiency = 1.0f;
    bool powered = true;

    COMPONENT_TYPE(FoodProcessor)
};


// ==================== Rig Locker Preset ====================

/**
 * @brief Rig locker preset management component (Phase 13)
 *
 * Stores saved suit/rig presets for quick equipping in the rig locker.
 * Each preset stores a named module loadout that can be saved, loaded,
 * renamed, favorited, and equipped.
 */
class RigLockerPreset : public ecs::Component {
public:
    struct Preset {
        std::string preset_id;
        std::string name;
        std::vector<std::string> module_ids;  // equipped module IDs
        bool is_favorite = false;
        float total_mass = 0.0f;
        int slot_count = 0;
    };

    std::string owner_id;
    std::vector<Preset> presets;
    std::string active_preset_id;
    int max_presets = 10;
    int total_equips = 0;
    int next_preset_seq = 0;

    const Preset* findPreset(const std::string& id) const {
        for (const auto& p : presets) {
            if (p.preset_id == id) return &p;
        }
        return nullptr;
    }

    Preset* findPreset(const std::string& id) {
        for (auto& p : presets) {
            if (p.preset_id == id) return &p;
        }
        return nullptr;
    }

    int favoriteCount() const {
        int count = 0;
        for (const auto& p : presets) {
            if (p.is_favorite) count++;
        }
        return count;
    }

    COMPONENT_TYPE(RigLockerPreset)
};

// ==================== FPS Salvage Path ====================

/**
 * @brief FPS salvage path component (Phase 13)
 *
 * Tracks FPS-mode salvage exploration state including entry point cutting,
 * room-by-room exploration progress, and loot discovery.
 */
class FPSSalvagePath : public ecs::Component {
public:
    enum class EntryState {
        Sealed,
        Cutting,
        Open
    };

    enum class LootRarity {
        Common,
        Uncommon,
        Rare,
        Epic,
        Legendary
    };

    struct EntryPoint {
        std::string entry_id;
        EntryState state = EntryState::Sealed;
        float cut_progress = 0.0f;
        float cut_required = 10.0f;  // seconds to cut through
        std::string tool_required;
    };

    struct LootNode {
        std::string loot_id;
        std::string item_name;
        LootRarity rarity = LootRarity::Common;
        bool discovered = false;
        bool collected = false;
        float value = 0.0f;
    };

    std::string site_id;
    std::string explorer_id;
    std::vector<EntryPoint> entry_points;
    std::vector<LootNode> loot_nodes;
    float exploration_progress = 0.0f;  // 0.0 to 1.0
    int rooms_explored = 0;
    int total_rooms = 0;
    bool active = false;
    int total_collections = 0;

    EntryPoint* findEntry(const std::string& id) {
        for (auto& e : entry_points) {
            if (e.entry_id == id) return &e;
        }
        return nullptr;
    }

    const EntryPoint* findEntry(const std::string& id) const {
        for (const auto& e : entry_points) {
            if (e.entry_id == id) return &e;
        }
        return nullptr;
    }

    LootNode* findLoot(const std::string& id) {
        for (auto& l : loot_nodes) {
            if (l.loot_id == id) return &l;
        }
        return nullptr;
    }

    int discoveredCount() const {
        int count = 0;
        for (const auto& l : loot_nodes) {
            if (l.discovered) count++;
        }
        return count;
    }

    int collectedCount() const {
        int count = 0;
        for (const auto& l : loot_nodes) {
            if (l.collected) count++;
        }
        return count;
    }

    COMPONENT_TYPE(FPSSalvagePath)
};

// ==================== Lavatory Interaction ====================

class LavatoryInteraction : public ecs::Component {
public:
    enum class InteractionPhase {
        Idle = 0,
        Approaching = 1,
        DoorOpening = 2,
        TransitionToThirdPerson = 3,
        UsingFacility = 4,
        TransitionToFirstPerson = 5,
        DoorClosing = 6,
        Complete = 7
    };

    std::string lavatory_id;
    std::string room_id;
    std::string user_id;

    int phase = 0;                    // InteractionPhase as int
    float phase_progress = 0.0f;      // 0..1 progress through current phase
    float phase_duration = 1.5f;      // Seconds per phase

    bool door_open = false;
    bool in_third_person = false;
    bool audio_playing = false;
    float use_duration = 5.0f;        // How long the facility is used
    float use_timer = 0.0f;

    bool occupied = false;
    float hygiene_bonus = 15.0f;      // Fatigue reduction on use

    COMPONENT_TYPE(LavatoryInteraction)
};

// ==================== EVA Airlock Exit ====================

class EVAAirlockExit : public ecs::Component {
public:
    enum class ExitState {
        Inactive = 0,
        RequestingExit = 1,
        CheckingDockState = 2,
        PreparingExit = 3,
        Exiting = 4,
        InSpace = 5,
        Returning = 6,
        Complete = 7
    };

    std::string airlock_id;
    std::string ship_id;
    std::string player_id;

    int state = 0;                     // ExitState as int
    float state_progress = 0.0f;       // 0..1 progress
    float state_duration = 2.0f;       // Seconds per state

    bool ship_docked = false;          // Is the ship currently docked?
    bool exit_blocked = false;         // Exit blocked (e.g., docked)
    float suit_oxygen = 100.0f;        // Player suit oxygen level
    float min_oxygen = 10.0f;          // Minimum oxygen to allow exit

    float distance_from_ship = 0.0f;   // Distance after exit
    float max_tether_range = 200.0f;   // Max distance before tether recall
    bool tether_active = true;

    COMPONENT_TYPE(EVAAirlockExit)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_FPS_COMPONENTS_H
