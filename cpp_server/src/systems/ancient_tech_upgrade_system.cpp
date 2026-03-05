#include "systems/ancient_tech_upgrade_system.h"
#include "ecs/world.h"
#include <sstream>
#include <iomanip>

namespace atlas {
namespace systems {

AncientTechUpgradeSystem::AncientTechUpgradeSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void AncientTechUpgradeSystem::updateComponent(ecs::Entity& entity, components::AncientTechModule& tech, float delta_time) {
    auto* upg = entity.getComponent<components::AncientTechUpgradeState>();
    if (!upg || !upg->upgrading) return;
    if (upg->upgrade_cost <= 0.0f) return;

    upg->upgrade_progress = std::min(1.0f, upg->upgrade_progress + delta_time / upg->upgrade_cost);
    if (upg->upgrade_progress >= 1.0f) {
        upg->upgrade_progress = 1.0f;
        upg->upgrading = false;
        tech.state = components::AncientTechModule::TechState::Upgraded;
        upg->bonus_type = tech.tech_type;
        upg->bonus_magnitude = tech.power_multiplier;
    }
}

bool AncientTechUpgradeSystem::startUpgrade(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* tech = getComponentFor(entity_id);
    if (!tech) return false;
    if (tech->state != components::AncientTechModule::TechState::Repaired) return false;

    auto* upg = entity->getComponent<components::AncientTechUpgradeState>();
    if (!upg) {
        auto u = std::make_unique<components::AncientTechUpgradeState>();
        upg = u.get();
        entity->addComponent(std::move(u));
    }
    upg->upgrading = true;
    upg->upgrade_progress = 0.0f;
    return true;
}

float AncientTechUpgradeSystem::getUpgradeProgress(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* upg = entity->getComponent<components::AncientTechUpgradeState>();
    if (!upg) return 0.0f;
    return upg->upgrade_progress;
}

bool AncientTechUpgradeSystem::hasRuleBreakingBonuses(const std::string& entity_id) const {
    const auto* tech = getComponentFor(entity_id);
    if (!tech) return false;
    return tech->state == components::AncientTechModule::TechState::Upgraded;
}

float AncientTechUpgradeSystem::getStatMultiplier(const std::string& entity_id) const {
    const auto* tech = getComponentFor(entity_id);
    if (!tech) return 1.0f;
    if (tech->state == components::AncientTechModule::TechState::Upgraded) {
        return tech->power_multiplier;
    }
    return 1.0f;
}

int AncientTechUpgradeSystem::getUpgradingCount() const {
    int count = 0;
    auto entities = world_->getEntities<components::AncientTechUpgradeState>();
    for (auto* entity : entities) {
        auto* upg = entity->getComponent<components::AncientTechUpgradeState>();
        if (upg && upg->upgrading) {
            ++count;
        }
    }
    return count;
}

int AncientTechUpgradeSystem::getUpgradedCount() const {
    int count = 0;
    auto entities = world_->getEntities<components::AncientTechModule>();
    for (auto* entity : entities) {
        auto* tech = entity->getComponent<components::AncientTechModule>();
        if (tech && tech->state == components::AncientTechModule::TechState::Upgraded) {
            ++count;
        }
    }
    return count;
}

std::string AncientTechUpgradeSystem::getBonusDescription(const std::string& entity_id) const {
    const auto* tech = getComponentFor(entity_id);
    if (!tech) return "";
    if (tech->state != components::AncientTechModule::TechState::Upgraded) return "";

    std::ostringstream oss;
    oss << "Ancient " << tech->tech_type << " module: "
        << std::fixed << std::setprecision(1) << tech->power_multiplier
        << "x " << tech->tech_type << " capacity (exceeds modern limit)";
    return oss.str();
}

bool AncientTechUpgradeSystem::cancelUpgrade(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* upg = entity->getComponent<components::AncientTechUpgradeState>();
    if (!upg || !upg->upgrading) return false;
    auto* tech = getComponentFor(entity_id);
    if (tech) {
        tech->state = components::AncientTechModule::TechState::Repaired;
    }
    upg->upgrading = false;
    upg->upgrade_progress = 0.0f;
    return true;
}

} // namespace systems
} // namespace atlas
