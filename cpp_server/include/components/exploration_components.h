#ifndef NOVAFORGE_COMPONENTS_EXPLORATION_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_EXPLORATION_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

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
// ==================== Ancient Tech Upgrade State ====================

/**
 * @brief Tracks the upgrade process for ancient tech modules
 * 
 * Paired with AncientTechModule to track the upgrade from Repaired → Upgraded state,
 * where modules gain rule-breaking stat bonuses exceeding modern limits.
 */
class AncientTechUpgradeState : public ecs::Component {
public:
    bool upgrading = false;
    float upgrade_progress = 0.0f;
    float upgrade_cost = 200.0f;    // Time units to complete upgrade
    std::string bonus_type;          // e.g. "shield", "weapon", "engine"
    float bonus_magnitude = 0.0f;    // Actual bonus value (computed on upgrade complete)

    COMPONENT_TYPE(AncientTechUpgradeState)
};
// ==================== Ancient AI Remnants ====================

/**
 * @brief An AI remnant boss guarding an ancient tech site
 *
 * These are autonomous AI entities that persist at ancient sites,
 * with difficulty and type determined by the site tier.
 */
class AncientAIRemnant : public ecs::Component {
public:
    enum class RemnantType {
        Sentinel,   // Tier 1 - Single defensive guardian
        Swarm,      // Tier 2 - Multiple small drones
        Construct,  // Tier 3 - Assembled ancient machine
        Warden,     // Tier 4 - Powerful site protector
        Leviathan   // Tier 5 - Massive ancient entity
    };

    struct RewardEntry {
        std::string item_id;
        float drop_chance = 0.5f;
        int quantity = 1;
    };

    std::string remnant_id;
    std::string site_id;          // Ancient site this remnant guards
    RemnantType remnant_type = RemnantType::Sentinel;
    int tier = 1;                 // 1-5 site tier
    float difficulty = 1.0f;
    float hit_points = 1500.0f;
    float damage_output = 50.0f;
    float active_time = 0.0f;
    float max_duration = 7200.0f; // 2 hours before despawn
    bool active = true;
    bool defeated = false;
    int recommended_fleet_size = 1;
    std::vector<RewardEntry> rewards;

    bool isActive() const { return active && active_time < max_duration; }
    bool isExpired() const { return active_time >= max_duration; }

