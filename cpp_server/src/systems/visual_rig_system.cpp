#include "systems/visual_rig_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

VisualRigSystem::VisualRigSystem(ecs::World* world)
    : System(world) {
}

void VisualRigSystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::VisualRigState>();
    for (auto* entity : entities) {
        auto* visual = entity->getComponent<components::VisualRigState>();
        if (!visual) continue;

        // Auto-update visual state from RigLoadout if present
        auto* loadout = entity->getComponent<components::RigLoadout>();
        if (loadout) {
            // Calculate total bulk from loadout stats
            float bulk = 1.0f;
            bulk += loadout->total_cargo / 500.0f;  // cargo adds bulk
            bulk += loadout->total_shield / 50.0f;  // shields add bulk
            bulk += loadout->jetpack_fuel / 100.0f; // jetpack tanks add bulk
            visual->total_bulk = std::min(3.0f, bulk);

            // Glow intensity from power
            visual->glow_intensity = std::min(1.0f, loadout->total_power / 100.0f);
        }
    }
}

bool VisualRigSystem::initializeVisualState(const std::string& entity_id,
                                             uint64_t visual_seed) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::VisualRigState>();
    if (existing) return false;

    auto comp = std::make_unique<components::VisualRigState>();
    comp->rig_entity_id = entity_id;
    comp->visual_seed = visual_seed;
    comp->thruster_config = components::VisualRigState::ThrusterConfig::None;
    comp->cargo_size = components::VisualRigState::CargoSize::None;
    comp->primary_color = "neutral";
    comp->secondary_color = "black";
    entity->addComponent(std::move(comp));
    return true;
}

bool VisualRigSystem::removeVisualState(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    entity->removeComponent<components::VisualRigState>();
    return true;
}

bool VisualRigSystem::updateFromLoadout(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    auto* loadout = entity->getComponent<components::RigLoadout>();
    if (!loadout) return false;

    // Reset visual state
    visual->thruster_config = components::VisualRigState::ThrusterConfig::None;
    visual->cargo_size = components::VisualRigState::CargoSize::None;
    visual->has_shield_emitter = false;
    visual->has_antenna = false;
    visual->has_solar_panels = false;
    visual->has_drone_bay = false;
    visual->weapon_mount_count = 0;
    visual->tool_mount_count = 0;

    // Process each installed module
    for (const auto& mod_id : loadout->installed_module_ids) {
        auto* mod_entity = world_->getEntity(mod_id);
        if (!mod_entity) continue;
        auto* mod = mod_entity->getComponent<components::RigModule>();
        if (!mod) continue;

        switch (mod->type) {
            case components::RigModule::ModuleType::JetpackTank:
                // Upgrade thruster config
                if (visual->thruster_config == components::VisualRigState::ThrusterConfig::None) {
                    visual->thruster_config = components::VisualRigState::ThrusterConfig::Single;
                } else if (visual->thruster_config == components::VisualRigState::ThrusterConfig::Single) {
                    visual->thruster_config = components::VisualRigState::ThrusterConfig::Dual;
                } else if (visual->thruster_config == components::VisualRigState::ThrusterConfig::Dual) {
                    visual->thruster_config = components::VisualRigState::ThrusterConfig::Quad;
                }
                break;

            case components::RigModule::ModuleType::CargoPod:
                // Upgrade cargo size
                if (visual->cargo_size == components::VisualRigState::CargoSize::None) {
                    visual->cargo_size = components::VisualRigState::CargoSize::Small;
                } else if (visual->cargo_size == components::VisualRigState::CargoSize::Small) {
                    visual->cargo_size = components::VisualRigState::CargoSize::Medium;
                } else if (visual->cargo_size == components::VisualRigState::CargoSize::Medium) {
                    visual->cargo_size = components::VisualRigState::CargoSize::Large;
                }
                break;

            case components::RigModule::ModuleType::Shield:
                visual->has_shield_emitter = true;
                break;

            case components::RigModule::ModuleType::Sensor:
            case components::RigModule::ModuleType::ScannerSuite:
                visual->has_antenna = true;
                break;

            case components::RigModule::ModuleType::SolarPanel:
                visual->has_solar_panels = true;
                break;

            case components::RigModule::ModuleType::DroneController:
                visual->has_drone_bay = true;
                break;

            case components::RigModule::ModuleType::WeaponMount:
                visual->weapon_mount_count++;
                break;

            case components::RigModule::ModuleType::ToolMount:
                visual->tool_mount_count++;
                break;

            default:
                break;
        }
    }

    // Calculate bulk and glow from loadout stats
    float bulk = 1.0f;
    bulk += loadout->total_cargo / 500.0f;
    bulk += loadout->total_shield / 50.0f;
    bulk += loadout->jetpack_fuel / 100.0f;
    visual->total_bulk = std::min(3.0f, bulk);
    visual->glow_intensity = std::min(1.0f, loadout->total_power / 100.0f);

    return true;
}

