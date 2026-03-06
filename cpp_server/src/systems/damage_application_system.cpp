#include "systems/damage_application_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

using DA = components::DamageApplication;

DamageApplicationSystem::DamageApplicationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void DamageApplicationSystem::updateComponent(ecs::Entity& entity,
    components::DamageApplication& da, float delta_time) {
    if (!da.active || da.pending.empty()) return;
    da.elapsed += delta_time;

    auto* health = entity.getComponent<components::Health>();
    if (!health) { da.pending.clear(); return; }

    for (auto& dmg : da.pending) {
        float raw = dmg.raw_amount;
        float applied = 0.0f;

        // Apply to shield first
        if (health->shield_hp > 0.0f) {
            float resist = 0.0f;
            switch (dmg.type) {
                case DA::DamageType::EM:        resist = health->shield_em_resist; break;
                case DA::DamageType::Thermal:   resist = health->shield_thermal_resist; break;
                case DA::DamageType::Kinetic:   resist = health->shield_kinetic_resist; break;
                case DA::DamageType::Explosive:  resist = health->shield_explosive_resist; break;
            }
            float effective = raw * (1.0f - resist);
            float shield_dmg = std::min(effective, health->shield_hp);
            health->shield_hp -= shield_dmg;
            applied += shield_dmg;
            da.total_mitigated += raw * resist;
            raw = effective - shield_dmg;  // overflow
        }

        // Apply overflow to armor
        if (raw > 0.0f && health->armor_hp > 0.0f) {
            float resist = 0.0f;
            switch (dmg.type) {
                case DA::DamageType::EM:        resist = health->armor_em_resist; break;
                case DA::DamageType::Thermal:   resist = health->armor_thermal_resist; break;
                case DA::DamageType::Kinetic:   resist = health->armor_kinetic_resist; break;
                case DA::DamageType::Explosive:  resist = health->armor_explosive_resist; break;
            }
            float effective = raw * (1.0f - resist);
            float armor_dmg = std::min(effective, health->armor_hp);
            health->armor_hp -= armor_dmg;
            applied += armor_dmg;
            da.total_mitigated += raw * resist;
            raw = effective - armor_dmg;  // overflow
        }

        // Apply overflow to hull
        if (raw > 0.0f && health->hull_hp > 0.0f) {
            float resist = 0.0f;
            switch (dmg.type) {
                case DA::DamageType::EM:        resist = health->hull_em_resist; break;
                case DA::DamageType::Thermal:   resist = health->hull_thermal_resist; break;
                case DA::DamageType::Kinetic:   resist = health->hull_kinetic_resist; break;
                case DA::DamageType::Explosive:  resist = health->hull_explosive_resist; break;
            }
            float effective = raw * (1.0f - resist);
            float hull_dmg = std::min(effective, health->hull_hp);
            health->hull_hp -= hull_dmg;
            applied += hull_dmg;
            da.total_mitigated += raw * resist;
        }

        da.total_applied += applied;
        da.hits_processed++;
    }
    da.pending.clear();
}

bool DamageApplicationSystem::initializeDamageTracking(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DamageApplication>();
    entity->addComponent(std::move(comp));
    return true;
}

bool DamageApplicationSystem::queueDamage(const std::string& entity_id,
    const std::string& source_id, float raw_amount, int damage_type, float timestamp) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* da = entity->getComponent<components::DamageApplication>();
    if (!da) return false;
    if (static_cast<int>(da->pending.size()) >= da->max_pending) return false;

    DA::PendingDamage pd;
    pd.source_id = source_id;
    pd.raw_amount = raw_amount;
    pd.type = static_cast<DA::DamageType>(damage_type);
    pd.timestamp = timestamp;
    da->pending.push_back(pd);
    return true;
}

int DamageApplicationSystem::getPendingCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* da = entity->getComponent<components::DamageApplication>();
    return da ? static_cast<int>(da->pending.size()) : 0;
}

float DamageApplicationSystem::getTotalApplied(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* da = entity->getComponent<components::DamageApplication>();
    return da ? da->total_applied : 0.0f;
}

float DamageApplicationSystem::getTotalMitigated(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* da = entity->getComponent<components::DamageApplication>();
    return da ? da->total_mitigated : 0.0f;
}

int DamageApplicationSystem::getHitsProcessed(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* da = entity->getComponent<components::DamageApplication>();
    return da ? da->hits_processed : 0;
}

bool DamageApplicationSystem::hasPendingDamage(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* da = entity->getComponent<components::DamageApplication>();
    return da ? !da->pending.empty() : false;
}

bool DamageApplicationSystem::clearPending(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* da = entity->getComponent<components::DamageApplication>();
    if (!da) return false;
    da->pending.clear();
    return true;
}

} // namespace systems
} // namespace atlas