    static std::string getRemnantTypeName(RemnantType t) {
        switch (t) {
            case RemnantType::Sentinel: return "Sentinel";
            case RemnantType::Swarm: return "Swarm";
            case RemnantType::Construct: return "Construct";
            case RemnantType::Warden: return "Warden";
            case RemnantType::Leviathan: return "Leviathan";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(AncientAIRemnant)
};


// ==================== Terraforming ====================

/**
 * @brief Long-term planet modification data (Phase 14)
 *
 * Tracks a multi-stage terraforming project on a planet, progressing through
 * Planning → Infrastructure → AtmosphereProcessing → TemperatureRegulation →
 * BiomeSeeding → Complete.
 */
class Terraforming : public ecs::Component {
public:
    enum class TerraformStage {
        Planning,
        Infrastructure,
        AtmosphereProcessing,
        TemperatureRegulation,
        BiomeSeeding,
        Complete
    };

    std::string planet_id;
    TerraformStage stage = TerraformStage::Planning;
    float progress = 0.0f;           // 0.0 to 1.0 progress in current stage
    float total_progress = 0.0f;     // overall progress across all stages (0.0 to 1.0)
    float atmosphere_target = 1.0f;
    float temperature_target = 293.0f;   // target temperature in Kelvin (20°C)
    float water_coverage_target = 0.7f;
    float current_atmosphere = 0.0f;
    float current_temperature = 200.0f;  // initial cold planet
    float current_water_coverage = 0.0f;
    float resource_cost_per_tick = 100.0f;
    double total_credits_spent = 0.0;
    bool is_active = false;
    float time_per_stage = 3600.0f;      // seconds per stage at full rate
    float elapsed_in_stage = 0.0f;

    static std::string stageToString(TerraformStage s) {
        switch (s) {
            case TerraformStage::Planning: return "planning";
            case TerraformStage::Infrastructure: return "infrastructure";
            case TerraformStage::AtmosphereProcessing: return "atmosphere_processing";
            case TerraformStage::TemperatureRegulation: return "temperature_regulation";
            case TerraformStage::BiomeSeeding: return "biome_seeding";
            case TerraformStage::Complete: return "complete";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(Terraforming)
};

// ==================== Rover Interior System (Phase 14) ====================

/**
 * @brief Rover interior configuration component
 *
 * Enables rover interiors with rig locker, equipment mount, and scannable rooms.
 * Each rover can have multiple rooms with different purposes.
 */
class RoverInterior : public ecs::Component {
public:
    enum class RoomType {
        Cockpit,        // Driver's seat and controls
        CargoHold,      // Storage area
        RigLocker,      // Suit/rig storage and equip area
        EquipmentBay,   // Tool and equipment mounting
        Scanner,        // Scanning equipment room
        Airlock         // Entry/exit point
    };

    struct InteriorRoom {
        std::string room_id;
        RoomType type = RoomType::Cockpit;
        float size_x = 2.0f;  // meters
        float size_y = 2.0f;
        float size_z = 2.0f;
        bool is_pressurized = true;
        int equipment_slots = 0;
        std::vector<std::string> installed_equipment_ids;
    };

    std::string rover_id;
    std::vector<InteriorRoom> rooms;
    int max_rooms = 4;
    float total_interior_volume = 0.0f;  // cubic meters
    bool has_rig_locker = false;
    bool has_equipment_bay = false;
    int rig_locker_capacity = 2;        // max rigs that can be stored
    int equipment_bay_slots = 4;        // max equipment items
    std::vector<std::string> stored_rig_ids;
    bool is_sealed = true;              // interior pressurization state
    float oxygen_level = 100.0f;        // percent

    int getRoomCount() const { return static_cast<int>(rooms.size()); }
    bool canAddRoom() const { return static_cast<int>(rooms.size()) < max_rooms; }

    static std::string roomTypeToString(RoomType t) {
        switch (t) {
            case RoomType::Cockpit: return "cockpit";
            case RoomType::CargoHold: return "cargo_hold";
            case RoomType::RigLocker: return "rig_locker";
            case RoomType::EquipmentBay: return "equipment_bay";
            case RoomType::Scanner: return "scanner";
            case RoomType::Airlock: return "airlock";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(RoverInterior)
};

// ==================== Bike Garage System (Phase 14) ====================

/**
 * @brief Bike storage component for rovers and ships
 *
 * Manages storage of grav bikes in vehicles. Supports store/retrieve operations
 * with capacity limits and docking state tracking.
 */
class BikeGarage : public ecs::Component {
public:
    struct StoredBike {
        std::string bike_id;
        uint64_t bike_seed = 0;
        std::string faction_style;
        float fuel_level = 100.0f;     // percent remaining
        float hull_integrity = 100.0f; // percent remaining
        bool is_locked = false;
    };

    std::string owner_entity_id;       // Ship or rover that owns this garage
    int max_capacity = 2;              // max bikes that can be stored
    std::vector<StoredBike> stored_bikes;
    bool is_open = false;              // garage bay door state
    float bay_door_progress = 0.0f;    // 0.0 = closed, 1.0 = open
    float door_speed = 0.5f;           // door movement per second
    bool power_enabled = true;         // needs power to operate

    int getBikeCount() const { return static_cast<int>(stored_bikes.size()); }
    bool canStoreBike() const { return power_enabled && getBikeCount() < max_capacity; }
    bool isFull() const { return getBikeCount() >= max_capacity; }

    bool hasBike(const std::string& bike_id) const {
        for (const auto& b : stored_bikes) {
            if (b.bike_id == bike_id) return true;
        }
        return false;
    }

    COMPONENT_TYPE(BikeGarage)
};

// ==================== Visual Rig Generator System (Phase 13) ====================

/**
 * @brief Visual rig state component for PCG shape generation
 *
 * Tracks the visual state of a rig with installed modules affecting
 * appearance (thrusters, cargo pods, shield emitters, etc.)
 */
class VisualRigState : public ecs::Component {
public:
    enum class ThrusterConfig {
        None,
        Single,    // One central thruster
        Dual,      // Two side thrusters
        Quad       // Four corner thrusters
    };

    enum class CargoSize {
        None,
        Small,     // Compact cargo pod
        Medium,    // Standard cargo pod
        Large      // Extended cargo rack
    };

    std::string rig_entity_id;
    uint64_t visual_seed = 0;          // PCG seed for visual variation
    ThrusterConfig thruster_config = ThrusterConfig::None;
    CargoSize cargo_size = CargoSize::None;
    float thruster_scale = 1.0f;       // size multiplier for thrusters
    float cargo_scale = 1.0f;          // size multiplier for cargo pods
    bool has_shield_emitter = false;
    bool has_antenna = false;
    bool has_solar_panels = false;
    bool has_drone_bay = false;
    int weapon_mount_count = 0;
    int tool_mount_count = 0;
    float total_bulk = 1.0f;           // overall size modifier
    float glow_intensity = 0.0f;       // power indicator glow
    std::string primary_color;         // faction/custom color
    std::string secondary_color;
    std::vector<std::string> trinket_ids;  // attached trinkets (bobbleheads, stickers)
    int max_trinkets = 4;

    bool canAddTrinket() const { return static_cast<int>(trinket_ids.size()) < max_trinkets; }

    static std::string thrusterConfigToString(ThrusterConfig t) {
        switch (t) {
            case ThrusterConfig::None: return "none";
            case ThrusterConfig::Single: return "single";
            case ThrusterConfig::Dual: return "dual";
            case ThrusterConfig::Quad: return "quad";
            default: return "unknown";
        }
    }

    static std::string cargoSizeToString(CargoSize s) {
        switch (s) {
            case CargoSize::None: return "none";
            case CargoSize::Small: return "small";
            case CargoSize::Medium: return "medium";
            case CargoSize::Large: return "large";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(VisualRigState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_EXPLORATION_COMPONENTS_H