std::string VisualRigSystem::getThrusterConfig(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return "unknown";

    return components::VisualRigState::thrusterConfigToString(visual->thruster_config);
}

bool VisualRigSystem::setThrusterScale(const std::string& entity_id, float scale) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    visual->thruster_scale = std::max(0.1f, std::min(3.0f, scale));
    return true;
}

float VisualRigSystem::getThrusterScale(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 1.0f;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 1.0f;

    return visual->thruster_scale;
}

std::string VisualRigSystem::getCargoSize(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return "unknown";

    return components::VisualRigState::cargoSizeToString(visual->cargo_size);
}

bool VisualRigSystem::setCargoScale(const std::string& entity_id, float scale) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    visual->cargo_scale = std::max(0.1f, std::min(3.0f, scale));
    return true;
}

float VisualRigSystem::getCargoScale(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 1.0f;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 1.0f;

    return visual->cargo_scale;
}

bool VisualRigSystem::hasShieldEmitter(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    return visual->has_shield_emitter;
}

bool VisualRigSystem::hasAntenna(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    return visual->has_antenna;
}

bool VisualRigSystem::hasSolarPanels(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    return visual->has_solar_panels;
}

bool VisualRigSystem::hasDroneBay(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    return visual->has_drone_bay;
}

int VisualRigSystem::getWeaponMountCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 0;

    return visual->weapon_mount_count;
}

int VisualRigSystem::getToolMountCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 0;

    return visual->tool_mount_count;
}

float VisualRigSystem::getTotalBulk(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 1.0f;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 1.0f;

    return visual->total_bulk;
}

float VisualRigSystem::getGlowIntensity(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 0.0f;

    return visual->glow_intensity;
}

bool VisualRigSystem::setGlowIntensity(const std::string& entity_id, float intensity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    visual->glow_intensity = std::max(0.0f, std::min(1.0f, intensity));
    return true;
}

bool VisualRigSystem::setColors(const std::string& entity_id,
                                 const std::string& primary,
                                 const std::string& secondary) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    visual->primary_color = primary;
    visual->secondary_color = secondary;
    return true;
}

std::string VisualRigSystem::getPrimaryColor(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return "";

    return visual->primary_color;
}

std::string VisualRigSystem::getSecondaryColor(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return "";

    return visual->secondary_color;
}

bool VisualRigSystem::addTrinket(const std::string& entity_id,
                                  const std::string& trinket_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    if (!visual->canAddTrinket()) return false;

    // Check for duplicate
    if (std::find(visual->trinket_ids.begin(), visual->trinket_ids.end(), trinket_id)
        != visual->trinket_ids.end()) {
        return false;
    }

    visual->trinket_ids.push_back(trinket_id);
    return true;
}

bool VisualRigSystem::removeTrinket(const std::string& entity_id,
                                     const std::string& trinket_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    auto it = std::find(visual->trinket_ids.begin(), visual->trinket_ids.end(), trinket_id);
    if (it == visual->trinket_ids.end()) return false;

    visual->trinket_ids.erase(it);
    return true;
}

int VisualRigSystem::getTrinketCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return 0;

    return static_cast<int>(visual->trinket_ids.size());
}

bool VisualRigSystem::canAddTrinket(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* visual = entity->getComponent<components::VisualRigState>();
    if (!visual) return false;

    return visual->canAddTrinket();
}

} // namespace systems
} // namespace atlas
