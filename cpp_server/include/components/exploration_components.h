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

// ==================== Planetary Traversal System (Phase 14) ====================

/**
 * @brief Planetary surface traversal component
 *
 * Surface movement across planetary terrain with vehicle support and
 * terrain speed modifiers. Tracks position, heading, and distance traveled.
 */
class PlanetaryTraversal : public ecs::Component {
public:
    enum class TerrainType {
        Plains,
        Mountains,
        Valleys,
        Plateaus,
        Craters,
        Dunes,
        Tundra,
        Volcanic
    };

    std::string planet_entity_id;
    float position_x = 0.0f;
    float position_y = 0.0f;
    float heading = 0.0f;          // degrees (0-360)
    float speed = 0.0f;            // current speed m/s
    float max_speed = 5.0f;        // on foot default
    TerrainType terrain = TerrainType::Plains;
    float terrain_speed_modifier = 1.0f;
    std::string vehicle_id;        // empty = on foot
    float destination_x = 0.0f;
    float destination_y = 0.0f;
    bool has_destination = false;
    bool is_traversing = false;
    float distance_traveled = 0.0f;

    static float getTerrainModifier(TerrainType t) {
        switch (t) {
            case TerrainType::Plains: return 1.0f;
            case TerrainType::Mountains: return 0.4f;
            case TerrainType::Valleys: return 0.7f;
            case TerrainType::Plateaus: return 0.9f;
            case TerrainType::Craters: return 0.5f;
            case TerrainType::Dunes: return 0.6f;
            case TerrainType::Tundra: return 0.7f;
            case TerrainType::Volcanic: return 0.3f;
            default: return 1.0f;
        }
    }

