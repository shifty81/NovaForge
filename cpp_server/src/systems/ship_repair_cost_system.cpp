#include "systems/ship_repair_cost_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

ShipRepairCostSystem::ShipRepairCostSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShipRepairCostSystem::updateComponent(ecs::Entity& entity,
    components::ShipRepairCost& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool ShipRepairCostSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShipRepairCost>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipRepairCostSystem::recordDamage(const std::string& entity_id,
    const std::string& source_id, float shield_dmg, float armor_dmg,
    float hull_dmg) {
    auto* src = getComponentFor(entity_id);
    if (!src) return false;

    // Evict oldest record if at capacity
    if (static_cast<int>(src->damage_records.size()) >= src->max_records) {
        src->damage_records.erase(src->damage_records.begin());
    }

    components::ShipRepairCost::DamageRecord rec;
    rec.source_id = source_id;
    rec.shield_damage = shield_dmg;
    rec.armor_damage = armor_dmg;
    rec.hull_damage = hull_dmg;
    rec.timestamp = src->elapsed;
    src->damage_records.push_back(rec);

    // Accumulate repair cost
    src->total_repair_cost += shield_dmg * src->shield_cost_rate;
    src->total_repair_cost += armor_dmg * src->armor_cost_rate;
    src->total_repair_cost += hull_dmg * src->hull_cost_rate;
    src->total_damage_events++;
    return true;
}

bool ShipRepairCostSystem::applyRepair(const std::string& entity_id) {
    auto* src = getComponentFor(entity_id);
    if (!src) return false;
    if (!src->docked) return false;
    if (src->total_repair_cost <= 0.0) return false;

    double discount = src->total_repair_cost * src->discount_rate;
    double final_cost = src->total_repair_cost - discount;
    if (final_cost < 0.0) final_cost = 0.0;

    src->total_isc_spent_on_repairs += final_cost;
    src->total_repair_cost = 0.0;
    src->damage_records.clear();
    src->total_repairs_completed++;
    return true;
}

bool ShipRepairCostSystem::setDocked(const std::string& entity_id, bool docked) {
    auto* src = getComponentFor(entity_id);
    if (!src) return false;
    src->docked = docked;
    return true;
}

bool ShipRepairCostSystem::setDiscount(const std::string& entity_id, float discount) {
    auto* src = getComponentFor(entity_id);
    if (!src) return false;
    src->discount_rate = std::max(0.0f, std::min(1.0f, discount));
    return true;
}

bool ShipRepairCostSystem::setCostRates(const std::string& entity_id,
    float shield_rate, float armor_rate, float hull_rate) {
    auto* src = getComponentFor(entity_id);
    if (!src) return false;
    src->shield_cost_rate = std::max(0.0f, shield_rate);
    src->armor_cost_rate = std::max(0.0f, armor_rate);
    src->hull_cost_rate = std::max(0.0f, hull_rate);
    return true;
}

double ShipRepairCostSystem::getRepairCost(const std::string& entity_id) const {
    auto* src = getComponentFor(entity_id);
    return src ? src->total_repair_cost : 0.0;
}

double ShipRepairCostSystem::getTotalIscSpent(const std::string& entity_id) const {
    auto* src = getComponentFor(entity_id);
    return src ? src->total_isc_spent_on_repairs : 0.0;
}

int ShipRepairCostSystem::getDamageRecordCount(const std::string& entity_id) const {
    auto* src = getComponentFor(entity_id);
    return src ? static_cast<int>(src->damage_records.size()) : 0;
}

int ShipRepairCostSystem::getTotalRepairsCompleted(const std::string& entity_id) const {
    auto* src = getComponentFor(entity_id);
    return src ? src->total_repairs_completed : 0;
}

int ShipRepairCostSystem::getTotalDamageEvents(const std::string& entity_id) const {
    auto* src = getComponentFor(entity_id);
    return src ? src->total_damage_events : 0;
}

} // namespace systems
} // namespace atlas
