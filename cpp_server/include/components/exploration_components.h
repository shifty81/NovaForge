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


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_EXPLORATION_COMPONENTS_H