    static std::string terrainTypeToString(TerrainType t) {
        switch (t) {
            case TerrainType::Plains: return "plains";
            case TerrainType::Mountains: return "mountains";
            case TerrainType::Valleys: return "valleys";
            case TerrainType::Plateaus: return "plateaus";
            case TerrainType::Craters: return "craters";
            case TerrainType::Dunes: return "dunes";
            case TerrainType::Tundra: return "tundra";
            case TerrainType::Volcanic: return "volcanic";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(PlanetaryTraversal)
};

// ==================== Solar Panel System (Phase 14) ====================

/**
 * @brief Solar energy generation component
 *
 * Solar energy generation with day/night cycle, panel degradation,
 * and energy storage. Tracks panel count, efficiency, and battery level.
 */
class SolarPanel : public ecs::Component {
public:
    std::string owner_entity_id;
    int panel_count = 0;
    int max_panels = 10;
    float panel_efficiency = 1.0f;     // 0.0-1.0
    float energy_per_panel = 5.0f;     // MW per panel at full sunlight
    float total_energy_output = 0.0f;  // current output MW
    float day_cycle_position = 0.5f;   // 0.0-1.0, 0.5=noon, 0/1=midnight
    float day_cycle_speed = 0.01f;     // per second advance
    bool is_deployed = false;
    float degradation_rate = 0.001f;   // efficiency loss per second when deployed
    float maintenance_level = 1.0f;    // 0.0-1.0
    float energy_stored = 0.0f;        // MWh in battery
    float max_energy_storage = 100.0f; // MWh max

    float getSunlightFactor() const {
        float angle = day_cycle_position * 3.14159265f;
        float factor = std::sin(angle);
        return factor > 0.0f ? factor : 0.0f;
    }

    COMPONENT_TYPE(SolarPanel)
};

// ==================== Farming Deck System (Phase 14) ====================

/**
 * @brief Agricultural module component
 *
 * Manages crop growth lifecycle with watering, fertilizing, and harvesting.
 * Supports multiple crop plots with different crop types.
 */
class FarmingDeck : public ecs::Component {
public:
    enum class CropType {
        Grain,
        Vegetables,
        Fruit,
        Herbs,
        Algae
    };

    enum class GrowthStage {
        Empty,
        Planted,
        Growing,
        Mature,
        Harvestable,
        Withered
    };

    struct CropPlot {
        std::string plot_id;
        CropType crop_type = CropType::Grain;
        GrowthStage stage = GrowthStage::Empty;
        float growth_progress = 0.0f;   // 0.0-1.0
        float growth_rate = 0.1f;        // per second
        float water_level = 1.0f;        // 0.0-1.0
        float nutrient_level = 1.0f;     // 0.0-1.0
        float water_consumption = 0.02f; // per second
        float nutrient_consumption = 0.01f; // per second
    };

    std::string owner_entity_id;
    std::vector<CropPlot> plots;
    int max_plots = 6;
    float total_food_produced = 0.0f;
    bool is_powered = true;
    float light_level = 1.0f;           // 0.0-1.0
    float temperature = 22.0f;          // celsius, optimal 18-28

    int getPlotCount() const { return static_cast<int>(plots.size()); }
    bool canAddPlot() const { return getPlotCount() < max_plots; }

    static float getYieldMultiplier(CropType t) {
        switch (t) {
            case CropType::Grain: return 1.0f;
            case CropType::Vegetables: return 0.8f;
            case CropType::Fruit: return 0.6f;
            case CropType::Herbs: return 0.4f;
            case CropType::Algae: return 1.5f;
            default: return 1.0f;
        }
    }

    static std::string cropTypeToString(CropType t) {
        switch (t) {
            case CropType::Grain: return "grain";
            case CropType::Vegetables: return "vegetables";
            case CropType::Fruit: return "fruit";
            case CropType::Herbs: return "herbs";
            case CropType::Algae: return "algae";
            default: return "unknown";
        }
    }

    static std::string growthStageToString(GrowthStage s) {
        switch (s) {
            case GrowthStage::Empty: return "empty";
            case GrowthStage::Planted: return "planted";
            case GrowthStage::Growing: return "growing";
            case GrowthStage::Mature: return "mature";
            case GrowthStage::Harvestable: return "harvestable";
            case GrowthStage::Withered: return "withered";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(FarmingDeck)
};

// ==================== Grid Construction System (Phase 14) ====================

/**
 * @brief Snappable grid-based construction for habitats
 *
 * Manages a 2D grid of modules with power networking, structural integrity
 * calculation, and module placement/removal.
 */
class GridConstruction : public ecs::Component {
public:
    enum class ModuleType {
        Empty,
        Foundation,
        Wall,
        Floor,
        PowerNode,
        HabitatModule,
        StorageModule,
        DefenseModule
    };

    struct GridCell {
        ModuleType module_type = ModuleType::Empty;
        std::string module_id;
        float health = 1.0f;               // 0.0-1.0
        bool is_powered = false;
    };

    std::string owner_entity_id;
    int grid_width = 8;
    int grid_height = 8;
    std::vector<std::vector<GridCell>> cells;
    int powered_cell_count = 0;
    float total_power_generation = 0.0f;
    float total_power_consumption = 0.0f;
    float structural_integrity = 0.0f;     // 0.0-1.0
    int total_modules_placed = 0;
    bool is_powered = true;

    void initCells(int w, int h) {
        grid_width = w;
        grid_height = h;
        cells.resize(h);
        for (int y = 0; y < h; y++) {
            cells[y].resize(w);
        }
    }

    bool inBounds(int x, int y) const {
        return x >= 0 && x < grid_width && y >= 0 && y < grid_height;
    }

    int getModuleCount() const {
        int count = 0;
        for (int y = 0; y < grid_height; y++) {
            for (int x = 0; x < grid_width; x++) {
                if (cells[y][x].module_type != ModuleType::Empty) count++;
            }
        }
        return count;
    }

    static std::string moduleTypeToString(ModuleType m) {
        switch (m) {
            case ModuleType::Empty: return "empty";
            case ModuleType::Foundation: return "foundation";
            case ModuleType::Wall: return "wall";
            case ModuleType::Floor: return "floor";
            case ModuleType::PowerNode: return "power_node";
            case ModuleType::HabitatModule: return "habitat_module";
            case ModuleType::StorageModule: return "storage_module";
            case ModuleType::DefenseModule: return "defense_module";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(GridConstruction)
};

// ==================== Ancient Module Discovery ====================

class AncientModuleDiscovery : public ecs::Component {
public:
    enum class DiscoveryState {
        Undiscovered = 0,
        Scanning = 1,
        Discovered = 2,
        Extracting = 3,
        Extracted = 4,
        Analyzed = 5
    };

    struct DiscoveredModule {
        std::string module_id;
        std::string tech_type;
        int state = 0;                // DiscoveryState as int
        float scan_progress = 0.0f;
        float extract_progress = 0.0f;
        float extract_required = 10.0f;
        float repair_difficulty = 0.5f;   // 0..1 difficulty
        bool repairable = true;
        float estimated_value = 0.0f;
    };

    std::string site_id;
    std::string explorer_id;
    std::vector<DiscoveredModule> modules;
    int max_modules = 10;
    bool active = false;
    float scan_range = 50.0f;
    int total_extractions = 0;

    DiscoveredModule* findModule(const std::string& id) {
        for (auto& m : modules) {
            if (m.module_id == id) return &m;
        }
        return nullptr;
    }

    const DiscoveredModule* findModule(const std::string& id) const {
        for (const auto& m : modules) {
            if (m.module_id == id) return &m;
        }
        return nullptr;
    }

    int discoveredCount() const {
        int count = 0;
        for (const auto& m : modules) {
            if (m.state >= static_cast<int>(DiscoveryState::Discovered)) count++;
        }
        return count;
    }

    int extractedCount() const {
        int count = 0;
        for (const auto& m : modules) {
            if (m.state >= static_cast<int>(DiscoveryState::Extracted)) count++;
        }
        return count;
    }

    COMPONENT_TYPE(AncientModuleDiscovery)
};

// ==================== Asteroid Belt System ====================

class AsteroidBelt : public ecs::Component {
public:
    struct Asteroid {
        std::string asteroid_id;
        std::string ore_type;     // "Veldspar", "Scordite", "Pyroxeres", "Kernite", "Omber"
        float quantity = 5000.0f;
        float max_quantity = 5000.0f;
        float richness = 1.0f;    // yield multiplier 0.5-2.0
        bool depleted = false;
    };

    std::string belt_id;
    std::string system_id;
    std::vector<Asteroid> asteroids;
    int max_asteroids = 20;
    float respawn_timer = 0.0f;
    float respawn_interval = 3600.0f;  // seconds
    int total_mined = 0;
    int total_respawned = 0;
    bool active = true;

    COMPONENT_TYPE(AsteroidBelt)
};

// ==================== Scan Probe System ====================

class ScanProbe : public ecs::Component {
public:
    enum class ProbeType { Core, Combat, Survey };
    enum class ProbeState { Idle, Scanning, Complete, Expired };

    struct Probe {
        std::string probe_id;
        ProbeType type = ProbeType::Core;
        ProbeState state = ProbeState::Idle;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float scan_radius = 4.0f;       // AU
        float scan_strength = 1.0f;
        float scan_progress = 0.0f;     // 0-1
        float scan_time = 10.0f;        // seconds
        float lifetime = 300.0f;        // seconds before expiry
    };

    struct ScanResult {
        std::string result_id;
        std::string signature_type;    // "Anomaly", "Wormhole", "Data", "Relic", "Gas", "Combat"
        float signal_strength = 0.0f;  // 0-1
    };

    std::string owner_id;
    std::vector<Probe> probes;
    std::vector<ScanResult> results;
    int max_probes = 8;
    int total_scans_completed = 0;
    int total_sites_found = 0;
    bool active = true;

    COMPONENT_TYPE(ScanProbe)
};

// ==================== Loot Container ====================

/**
 * @brief Manages loot containers from wrecks and exploration sites
 *
 * Tracks container contents with expiry timers. Supports access control
 * via owner, blue-tag sharing, and abandonment mechanics.
 */
class LootContainer : public ecs::Component {
public:
    struct LootItem {
        std::string item_id;
        std::string name;
        std::string category;    // "Module", "Ammo", "Ore", "Salvage", "DataCore"
        int quantity = 1;
        float volume = 1.0f;     // m3 per unit
        float value = 0.0f;      // estimated ISC value
    };

    std::string container_id;
    std::string owner_id;
    std::string source_type;     // "Wreck", "Anomaly", "Mission", "PlayerJettison"
    std::vector<LootItem> items;
    int max_items = 30;
    float expiry_duration = 7200.0f;   // 2 hours default
    float time_remaining = 7200.0f;
    float total_value = 0.0f;
    float total_volume = 0.0f;
    int total_looted = 0;
    bool is_abandoned = false;
    bool is_locked = false;
    bool active = true;

    COMPONENT_TYPE(LootContainer)
};

// ==================== Mining Laser Cycle ====================

/**
 * @brief Mining laser cycle tracking with progress, yield, and cargo transfer
 *
 * Tracks individual mining laser activation cycles on a target asteroid.
 * Each cycle extracts ore based on yield rate and transfers to cargo hold.
 */
class MiningLaserCycle : public ecs::Component {
public:
    struct ActiveCycle {
        std::string laser_id;
        std::string target_asteroid_id;
        std::string ore_type;
        float cycle_time = 10.0f;        // seconds per cycle
        float progress = 0.0f;           // 0.0 to 1.0
        float yield_per_cycle = 100.0f;  // units of ore per completed cycle
        bool completed = false;
    };

    std::vector<ActiveCycle> cycles;
    std::string cargo_entity_id;         // entity holding cargo for ore deposit
    int max_active_lasers = 3;
    int total_cycles_completed = 0;
    float total_ore_mined = 0.0f;
    float cargo_capacity = 5000.0f;
    float cargo_used = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(MiningLaserCycle)
};

/**
 * @brief Tracks ore processing / refining state per entity
 *
 * When raw ore is mined, it enters a processing queue. Each tick the
 * system converts queued ore into refined materials at a configurable
 * efficiency rate, depositing results into the entity's cargo manifest.
 */
class OreProcessing : public ecs::Component {
public:
    struct OreJob {
        std::string ore_type;
        float raw_amount = 0.0f;
        float refined_amount = 0.0f;
        float progress = 0.0f;        // 0.0 to 1.0
        float processing_time = 30.0f; // seconds per batch
        bool completed = false;
    };

    std::vector<OreJob> jobs;
    float efficiency = 0.75f;           // 75% yield by default
    float processing_speed = 1.0f;     // multiplier on processing rate
    int max_concurrent_jobs = 2;
    int total_batches_completed = 0;
    float total_raw_processed = 0.0f;
    float total_refined_output = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(OreProcessing)
};

// ==================== Anomaly Escalation ====================

/**
 * @brief Tracks combat anomaly escalation for PvE content progression
 *
 * When a player clears an anomaly site, the system may trigger a harder
 * follow-up site (escalation).  Each escalation tier increases NPC
 * difficulty, reward multiplier, and may change site type.
 */
class AnomalyEscalation : public ecs::Component {
public:
    enum class EscalationState { Idle, SiteActive, Cleared, Escalating, EscalationReady, Failed };

    struct EscalationTier {
        int tier = 0;                    // 0 = base, 1-5 = escalation tiers
        std::string site_type;           // e.g. "Combat", "Relic", "Data"
        float difficulty_multiplier = 1.0f;
        float reward_multiplier = 1.0f;
        int npc_count = 5;
        bool completed = false;
    };

    std::vector<EscalationTier> tiers;
    EscalationState state = EscalationState::Idle;
    int current_tier = 0;
    float escalation_chance = 0.3f;      // 30% chance to escalate per clear
    float escalation_timer = 0.0f;       // countdown until escalation spawns
    float escalation_delay = 10.0f;      // seconds before escalation spawns
    std::string system_id;               // star system this anomaly is in
    std::string owner_id;                // player who triggered escalation
    int max_tiers = 5;
    int total_sites_cleared = 0;
    int total_escalations_triggered = 0;
    int total_escalations_completed = 0;
    int total_escalations_failed = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(AnomalyEscalation)
};

// ==================== Asteroid Respawn System ====================

/**
 * @brief Manages asteroid belt regeneration over time
 *
 * Tracks depletion percentage per belt and gradually respawns asteroids
 * so the mining loop is sustainable.  Respawn rate is configurable per
 * belt and scales with total depletion.
 */
class AsteroidRespawn : public ecs::Component {
public:
    enum class RespawnState { Active, Depleted, Regenerating, Full };

    std::string belt_id;
    std::string system_id;
    RespawnState state = RespawnState::Full;
    int total_asteroids = 20;           // current count
    int max_asteroids = 20;             // belt capacity
    int depleted_count = 0;             // how many are depleted
    float depletion_pct = 0.0f;         // 0.0-1.0 fraction of belt mined
    float respawn_rate = 0.01f;         // asteroids per second regeneration
    float respawn_accumulator = 0.0f;   // fractional respawn accumulator
    float regeneration_delay = 60.0f;   // seconds after depletion before respawn starts
    float delay_timer = 0.0f;           // current delay countdown
    int total_respawned = 0;            // lifetime count of respawned asteroids
    int total_depleted = 0;             // lifetime count of mined-out asteroids
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(AsteroidRespawn)
};

/**
 * @brief Sector map fog-of-war discovery state
 *
 * Tracks which sectors have been discovered (visibility 0=hidden,
 * 1=partial, 2=full) and visit counts for exploration metrics.
 */
class SectorMapDiscovery : public ecs::Component {
public:
    struct DiscoveredSector {
        std::string sector_id;
        std::string sector_name;
        int visibility = 0;     // 0=hidden, 1=partial, 2=full
        int visit_count = 0;
        float discovery_time = 0.0f;
    };

    std::vector<DiscoveredSector> sectors;
    int max_sectors = 100;
    int total_discoveries = 0;
    int total_visits = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SectorMapDiscovery)
};

/**
 * @brief Dynamic anomaly spawning state for a star system
 *
 * Controls the periodic spawning and despawning of combat and
 * exploration anomalies.  Anomaly count scales with system security
 * level (more in null-sec) and player activity.
 */
class AnomalySpawningState : public ecs::Component {
public:
    enum class AnomalyType { Combat, Gas, Relic, Data, Wormhole };

    struct SpawnedAnomaly {
        std::string anomaly_id;
        AnomalyType type = AnomalyType::Combat;
        int difficulty = 1;          // 1–5
        float lifetime = 0.0f;       // seconds alive
        float max_lifetime = 3600.0f; // 1 hour default
        bool completed = false;
    };

    std::string system_id;
    float security_level = 0.5f;        // determines max anomalies
    int base_max_anomalies = 3;
    int max_anomalies = 3;              // adjusted by security
    float spawn_interval = 300.0f;      // seconds between spawn checks
    float spawn_timer = 0.0f;
    float despawn_check_interval = 60.0f;
    float despawn_timer = 0.0f;
    std::vector<SpawnedAnomaly> anomalies;
    int total_spawned = 0;
    int total_completed = 0;
    int total_despawned = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(AnomalySpawningState)
};

// ==================== Sleeper AI ====================

/**
 * @brief Sleeper NPC artificial intelligence state
 *
 * Models the advanced AI behaviour of Sleeper drones found in wormhole
 * space.  Sleepers have coordinated target selection, remote-repair
 * capability, and escalation mechanics where reinforcements warp in
 * when the player inflicts enough damage.
 */
class SleeperAIState : public ecs::Component {
public:
    enum class SleeperRole { Sentry, Escort, Guardian, Warden };
    enum class AlertLevel { Dormant, Alerted, Combat, Escalated };

    struct SleeperUnit {
        std::string unit_id;
        SleeperRole role = SleeperRole::Sentry;
        float hp = 1000.0f;
        float max_hp = 1000.0f;
        float dps = 150.0f;
        float remote_rep_amount = 0.0f;  // per second if Guardian
        std::string current_target;
        bool alive = true;
    };

    std::string site_id;
    AlertLevel alert_level = AlertLevel::Dormant;
    std::vector<SleeperUnit> units;
    int max_units = 10;
    float damage_threshold = 2000.0f;     // total damage to trigger escalation
    float damage_taken = 0.0f;
    float escalation_cooldown = 60.0f;    // seconds between escalation waves
    float escalation_timer = 0.0f;
    int escalation_wave = 0;
    int max_escalation_waves = 3;
    float target_switch_interval = 5.0f;  // coordinated re-targeting
    float target_switch_timer = 0.0f;
    int total_kills = 0;
    int total_losses = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SleeperAIState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_EXPLORATION_COMPONENTS_H
